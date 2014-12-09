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
#include "interrupt_handlers.h"

#pragma warn_implicitconv off
#pragma warn_unusedarg off


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
    INT32U sys_clk;
    INT32U per_clk;
    INT32U per_div;
	INT16U cmcor;	

    sys_clk =  SYSTEM.SCKCR.LONG;
    per_div = (sys_clk >> 8u) & 0xFu;
    if (per_div <= 3u)
	{
		per_clk =  configCPU_CLOCK_HZ << (3u - per_div);
		
		cmcor = per_clk / (32u * configTICK_RATE_HZ);

	    MSTP(CMT0) = 0;                                             /* Enable CMT0 module.                                  */

	    CMT.CMSTR0.BIT.STR0 = 0;                                    /* Stop timer channel 0.                                */
	    CMT0.CMCR.BIT.CKS   = 1;                                    /* Set peripheral clock divider.                        */

	    CMT0.CMCOR = cmcor - 1u;                                    /* Set compare-match value.                             */
	    CMT0.CMCNT = 0;                                             /* Clear counter register.                              */

	    IR(CMT0, CMI0)  = 0;                                        /* Clear any pending ISR.                               */
	    IPR(CMT0,)      = 3;                                        /* Set interrupt priority.                              */
	    IEN(CMT0, CMI0) = 1;                                        /* Enable interrupt source.                             */

	    CMT0.CMCR.BIT.CMIE = 1;                                     /* Enable interrupt.                                    */

	    CMT.CMSTR0.BIT.STR0 = 1;                                    /* Start timer.                                         */		
    }
    
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
  OS_SAVE_ISR();
  OS_INT_ENTER();
  
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
  OS_INT_EXIT();
  OS_RESTORE_ISR();
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
void SwitchContext(void)
{
  // ************************
  // Entrada de interrupção
  // ************************
  OS_SAVE_ISR();
  OS_INT_ENTER();

  // Interrupt Handling
  
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





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////  Task Installation Function                      /////
/////                                                  /////
/////  Parameters:                                     /////
/////  Function pointer, task priority and task name   /////
/////                                                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


void CreateVirtualStack(void(*FctPtr)(void), INT16U NUMBER_OF_STACKED_BYTES)
{  
   OS_CPU_TYPE *stk_pt = (OS_CPU_TYPE*)&STACK[iStackAddress + (NUMBER_OF_STACKED_BYTES / sizeof(OS_CPU_TYPE))];
   // Init PSW Register
   *--stk_pt = (INT32U)PSW_INIT;
   // Task Pointer
   *--stk_pt = (INT32U)FctPtr;
   // FPSW
   *--stk_pt = 0x00000100u;
   // General Purpose Registers
   *--stk_pt = 0x15151515L;
   *--stk_pt = 0x14141414L;
   *--stk_pt = 0x13131313L;
   *--stk_pt = 0x12121212L;
   *--stk_pt = 0x11111111L;
   *--stk_pt = 0x10101010L;
   *--stk_pt = 0x09090909L;
   *--stk_pt = 0x08080808L;
   *--stk_pt = 0x07070707L;
   *--stk_pt = 0x06060606L;
   *--stk_pt = 0x05050505L;
   *--stk_pt = 0x04040404L;
   *--stk_pt = 0x03030303L;
   *--stk_pt = 0x02020202L;
   *--stk_pt = 0x00000000L;
   *--stk_pt = 0x00009ABCu;
   *--stk_pt = 0x12345678u;
}




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
