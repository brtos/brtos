/**
* \file HAL.c
* \brief BRTOS Hardware Abstraction Layer Functions.
*
* This file contain the functions that are processor dependant.
*
*
**/

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
*                                   OS HAL Functions to Coldfire V1
*
*
*   Author:   Gustavo Weber Denardin
*   Revision: 1.0
*   Date:     20/03/2009
*
*********************************************************************************************************/

#include "BRTOS.h"


#if (SP_SIZE == 32)
  INT32U SPvalue;                             ///< Used to save and restore a task stack pointer
#endif

#if (SP_SIZE == 16)
  INT16U SPvalue;                             ///< Used to save and restore a task stack pointer
#endif




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      OS Tick Timer Setup                         /////
/////                                                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void TickTimerSetup(void)
{
	INT32U 		module  = configCPU_CLOCK_HZ / (INT32U)configTICK_RATE_HZ;
	
	*(NVIC_SYSTICK_CTRL) = 0;			// Disable Sys Tick Timer
    *(NVIC_SYSTICK_LOAD) = module - 1u;	// Set tick timer module
    *(NVIC_SYSTICK_CTRL) = NVIC_SYSTICK_CLK | NVIC_SYSTICK_INT | NVIC_SYSTICK_ENABLE;
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      OS RTC Setup                                /////
/////                                                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void OSRTCSetup(void)
{  
 
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
void TickTimer(void)
{
  // ************************
  // Entrada de interrupção
  // ************************
  
  // Interrupt handling
  TICKTIMER_INT_HANDLER;

  OSIncCounter();
  
  // BRTOS TRACE SUPPORT
  #if (OSTRACE == 1) 
      #if(OS_TICK_SHOW == 1) 
          #if(OS_TRACE_BY_TASK == 1)
          Update_OSTrace(0, ISR_TICK);
          #else
          Update_OSTrace(configMAX_TASK_INSTALL - 1, ISR_TICK);
          #endif         
      #endif       
  #endif  

  #if (NESTING_INT == 1)
  OS_ENABLE_NESTING();
  #endif   
    
  // ************************
  // Handler code for the tick
  // ************************
  OS_TICK_HANDLER();
  
  // ************************
  // Interrupt Exit
  // ************************
  OS_INT_EXIT_EXT();
  // ************************  
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////   Software Interrupt to provide Switch Context   /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
 
/************************************************************//**
* \fn interrupt void SwitchContext(void)
* \brief Software interrupt handler routine (Internal kernel function).
*  Used to switch the tasks context.
****************************************************************/

__attribute__ ((naked)) void SwitchContext(void)
{
  // ************************
  // Entrada de interrupção
  // ************************
  OS_SAVE_ISR();

  // Interrupt Handling
  Clear_PendSV();

  // ************************
  // Interrupt Exit
  // ************************
  OS_INT_EXIT();
  OS_RESTORE_ISR();
  // ************************
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


__attribute__ ((naked)) void SwitchContextToFirstTask(void)
{
	/* Make PendSV and SysTick the lowest priority interrupts. */
	*(NVIC_SYSPRI3) |= NVIC_PENDSV_PRI;
	*(NVIC_SYSPRI3) |= NVIC_SYSTICK_PRI;
	OS_RESTORE_SP();
	OS_RESTORE_CONTEXT();
	OS_RESTORE_ISR();
}



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////  Task Installation Function                      /////
/////                                                  /////
/////  Parameters:                                     /////
/////  Function pointer, task priority and task name   /////
/////                                                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void          OS_TaskReturn             (void);

#if (!BRTOS_DYNAMIC_TASKS_ENABLED)
#if (TASK_WITH_PARAMETERS == 1)
  void CreateVirtualStack(void(*FctPtr)(void*), INT16U NUMBER_OF_STACKED_BYTES, void *parameters)
#else
  void CreateVirtualStack(void(*FctPtr)(void), INT16U NUMBER_OF_STACKED_BYTES)
#endif
{  
	#ifdef WATERMARK
	OS_CPU_TYPE *temp_stk_pt = (OS_CPU_TYPE*)&STACK[iStackAddress];
	
	*temp_stk_pt++ = (INT32U)(((NumberOfInstalledTasks + '0') << 24) + 'T' + ('S' << 8) + ('K' << 16));
	#endif
	
	OS_CPU_TYPE *stk_pt = (OS_CPU_TYPE*)&STACK[iStackAddress + (NUMBER_OF_STACKED_BYTES / sizeof(OS_CPU_TYPE))];
	
	*--stk_pt = (INT32U)INITIAL_XPSR;                   	/* xPSR                                                   */

    *--stk_pt = (INT32U)FctPtr;                             /* Entry Point                                            */
    /// ??????????????????????
    *--stk_pt = 0;                      					/* R14 (LR)                                               */
    /// ??????????????????????
    *--stk_pt = (INT32U)0x12121212u;                        /* R12                                                    */
    *--stk_pt = (INT32U)0x03030303u;                        /* R3                                                     */
    *--stk_pt = (INT32U)0x02020202u;                        /* R2                                                     */
    //*--stk_pt = (INT32U)p_stk_limit;                        /* R1                                                     */
	*--stk_pt = (INT32U)(NUMBER_OF_STACKED_BYTES / 10);		/* R1                                                     */
   #if (TASK_WITH_PARAMETERS == 1)
	*--stk_pt = (INT32U)parameters;                         /* R0 : argument                                          */
   #else
	*--stk_pt = (INT32U)0;                              	/* R0 : argument                                          */
   #endif
#if (FPU_SUPPORT == 1)
    *--stk_pt = (INT32U)0xFFFFFFFDu;                        /* R14                                                    */
#endif
                                                            /* Remaining registers saved on process stack             */
    *--stk_pt = (INT32U)0x11111111u;                        /* R11                                                    */
    *--stk_pt = (INT32U)0x10101010u;                        /* R10                                                    */
    *--stk_pt = (INT32U)0x09090909u;                        /* R9                                                     */
    *--stk_pt = (INT32U)0x08080808u;                        /* R8                                                     */
    *--stk_pt = (INT32U)0x07070707u;                        /* R7                                                     */
    *--stk_pt = (INT32U)0x06060606u;                        /* R6                                                     */
    *--stk_pt = (INT32U)0x05050505u;                        /* R5                                                     */
    *--stk_pt = (INT32U)0x04040404u;                        /* R4                                                     */
    
	#ifdef WATERMARK
	do{
		*--stk_pt = 0x24242424;
	}while (stk_pt > temp_stk_pt);    
	#endif    
}
#endif

#if (BRTOS_DYNAMIC_TASKS_ENABLED == 1)
#if (TASK_WITH_PARAMETERS == 1)
  unsigned int CreateDVirtualStack(void(*FctPtr)(void*), unsigned int stk, void *parameters)
#else
  unsigned int CreateDVirtualStack(void(*FctPtr)(void), unsigned int stk)
#endif
{
	OS_CPU_TYPE *stk_pt = (OS_CPU_TYPE *)stk;

	*--stk_pt = (INT32U)INITIAL_XPSR;                   	/* xPSR                                                   */

    *--stk_pt = (INT32U)FctPtr;                             /* Entry Point                                            */
    /// ??????????????????????
    *--stk_pt = 0;                      					/* R14 (LR)                                               */
    /// ??????????????????????
    *--stk_pt = (INT32U)0x12121212u;                        /* R12                                                    */
    *--stk_pt = (INT32U)0x03030303u;                        /* R3                                                     */
    *--stk_pt = (INT32U)0x02020202u;                        /* R2                                                     */
	*--stk_pt = (INT32U)0x01010101u;						/* R1                                                     */
   #if (TASK_WITH_PARAMETERS == 1)
	*--stk_pt = (INT32U)parameters;                         /* R0 : argument                                          */
   #else
	*--stk_pt = (INT32U)0;                              	/* R0 : argument                                          */
   #endif
#if (FPU_SUPPORT == 1)
    *--stk_pt = (INT32U)0xFFFFFFFDu;                        /* R14                                                    */
#endif
                                                            /* Remaining registers saved on process stack             */
    *--stk_pt = (INT32U)0x11111111u;                        /* R11                                                    */
    *--stk_pt = (INT32U)0x10101010u;                        /* R10                                                    */
    *--stk_pt = (INT32U)0x09090909u;                        /* R9                                                     */
    *--stk_pt = (INT32U)0x08080808u;                        /* R8                                                     */
    *--stk_pt = (INT32U)0x07070707u;                        /* R7                                                     */
    *--stk_pt = (INT32U)0x06060606u;                        /* R6                                                     */
    *--stk_pt = (INT32U)0x05050505u;                        /* R5                                                     */
    *--stk_pt = (INT32U)0x04040404u;                        /* R4                                                     */

    return (unsigned int)stk_pt;
}
#endif






#if (NESTING_INT == 1)

INT32U OS_CPU_SR_Save(void)
{
	INT32U priority;
	__asm
	(
		    "MRS     %0, PRIMASK         \n"
		    "CPSID   I					 \n"
			: "=r"   (priority)
	);

	return priority;
}


void OS_CPU_SR_Restore(INT32U SR)
{
	__asm volatile ("MSR PRIMASK, %0\n\t" : : "r" (SR) );
}



#endif



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
