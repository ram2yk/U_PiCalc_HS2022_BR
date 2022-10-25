/*
 * U_PiCalc_HS2022.c
 *
 * Created: 04.10.2022 18:00:00
 * Author : Balaram Ramalingam
 */ 

#include <math.h>	
#include <stdio.h>
#include <string.h>
// #include <inttypes.h>
#include "avr_compiler.h"
#include "pmic_driver.h"
#include "TC_driver.h"
#include "clksys_driver.h"
#include "sleepConfig.h"
#include "port_driver.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"
#include "stack_macros.h"

#include "mem_check.h"

#include "init.h"
#include "utils.h"
#include "errorHandler.h"
#include "NHD0420Driver.h"

#include "ButtonHandler.h"

// Function forward declarations
void controllerTask(void* pvParameters);
void leibnizTask(void* pvParameters);
void nilakanthaTask(void* pvParameters);
void buttonTask(void* pvParameters);

//EventGroup for ButtonEvents.
EventGroupHandle_t egButtonEvents = NULL;
#define BUTTON1_SHORT	0x01
#define BUTTON1_LONG	0x02
#define BUTTON2_SHORT	0x04
#define BUTTON2_LONG	0x08
#define BUTTON3_SHORT	0x10
#define BUTTON3_LONG	0x20
#define BUTTON4_SHORT	0x40
#define BUTTON4_LONG	0x80
#define BUTTON_ALL		0xFF

#define ALGO_LEIBNIZ			"lbnz"
#define ALGO_NILAKANTHA			"nlkn"

float pi_5decimal = 3.14159;

TaskHandle_t xHandleLeibniz = NULL;
TaskHandle_t xHandleNilakantha = NULL;

float pi = 0.0;
char algorithm[5] = ALGO_LEIBNIZ;  // Default algorithm: leibniz

// Variables to measure the time
TickType_t xTimeDifference = 0;
TickType_t xTimeDifferenceLeibniz = 0;
TickType_t xTimeDifferenceNilakantha = 0;

 
int main(void)
{
	vInitClock();
	vInitDisplay();
	
	// Calculation task for PI with leibniz series
	xTaskCreate( leibnizTask, (const char *) "leibniz_task", configMINIMAL_STACK_SIZE+150, NULL, 1, &xHandleLeibniz);
	vTaskSuspend( xHandleLeibniz );
	
	// Calculation task for PI with another method
	xTaskCreate(nilakanthaTask, (const char *) "nilakantha_task", configMINIMAL_STACK_SIZE+150, NULL, 1, &xHandleNilakantha);
	vTaskSuspend( xHandleNilakantha );
	
	// Interface Task
	xTaskCreate( controllerTask, (const char *) "control_task", configMINIMAL_STACK_SIZE+150, NULL, 2, NULL);
	
	// Button Task
	xTaskCreate( buttonTask, (const char*) "button_task", configMINIMAL_STACK_SIZE, NULL, 3, NULL); //Init ButtonTask. Medium Priority. Somehow important to time Button debouncing and timing.

	vTaskStartScheduler();
	return 0;
}

