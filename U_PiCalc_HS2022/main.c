/*
 * U_PiCalc_HS2022.c
 *
 * Created: 20.03.2018 18:32:07
 * Author : -
 */ 

#include <math.h>
#include <stdio.h>
#include <string.h>
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

// Function forward declararions
void controllerTask(void* pvParameters);
void leibnizTask(void* pvParameters);
void algo2Task(void* pvParameters);
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

#define ALGO_LEIBNIZ	"leibniz"
#define ALGO_2			"algo2"

TaskHandle_t xHandleLeibniz = NULL;
TaskHandle_t xHandleAlgo2 = NULL;

float pi = 0.0;
char algorithm[10] = ALGO_LEIBNIZ;  // Default algorithm: leibniz

 
int main(void)
{
	vInitClock();
	vInitDisplay();
	
	// Calculation task for PI with leibniz series
	xTaskCreate( leibnizTask, (const char *) "leibniz_task", configMINIMAL_STACK_SIZE+150, NULL, 3, &xHandleLeibniz);
	vTaskSuspend( xHandleLeibniz );
	
	// Calculation task for PI with another method
	xTaskCreate(algo2Task, (const char *) "algo2_task", configMINIMAL_STACK_SIZE+150, NULL, 3, &xHandleAlgo2);
	vTaskSuspend( xHandleAlgo2 );
	
	// Interface Task
	xTaskCreate( controllerTask, (const char *) "control_task", configMINIMAL_STACK_SIZE+150, NULL, 1, NULL);
	
	// Button Task
	xTaskCreate( buttonTask, (const char*) "button_task", configMINIMAL_STACK_SIZE, NULL, 2, NULL); //Init ButtonTask. Medium Priority. Somehow important to time Button debouncing and timing.
	

	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"PI-Calc HS2022 - Ram"); // Draw title
	vDisplayWriteStringAtPos(3,0,"|Str |Stp |Rst |Alg"); //Draw Button Info
	
	vTaskStartScheduler();
	return 0;
}

void controllerTask(void* pvParameters) {
	while(egButtonEvents == NULL) { //Wait for EventGroup to be initialized in other task
		vTaskDelay(10/portTICK_RATE_MS);
	}
	char pistring[12];
	for(;;) {
		if(xEventGroupGetBits(egButtonEvents) & BUTTON1_SHORT) {
			if (strcmp(algorithm, ALGO_LEIBNIZ) == 0) {
				vTaskResume(xHandleLeibniz);
				vTaskSuspend(xHandleAlgo2);
			} else if (strcmp(algorithm, ALGO_2) == 0) {
				vTaskResume( xHandleAlgo2 );
				vTaskSuspend( xHandleLeibniz );
			}
			
		}
		if(xEventGroupGetBits(egButtonEvents) & BUTTON2_SHORT) {
			vTaskSuspend( xHandleLeibniz );
			vTaskSuspend( xHandleAlgo2 );			
		}
		if(xEventGroupGetBits(egButtonEvents) & BUTTON3_SHORT) {
			pi = 0.0;
		}
		if(xEventGroupGetBits(egButtonEvents) & BUTTON4_SHORT) {
			if (strcmp(algorithm, ALGO_LEIBNIZ) == 0) {
				sprintf(&algorithm[0], "%s", ALGO_2);
				vDisplayWriteStringAtPos(2, 0, "Al:           ");
			} else if (strcmp(algorithm, ALGO_2) == 0) {
				sprintf(&algorithm[0], "%s", ALGO_LEIBNIZ);
				vDisplayWriteStringAtPos(2, 0, "Al:           ");
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
		
		sprintf(&pistring[0], "PI: %.8f", pi);
		vDisplayWriteStringAtPos(1,0, "%s", pistring);
		vDisplayWriteStringAtPos(2,0, "Al:%s", algorithm);
		
		xEventGroupClearBits(egButtonEvents, BUTTON_ALL);
		vTaskDelay(10/portTICK_RATE_MS);
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
		vTaskDelay((500/BUTTON_UPDATE_FREQUENCY_HZ)/portTICK_RATE_MS);
	}
}

void leibnizTask(void* pvParameters) {
	uint32_t n = 3;
	float piviertel = 1;
	for (;;) {
		piviertel = piviertel - 1.0/n + 1.0/(n+2);
		pi = piviertel * 4;
		n = n + 4;
		vTaskDelay(1000/portTICK_RATE_MS);
		
	}
}

void algo2Task(void* pvParameters) {
	uint32_t n = 3;
	float piviertel = 3;
	for (;;) {
		piviertel = piviertel - 1.0/n + 1.0/(n+2);
		n = n + 4;
		vTaskDelay(1000/portTICK_RATE_MS);
		
	}
}