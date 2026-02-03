/* -------------------------------------------------------------------------- 
 * Copyright (c) 2013-2019 ARM Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 *      Name:    Blinky.c
 *      Purpose: RTX example program
 *
 *---------------------------------------------------------------------------*/

#include <stdio.h>

#include "cmsis_os2.h"                  // ARM::CMSIS:RTOS2:Keil RTX5

#include "RTE_Components.h"
#include CMSIS_device_header

osThreadId_t tid_phaseA;                /* Thread id of thread: phase_a      */
osThreadId_t tid_phaseB;                /* Thread id of thread: phase_b      */
osThreadId_t tid_phaseC;                /* Thread id of thread: phase_c      */
osThreadId_t tid_phaseD;                /* Thread id of thread: phase_d      */
osThreadId_t tid_clock;                 /* Thread id of thread: clock        */

struct phases_t {
  int_fast8_t phaseA;
  int_fast8_t phaseB;
  int_fast8_t phaseC;
  int_fast8_t phaseD;
} g_phases;


/*----------------------------------------------------------------------------
 *      Switch LED on
 *---------------------------------------------------------------------------*/
void Switch_On (unsigned char led) {
  printf("LED On:  #%d\n\r", led);
}

/*----------------------------------------------------------------------------
 *      Switch LED off
 *---------------------------------------------------------------------------*/
void Switch_Off (unsigned char led) {
  printf("LED Off: #%d\n\r", led);
}


/*----------------------------------------------------------------------------
 *      Function 'signal_func' called from multiple threads
 *---------------------------------------------------------------------------*/
void signal_func (osThreadId_t tid)  {
  osThreadFlagsSet(tid, 0x0001);              /* set signal to thread 'thread' */
}

/*----------------------------------------------------------------------------
 *      Thread 1 'phaseA': Phase A output
 *---------------------------------------------------------------------------*/
void phaseA (void *argument) {
  for (;;) {
    osThreadFlagsWait(0x0001, osFlagsWaitAny ,osWaitForever); /*informa la cpu che il flag è stato soddisfatto e rimette di nuovo il task in attesa dell'evento 0x0001 quindi se risetto la flag A è di nuovo chiamabile*/
    g_phases.phaseA = 1; /*alzo il segnale*/
    osDelay(1500); /*A (BLOCKED) per 1500ms*/
    signal_func(tid_phaseB); /*triggero B NON AVVIENE CONTEXT SWITCHING IMMEDIATO ci vorrà un pò di tempo per cui prima che B venga chiamato viene prima abbassato A*/
    g_phases.phaseA = 0; /* A basso*/
    osDelay(1500);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 2 'phaseB': Phase B output
 *---------------------------------------------------------------------------*/
void phaseB (void *argument) {
  for (;;) {
    osThreadFlagsWait(0x0001, osFlagsWaitAny, osWaitForever);    /* wait for an event flag 0x0001 */
    g_phases.phaseB = 1;  
    osDelay(1500);
    signal_func(tid_phaseA);                /* call common signal function   */
    g_phases.phaseB = 0;
    osDelay(1500);
  }
}

/*----------------------------------------------------------------------------
 *      Thread 5 'clock': Signal Clock
 *---------------------------------------------------------------------------*/
void clock (void *argument) {
  for (;;) {
    osThreadFlagsWait(0x0100, osFlagsWaitAny, osWaitForever);    /* wait for an event flag 0x0100 */
    osDelay(80);                            /* delay  80ms                   */
  }
}

/*----------------------------------------------------------------------------
 *      Main: Initialize and start RTX Kernel
 *---------------------------------------------------------------------------*/
void app_main (void *argument) {

  tid_phaseA = osThreadNew(phaseA, NULL, NULL);
  tid_phaseB = osThreadNew(phaseB, NULL, NULL);
  //tid_phaseC = osThreadNew(phaseC, NULL, NULL);
  //tid_phaseD = osThreadNew(phaseD, NULL, NULL);
  //tid_clock  = osThreadNew(clock,  NULL, NULL);

  osThreadFlagsSet(tid_phaseA, 0x0001);     /* set signal to phaseA thread   */

  osDelay(osWaitForever);
}

int main (void) {

  // System Initialization
  SystemCoreClockUpdate();
  // ...
  osKernelInitialize();                 // Initialize CMSIS-RTOS
  osThreadNew(app_main, NULL, NULL);    // Create application main thread
  if (osKernelGetState() == osKernelReady) {
    osKernelStart();                    // Start thread execution
  }

  while(1);
}
