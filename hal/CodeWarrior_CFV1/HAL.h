/**
* \file HAL.h
* \brief BRTOS Hardware Abstraction Layer defines and inline assembly
*
* This file contain the defines and inline assembly that are processor dependant.
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
*                                     OS HAL Header to Coldfire V1
*
*
*   Author:   Gustavo Weber Denardin
*   Revision: 1.0
*   Date:     20/03/2009
*
*   Authors:  Carlos Henrique Barriquelo e Gustavo Weber Denardin
*   Revision: 1.2
*   Date:     01/10/2010
*
*********************************************************************************************************/

#ifndef OS_HAL_H
#define OS_HAL_H

#include "OS_types.h"
#include "hardware.h"
#include "BRTOSConfig.h"


/// Supported processors
#define COLDFIRE_V1		1
#define HCS08			    2
#define MSP430			  3
#define ATMEGA			  4
#define PIC18			    5


/// Define the used processor
#define PROCESSOR 		COLDFIRE_V1

/// Define the CPU type
#define OS_CPU_TYPE 	INT32U

/// Define if the optimized scheduler will be used
#define OPTIMIZED_SCHEDULER 1

/// Define if nesting interrupt is active
#define NESTING_INT 1

//#define TASK_WITH_PARAMETERS 1  // configured in BRTOSConfig.h

/// Define if its necessary to save status register / interrupt info
#if NESTING_INT == 1
  #define OS_SR_SAVE_VAR INT16U CPU_SR = 0;
#else
  #define OS_SR_SAVE_VAR
#endif

/// Define stack growth direction
#define STACK_GROWTH 0            /// 1 -> down; 0-> up

/// Define CPU Stack Pointer Size
#define SP_SIZE 32


extern INT8U  iNesting;
extern INT32U SPvalue;





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Port Defines                                /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/// Defines the change context command of the choosen processor
#define ChangeContext(void) asm ("TRAP %14");


/// Defines the disable interrupts command of the choosen microcontroller
#define UserEnterCritical() DisableInterrupts;
/// Defines the enable interrupts command of the choosen microcontroller
#define UserExitCritical() EnableInterrupts;

#if (NESTING_INT == 0)
  
  /// Defines the disable interrupts command of the choosen microcontroller
  #define OSEnterCritical() UserEnterCritical()
  /// Defines the enable interrupts command of the choosen microcontroller
  #define OSExitCritical() UserExitCritical()
  
#else

  INT16U OS_CPU_SR_Save(void);
  #define  OSEnterCritical()  (CPU_SR = OS_CPU_SR_Save())	 // Disable interrupts    
  void OS_CPU_SR_Restore(INT16U);
  #define  OSExitCritical()  (OS_CPU_SR_Restore(CPU_SR))	 // Enable interrupts  
  
#endif

/// Defines the low power command of the choosen microcontroller
#define OS_Wait asm(STOP #0x2000);
/// Defines the tick timer interrupt handler code (clear flag) of the choosen microcontroller
#define TICKTIMER_INT_HANDLER TPM1SC_TOF = 0
#define TIMER_MODULE  TPM1MOD
#define TIMER_COUNTER TPM1CNT


// stacked by the RTI interrupt process
// Mínimo de 60 devido ao salvamento de 15 registradores de 32 bits da CPU
// D0-D7 (32 bytes) + A0-A6 (28 bytes) + Format (1 byte) + Vector (1 byte) + SR (2 bytes) + SP (4 bytes)
/// Minimum size of a task stack.
/// Example in the coldfire processor: D0-D7 (32 bytes) + A0-A6 (28 bytes) + Format (1 byte) + Vector (1 byte) + SR (2 bytes) + SP (4 bytes) = 68 bytes
#if (NESTING_INT == 1)
#define NUMBER_MIN_OF_STACKED_BYTES 72
#else
#define NUMBER_MIN_OF_STACKED_BYTES 68
#endif

// User defined: stacked for user function calls + local variables
// Ainda, como podem ocorrer interrupções durante as tarefas, alocar 28 bytes a cada
// interrupção ativa
// 4 bytes to Local Variable Allocation
// 4 bytes to Function Call





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Functions Prototypes                        /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#if (TASK_WITH_PARAMETERS == 1)
  void CreateVirtualStack(void(*FctPtr)(void*), INT16U NUMBER_OF_STACKED_BYTES, void *parameters);
#else
  void CreateVirtualStack(void(*FctPtr)(void), INT16U NUMBER_OF_STACKED_BYTES);
#endif

/*****************************************************************************************//**
* \fn void TickTimerSetup(void)
* \brief Tick timer clock setup
* \return NONE
*********************************************************************************************/
void TickTimerSetup(void);

/*****************************************************************************************//**
* \fn void OSRTCSetup(void)
* \brief Real time clock setup
* \return NONE
*********************************************************************************************/
void OSRTCSetup(void);

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////




///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/////                                                     /////
/////               Coldfire Nesting Defines              /////
/////                                                     /////
/////                                                     /////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////

#if (NESTING_INT == 1)

