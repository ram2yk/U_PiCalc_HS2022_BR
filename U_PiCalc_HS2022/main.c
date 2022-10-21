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


void controllerTask(void* pvParameters);
void leibniztask(void* pvParameters);

float piviertel = 1.0;

int main(void)
{
	vInitClock();
	vInitDisplay();
	
	// Calculation task for PI with leibniz series
	xTaskCreate( leibniztask, (const char *) "leibniz_task", configMINIMAL_STACK_SIZE+150, NULL, 3, NULL);
	// Calculation task for PI with another method
	// xTaskCreate();
	// Interface Task
	xTaskCreate( controllerTask, (const char *) "control_task", configMINIMAL_STACK_SIZE+150, NULL, 1, NULL);
	

	vDisplayClear();
	vDisplayWriteStringAtPos(0,0,"PI-Calc HS2022");
	
	vTaskStartScheduler();
	return 0;
}

void controllerTask(void* pvParameters) {
	initButtons();
	for(;;) {
		updateButtons();
		if(getButtonPress(BUTTON1) == SHORT_PRESSED) {
			char pistring[12];
			sprintf(&pistring[0], "PI: %.8f", piviertel);
			vDisplayWriteStringAtPos(1,0, "%s", pistring);
		}
		if(getButtonPress(BUTTON2) == SHORT_PRESSED) {
			
		}
		if(getButtonPress(BUTTON3) == SHORT_PRESSED) {
			
		}
		if(getButtonPress(BUTTON4) == SHORT_PRESSED) {
			
		}
		if(getButtonPress(BUTTON1) == LONG_PRESSED) {
			
		}
		if(getButtonPress(BUTTON2) == LONG_PRESSED) {
			
		}
		if(getButtonPress(BUTTON3) == LONG_PRESSED) {
			
		}
		if(getButtonPress(BUTTON4) == LONG_PRESSED) {
			
		}
		vDisplayWriteStringAtPos(3,0,"Str|Stp|Rst|Alg"); //Draw Button Info
		vTaskDelay(10/portTICK_RATE_MS);
	}
}

void leibniztask(void* pvParameters) {
	
	uint32_t n = 3;
	piviertel = 1;
	// for (;;) {
	int jj;
	for (jj = 0; jj < 10; jj++) {
		piviertel = piviertel - 1.0/n + 1.0/(n+2);
		n = n + 4;
		vTaskDelay(1000/portTICK_RATE_MS);
	}
}