void controllerTask(void* pvParameters) {
	while(egButtonEvents == NULL) { //Wait for EventGroup to be initialized in other task
		vTaskDelay(10/portTICK_RATE_MS);
	}
	char piString[12];
	char timeString[11];

	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"PI-Calc HS2022 - Ram"); // Draw title
	vDisplayWriteStringAtPos(3,0,"|Str |Stp |Rst |Alg"); //Draw Button Info
	
	for(;;) {
		if(xEventGroupGetBits(egButtonEvents) & BUTTON1_SHORT) {
			if (strcmp(algorithm, ALGO_LEIBNIZ) == 0) {
				if (xHandleLeibniz != NULL) {
					vTaskResume(xHandleLeibniz);
				} else {
					xTaskCreate( leibnizTask, (const char *) "leibniz_task", configMINIMAL_STACK_SIZE+150, NULL, 3, &xHandleLeibniz);
				}
				vTaskSuspend(xHandleNilakantha);
			} else if (strcmp(algorithm, ALGO_NILAKANTHA) == 0) {
				if (xHandleNilakantha != NULL) {
					vTaskResume( xHandleNilakantha );
				} else {
					xTaskCreate(nilakanthaTask, (const char *) "nilakantha_task", configMINIMAL_STACK_SIZE+150, NULL, 3, &xHandleNilakantha);
				}
				
				vTaskSuspend( xHandleLeibniz );
			}
			
		}
		if(xEventGroupGetBits(egButtonEvents) & BUTTON2_SHORT) {
			vTaskSuspend( xHandleLeibniz );
			vTaskSuspend( xHandleNilakantha );			
		}
		if(xEventGroupGetBits(egButtonEvents) & BUTTON3_SHORT) {
			
			if (strcmp(algorithm, ALGO_LEIBNIZ) == 0) {
				if (xHandleLeibniz != NULL) {
					vTaskDelete(xHandleLeibniz);
					xTimeDifferenceLeibniz = 0;
					pi = 0;
					
				}
			} else if (strcmp(algorithm, ALGO_NILAKANTHA) == 0) {
				if (xHandleNilakantha != NULL) {
					vTaskDelete( xHandleNilakantha );
					xTimeDifferenceNilakantha = 0;
					pi = 0;
				}
			}
			vDisplayWriteStringAtPos(2,13, "T:           ");  // Clear 11 chars allocated for time display
		}
		if(xEventGroupGetBits(egButtonEvents) & BUTTON4_SHORT) {
			if (strcmp(algorithm, ALGO_LEIBNIZ) == 0) {
				sprintf(&algorithm[0], "%s", ALGO_NILAKANTHA);
				vDisplayWriteStringAtPos(2, 0, "Al:    ");  // Clear 4 chars allocated for algorithm display
				if (xHandleNilakantha != NULL) {
					vTaskResume( xHandleNilakantha );
				} else {
					xTaskCreate(nilakanthaTask, (const char *) "nilakantha_task", configMINIMAL_STACK_SIZE+150, NULL, 3, &xHandleNilakantha);
				}
				
				vTaskSuspend( xHandleLeibniz );
			} else if (strcmp(algorithm, ALGO_NILAKANTHA) == 0) {
				sprintf(&algorithm[0], "%s", ALGO_LEIBNIZ);
				vDisplayWriteStringAtPos(2, 0, "Al:    ");  // Clear 4 chars allocated for algorithm display
				if (xHandleLeibniz != NULL) {
					vTaskResume(xHandleLeibniz);
				} else {
					xTaskCreate( leibnizTask, (const char *) "leibniz_task", configMINIMAL_STACK_SIZE+150, NULL, 3, &xHandleLeibniz);
				}
				vTaskSuspend(xHandleNilakantha);
				
			}
		}
		if(getButtonPress(BUTTON1) == LONG_PRESSED) {
			
		}
		if(getButtonPress(BUTTON2) == LONG_PRESSED) {
			
		}
		if(getButtonPress(BUTTON3) == LONG_PRESSED) {
			
		}
		if(getButtonPress(BUTTON4) == LONG_PRESSED) {
			
		}
		
		sprintf(&piString[0], "PI: %.8f", pi);
		vDisplayWriteStringAtPos(1,0, "%s", piString);
		vDisplayWriteStringAtPos(2,0, "Al:%s", algorithm);


		if (strcmp(algorithm, ALGO_LEIBNIZ) == 0) {
			xTimeDifference = xTimeDifferenceLeibniz;
		} else if (strcmp(algorithm, ALGO_NILAKANTHA) == 0) {
			xTimeDifference = xTimeDifferenceNilakantha;
		}
		// Write time difference to display
		sprintf(&timeString[0], "T:%lu", xTimeDifference);
		vDisplayWriteStringAtPos(2,8, "%sms", timeString);
		
		xEventGroupClearBits(egButtonEvents, BUTTON_ALL);
		vTaskDelay(500/portTICK_RATE_MS);
	}
}

void buttonTask(void *pvParameters) {
	egButtonEvents = xEventGroupCreate();
	initButtons(); //Initialize Buttons
	for(;;) {
		updateButtons();
		
		if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON1_SHORT);
		}
		if(getButtonPress(BUTTON2) == SHORT_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON2_SHORT);
		}
		if(getButtonPress(BUTTON3) == SHORT_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON3_SHORT);
		}
		if(getButtonPress(BUTTON4) == SHORT_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON4_SHORT);
		}
		if(getButtonPress(BUTTON1) == LONG_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON1_LONG);
		}
		if(getButtonPress(BUTTON2) == LONG_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON2_LONG);
		}
		if(getButtonPress(BUTTON3) == LONG_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON3_LONG);
		}
		if(getButtonPress(BUTTON4) == LONG_PRESSED) {
			xEventGroupSetBits(egButtonEvents, BUTTON4_LONG);
		}
		vTaskDelay((1000/BUTTON_UPDATE_FREQUENCY_HZ)/portTICK_RATE_MS);
	}
}


void leibnizTask(void* pvParameters) {

	TickType_t xStartTimeLeibniz, xStopTimeLeibniz;
	
	xTimeDifferenceLeibniz = 0;
	xStartTimeLeibniz = xTaskGetTickCount();
	uint32_t n = 3;
	float piviertel = 1.0;
	for (;;) {
		piviertel = piviertel - 1.0/n + 1.0/(n+2);
		pi = piviertel * 4;
		n = n + 4;
		xStopTimeLeibniz = xTaskGetTickCount();
		if ((floorf(pi * 100000) / 100000) != 3.14159f) {
			xTimeDifferenceLeibniz = xStopTimeLeibniz - xStartTimeLeibniz;
		}

		vTaskDelay(10/portTICK_RATE_MS);
	}
	

}

void nilakanthaTask(void* pvParameters) {

	TickType_t xStartTimeNilakantha, xStopTimeNilakantha;

	xTimeDifferenceNilakantha = 0;
	xStartTimeNilakantha = xTaskGetTickCount();
	uint32_t n = 3;
	float pi_local = 3.0;
	for (;;) {
		pi_local = pi_local + 4.0/(pow(n,3) - n) - 4.0/(pow(n+2,3) - (n+2));
		pi = pi_local;
		n = n + 4;
		xStopTimeNilakantha = xTaskGetTickCount();
		if ((floorf(pi *100000) / 100000) != 3.14159f) {
			xTimeDifferenceNilakantha = xStopTimeNilakantha - xStartTimeNilakantha;
		}

		vTaskDelay(10/portTICK_RATE_MS);
	}
}