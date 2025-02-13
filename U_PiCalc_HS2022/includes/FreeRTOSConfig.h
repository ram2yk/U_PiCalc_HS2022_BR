/*
 * FreeRTOS Kernel V10.0.1
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <avr/io.h>

#define configUSE_PREEMPTION		1
#define configUSE_IDLE_HOOK			0
#define configUSE_TICK_HOOK			0

#define configCPU_CLOCK_HZ			( ( unsigned portLONG ) 32000000 )
#ifndef F_CPU
# warning ("F_CPU undefined !")
#else
#if (F_CPU != 32000000)
#  warning ("F_CPU and configCPU_CLOCK_HZ are not consistent !")
#endif
#endif

#define portTICK_PERIOD_MS ((portTickType) 1000 / configTICK_RATE_HZ)

#define configTICK_RATE_HZ			( ( portTickType ) 1000 )
#define config24BITADDRESSING		1 // 0 for 32A4 ; 1 for 128A1,256A3
#define configEXTENDED_ADRESSING	0
#define configENABLE_ROUND_ROBIN	1
#define configKERNEL_INTERRUPT_PRIORITY			0	// kernel interrupt level is low-level, don't change!
#define configMAX_SYSCALL_INTERRUPT_PRIORITY	2	// 0=low-level ,1=medium-level, 2=high-level
#if (configMAX_SYSCALL_INTERRUPT_PRIORITY < configKERNEL_INTERRUPT_PRIORITY)
#error configMAX_SYSCALL_INTERRUPT_PRIORITY must be >= configKERNEL_INTERRUPT_PRIORITY !
#endif

#define configCALL_STACK_SIZE	20

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE. 
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

//#define configUSE_PREEMPTION		1
//#define configUSE_IDLE_HOOK			1
//#define configUSE_TICK_HOOK			0
//#define configCPU_CLOCK_HZ			( ( unsigned long ) 8000000 )
//#define configTICK_RATE_HZ			( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES			( 4 )
#define configMINIMAL_STACK_SIZE		( ( unsigned short ) 200 )
#define configTOTAL_HEAP_SIZE			( (size_t ) ( 3000 ) )
#define configMAX_TASK_NAME_LEN			( 8 )
#define configUSE_TRACE_FACILITY		1
#define configUSE_16_BIT_TICKS			0
#define configIDLE_SHOULD_YIELD			1
#define configCHECK_FOR_STACK_OVERFLOW	2

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES 		1
#define configMAX_CO_ROUTINE_PRIORITIES ( 2 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet		1
#define INCLUDE_uxTaskPriorityGet		0
#define INCLUDE_vTaskDelete				0
#define INCLUDE_vTaskCleanUpResources	0
#define INCLUDE_vTaskSuspend			1
#define INCLUDE_vTaskDelayUntil			1
#define INCLUDE_vTaskDelay				1
#define INCLUDE_vTaskDelete				1

#define INCLUDE_uxTaskGetStackHighWaterMark	1 // used to check if stack is going low
#define	INCLUDE_xTaskGetCurrentTaskHandle	1

#define configUSE_TIMERS				1
#define INCLUDE_xTimerPendFunctionCall	1
#define configTIMER_QUEUE_LENGTH		5
#define configTIMER_TASK_PRIORITY		3
#define configTIMER_TASK_STACK_DEPTH	configMINIMAL_STACK_SIZE

#endif /* FREERTOS_CONFIG_H */
