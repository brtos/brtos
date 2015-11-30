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
#include "hardware.h"
#include "BRTOS.h"


#if !__GNUC__
#pragma warn_implicitconv off
#pragma warn_unusedarg off
#endif


#if (SP_SIZE == 32)
  INT32U SPvalue;             ///< Used to save and restore a task stack pointer
#endif

#if (SP_SIZE == 16)
  INT16U SPvalue;             ///< Used to save and restore a task stack pointer
#endif

  
  //////////////// ISR dedicated stack /////////////////

  #if (defined ISR_DEDICATED_STACK && ISR_DEDICATED_STACK == 1)
  	OS_CPU_TYPE ISR_STACK[ISR_STACK_SIZE]; //= {0x49535253,0x5441434B};
  	INT32U SPval_bkp = (INT32U)&ISR_STACK[ISR_STACK_SIZE-2];
  #endif



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      OS Tick Timer Setup                         /////
/////                                                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void TickTimerSetup(void)
{
  
  /* ### Init_TPM init code */
  /* TPM1SC: TOF=0,TOIE=0,CPWMS=0,CLKSB=0,CLKSA=0,PS2=0,PS1=0,PS0=0 */
  TPM1SC = 0x00;                       /* Stop and reset counter */
  TPM1MOD = (configCPU_CLOCK_HZ / configTICK_RATE_HZ) >> configTIMER_PRE_SCALER; /* Period value setting */
  (void)(TPM1SC == 0);                 /* Overflow int. flag clearing (first part) */
  /* TPM1SC: TOF=0,TOIE=1,CPWMS=0,CLKSB=0,CLKSA=1,PS2=0,PS1=0,PS0=0 */
  TPM1SC = 0x48 | configTIMER_PRE_SCALER;                       /* Int. flag clearing (2nd part) and timer control register setting */
  
  /* ### */
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
  /* ### Init_RTC init code */
  /* RTCMOD: RTCMOD=0x63 */
  // Cristal de Referência = 1Khz
  //RTCMOD = (configRTC_CRISTAL_HZ / configRTC_PRE_SCALER);
  /* RTCSC: RTIF=1,RTCLKS=0,RTIE=1,RTCPS=0x0B */
  //RTCSC = 0x1B;                                      
  /* ### */

  //OSResetTime(&Hora);
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#if !__GNUC__
#if (NESTING_INT == 1)
#pragma TRAP_PROC
#else
interrupt
#endif
#else
__attribute__ ((__optimize__("omit-frame-pointer")))
#endif
void TickTimer(void)
{

#if __GNUC__
	OS_SAVE_ISR();
#endif

  // ************************
  // Entrada de interrupção
  // ************************
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
  // ************************  

#if __GNUC__
	OS_RESTORE_ISR();
#endif
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
#if !__GNUC__
#if (NESTING_INT == 1)
#pragma TRAP_PROC
#else
interrupt
#endif
#else
__attribute__ ((__optimize__("omit-frame-pointer")))
#endif
void SwitchContext(void)
{

#if __GNUC__
	OS_SAVE_ISR();
#endif

	OS_INT_SCHED();

#if __GNUC__
	OS_RESTORE_ISR();
#endif
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

#if (TASK_WITH_PARAMETERS == 1)
  void CreateVirtualStack(void(*FctPtr)(void*), INT16U NUMBER_OF_STACKED_BYTES, void *parameters)
#else
  void CreateVirtualStack(void(*FctPtr)(void), INT16U NUMBER_OF_STACKED_BYTES)
#endif
{  
   OS_CPU_TYPE *stk_pt = (OS_CPU_TYPE*)&STACK[iStackAddress + (NUMBER_OF_STACKED_BYTES / sizeof(OS_CPU_TYPE))];
   //INT32U *stk_pt = (INT32U*)&STACK[iStackAddress + NUMBER_OF_STACKED_BYTES];
   
#if (defined STACK_MARK && STACK_MARK == 1)
   *--stk_pt = (INT32U)(('TSK'<<8) + NumberOfInstalledTasks + '0');
#endif   
   
   // Pointer to Task Entry
   *--stk_pt = (INT32U)FctPtr;

   // First 4 bytes defined to Coldfire Only
   // Format: First 4 bits = processor indicating a two-longword frame, always 0x04 in MCF51QE
   //         Other 4 bits = fault status field, always 0x00 if no error occurred
   // Vector
   // The 8-bit vector number, vector[7:0], defines the exception type and is
   // calculated by the processor for all internal faults and represents the
   // value supplied by the interrupt controller in the case of an interrupt   
   
   // Initial SR Register
   // Interrupts Enabled
   // CCR = 0x00     
		   
   *--stk_pt = (INT32U)0x40802000;
   
   #if (NESTING_INT == 1)  
   
   // Initialize registers   
   *--stk_pt = (INT32U)0x00;    // Save Int level
   
   *--stk_pt = (INT32U)0xA1;

#if (TASK_WITH_PARAMETERS == 1)   
   *--stk_pt = (INT32U)parameters;
#else
   *--stk_pt = (INT32U)0xA0;
#endif

   *--stk_pt = (INT32U)0xD2;
   *--stk_pt = (INT32U)0xD1;
   *--stk_pt = (INT32U)0xD0;
   
   *--stk_pt = (INT32U)0xA6;
   *--stk_pt = (INT32U)0xA5;
   *--stk_pt = (INT32U)0xA4;
   *--stk_pt = (INT32U)0xA3;
   *--stk_pt = (INT32U)0xA2;
   
   *--stk_pt = (INT32U)0xD7;   
   *--stk_pt = (INT32U)0xD6;   
   *--stk_pt = (INT32U)0xD5;   
   *--stk_pt = (INT32U)0xD4;   
   *--stk_pt = (INT32U)0xD3;               
   
   #else
   
   // Initialize registers
   *--stk_pt = (INT32U)0xA1;
   
#if (TASK_WITH_PARAMETERS == 1)   
   *--stk_pt = (INT32U)parameters;
#else
   *--stk_pt = (INT32U)0xA0;
#endif
   
   *--stk_pt = (INT32U)0xD2;
   *--stk_pt = (INT32U)0xD1;
   *--stk_pt = (INT32U)0xD0;
   
   *--stk_pt = (INT32U)0xA6;
   *--stk_pt = (INT32U)0xA5;
   *--stk_pt = (INT32U)0xA4;
   *--stk_pt = (INT32U)0xA3;
   *--stk_pt = (INT32U)0xA2;
   
   *--stk_pt = (INT32U)0xD7;   
   *--stk_pt = (INT32U)0xD6;   
   *--stk_pt = (INT32U)0xD5;   
   *--stk_pt = (INT32U)0xD4;   
   *--stk_pt = (INT32U)0xD3;
   
   #endif
}





#if (NESTING_INT == 1)

INT16U OS_CPU_SR_Save(void)
{
	INT16U cpu_sr;
	__asm__ volatile  (	"MOVE.W   %/sr,%d0 	   \n\t" \
						"MOVE.L   %d0,%/sp@-   \n\t" \
						"ORI.L    #0x0700,%d0  \n\t" \
						"MOVE.W   %d0,%/sr 	   \n\t" \
						"MOVE.L   %/sp@+,%d0   \n\t" \
						: "=d"   (cpu_sr)            );

	return cpu_sr;
}



void OS_CPU_SR_Restore(INT16U SR)
{  
  __asm volatile ("MOVE.W    %0,%/sr\n\t" : : "d" (SR) );
}

  

#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
