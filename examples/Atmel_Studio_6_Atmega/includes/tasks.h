
/*********************************************************************************************************
*                                               BRTOS
*                                Brazilian Real-Time Operating System
*                            Acronymous of Basic Real-Time Operating System
*
*                              
*                                  Open Source RTOS under MIT License
*
*
*
*                                              OS Tasks
*
*
*   Author:   Gustavo Weber Denardin
*   Revision: 1.0
*   Date:     20/03/2009
*
*********************************************************************************************************/

#include "hardware.h"

void System_Time(void);
void Task_2(void);
void Task_3(void);
void Task_Serial(void);


void Transmite_Uptime(void);
void Transmite_Duty_Cycle(void);
void Transmite_RAM_Ocupada(void);
void Transmite_Task_Stacks(void);
void Transmite_CPU_Load(void);
void Reason_of_Reset(void);