/*****************************************************************************************//**
* \fn inline asm void SaveContext(void)
* \brief Save context function
* \return NONE
*********************************************************************************************/
inline asm __declspec(register_abi) void SaveContext(void) 
{  
	LEA		   -36(A7),A7				    // reserve space on current stack
	MOVEM.L  D3-D7/A2-A6,(A7)			// save CPU registers
}

/// Save Context Define
#define OS_SAVE_CONTEXT() SaveContext()
////////////////////////////////////////////////////////////


/*****************************************************************************************//**
* \fn inline asm void RestoreContext(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/
inline asm __declspec(register_abi) void RestoreContext(void)
{  	
	MOVEM.L (A7),D3-D7/A2-A6  			// restore other CPU registers
	LEA		  36(A7),A7			     	    // adjust stack pointer value
}

/// Restore Context Define
#define OS_RESTORE_CONTEXT() RestoreContext()
////////////////////////////////////////////////////////////



/*****************************************************************************************//**
* \fn inline asm void SaveSP(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/
inline asm __declspec(register_abi) void SaveSP(void)
{  
	MOVE    A7,SPvalue              // save top of stack
}

/// Save Stack Pointer Define
#define OS_SAVE_SP() SaveSP()
////////////////////////////////////////////////////////////

/*****************************************************************************************//**
* \fn inline asm void RestoreContext(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/
inline asm __declspec(register_abi) void RestoreSP(void)
{  
	MOVE    SPvalue,A7              // restore top of stack
}

/// Restore Stack Pointer Define
#define OS_RESTORE_SP() RestoreSP()
////////////////////////////////////////////////////////////




/*****************************************************************************************//**
* \fn inline asm void RestoreSR(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/
inline asm __declspec(register_abi) void RestoreSR(void)
{  
    MOVE.W   SR,D1
    MOVE.W   26(A7),D0
    ANDI.L   #0x0700,D0    
    ANDI.L   #0xF8FF,D1
    OR       D1,D0
    MOVE.W   D0,SR
}

/// Restore Status Register Define
#define OS_ENABLE_NESTING() RestoreSR()
////////////////////////////////////////////////////////////




inline asm __declspec(register_abi) void RestoreIntSR(void)
{
    //MOVE.W   D2,SR
    NOP
}

/// Restore Status Register from ISR
#define OS_RESTORE_NESTING() RestoreIntSR()





inline asm __declspec(register_abi) CriticalDecNesting(void)
{                                                                     
        MOVE.W   SR,D2
        ORI.L    #0x0700,D2
        MOVE.W   D2,SR
        MVZ.B    iNesting,D0
        SUBQ.L   #1,D0
        MOVE.B   D0,iNesting
}



inline asm __declspec(register_abi) void OSRestoreISR(void) {

     LEA              4(A7),A7                             // adjust stack pointer value
     MOVEM.L          (A7),D0-D2/A0-A1                     // restore CPU scratch registers
     LEA              24(A7),A7                            // adjust stack pointer value
     RTE
}

#define OS_RESTORE_ISR()  OSRestoreISR()



  
#else





///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/////                                                     /////
/////          Coldfire Without Nesting Defines           /////
/////                                                     /////
/////                                                     /////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Save Context Define                         /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

inline asm __declspec(register_abi) void SaveContext(void) 
{  
	LEA		   -40(A7),A7				    // reserve space on current stack
	MOVEM.L  D3-D7/A2-A6,(A7)			// save CPU registers
}

#define OS_SAVE_CONTEXT() SaveContext()
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Restore Context Define                      /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

inline asm __declspec(register_abi) void RestoreContext(void)
{  
	MOVEM.L (A7),D3-D7/A2-A6  			// restore other CPU registers
	LEA		  40(A7),A7			     	    // adjust stack pointer value
}

#define OS_RESTORE_CONTEXT() RestoreContext()
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Save Stack Pointer Define                   /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

inline asm __declspec(register_abi) void OS_SAVE_SP(void) 
{  
	MOVE     A7,SPvalue           // save top of stack
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Save Stack Pointer Define                   /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

inline asm __declspec(register_abi) void OS_RESTORE_SP(void)
{  
	MOVE    SPvalue,A7              // restore top of stack
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////










inline asm __declspec(register_abi) CriticalDecNesting(void)
{                                                                     
        MVZ.B    iNesting,D0
        SUBQ.L   #1,D0
        MOVE.B   D0,iNesting
}


inline asm __declspec(register_abi) void OSRestoreISR(void) {

     MOVEM.L          (A7),D0-D2/A0-A1                     // restore CPU scratch registers
     LEA              20(A7),A7                            // adjust stack pointer value
     RTE
}

#define OS_RESTORE_ISR()  OSRestoreISR()


#endif


#if (OPTIMIZED_SCHEDULER == 1)

#define Optimezed_Scheduler()   asm                      \
                                {                        \
                                  FF1 D0                 \
                                  NEG D0                 \
                                  ADDI #31,D0            \
                                }

#endif



#define BTOSStartFirstTask()      OS_RESTORE_SP();       \
                                  OS_RESTORE_CONTEXT();  \
                                  OS_RESTORE_ISR()



#endif
