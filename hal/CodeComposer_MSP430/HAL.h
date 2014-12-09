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

// Supported processors
#define COLDFIRE_V1		1
#define HCS08			2
#define MSP430			3
#define ATMEGA			4
#define PIC18			5


/// Define the used processor
#define PROCESSOR 		MSP430

/// Define if nesting interrupt is active
#define NESTING_INT 0

/// Define if its necessary to save status register / interrupt info
#if NESTING_INT == 1
  #define OS_SR_SAVE_VAR INT16U CPU_SR = 0;
#else
  #define OS_SR_SAVE_VAR
#endif

/// Define stack growth direction
#define STACK_GROWTH 0            /// 1 -> down; 0-> up


/// Define CPU Stack Pointer Size
#define SP_SIZE 16

/// Define CPU Type
#define OS_CPU_TYPE INT16U

extern INT8U iNesting;

extern INT16U SPvalue;                             ///< Used to save and restore a task stack pointer




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Port Defines                                /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/// Defines the change context command of the chosen processor
#define ChangeContext() SwitchContext()


/// Defines the disable interrupts command of the chosen microcontroller
#define UserEnterCritical() asm("	DINT	");
/// Defines the enable interrupts command of the choosen microcontroller
#define UserExitCritical()  asm("	EINT	");

#if (NESTING_INT == 0)
/// Defines the disable interrupts command of the chosen microcontroller
#define OSEnterCritical() UserEnterCritical()
/// Defines the enable interrupts command of the chosen microcontroller
#define OSExitCritical() UserExitCritical()
#endif

/// Defines the low power command of the chosen microcontroller
#define OS_Wait __bis_SR_register(CPUOFF + GIE)
/// Defines the tick timer interrupt handler code (clear flag) of the chosen microcontroller
#define TICKTIMER_INT_HANDLER 	
#define TIMER_MODULE 			TACCR0 
#define TIMER_COUNTER 			TAR
//#define IDLE_STACK_SIZE 		32


/* stacked by the RTI interrupt process */
// Mínimo de 60 devido ao salvamento de 12 registradores de 16 bits da CPU
// R4-R15 (24 bytes) + SR (12 bits) + SP (20 bits)
#define NUMBER_MIN_OF_STACKED_BYTES 28
/// Minimum size of a task stack.

// User defined: stacked for user function calls + local variables
// Ainda, como podem ocorrer interrupções durante as tarefas, alocar 28 bytes a cada
// interrupção ativa





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Functions Prototypes                        /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void CreateVirtualStack(void(*FctPtr)(void), INT16U NUMBER_OF_STACKED_BYTES);

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
// fix MSP430 sw isr
/*****************************************************************************************//**
* \fn void SwitchContext(void)
* \brief Switch context function (mimic SW ISR in MSP430)
* \return NONE
*********************************************************************************************/
void SwitchContext(void);



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/////                                                     /////
/////          MSP430 Without Nesting Defines             /////
/////                                                     /////
/////                                                     /////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Save Context Define                         /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#define SaveContext() 			asm("	PUSH.W	R10 \n\t" \
                                    "	PUSH.W	R9 \n\t" \
                                    "	PUSH.W 	R8 \n\t" \
                                    "	PUSH.W	R7 \n\t" \
                                    "	PUSH.W	R6 \n\t" \
                                    "	PUSH.W	R5 \n\t" \
                                    "	PUSH.W	R4 \n\t" )


#define OS_SAVE_CONTEXT() SaveContext()
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Save Stack Pointer Define                   /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

// save top of stack
#define SaveCurrentSP() 		asm("	MOV.W	R1,	&SPvalue \n\t") 

#define OS_SAVE_SP() SaveCurrentSP()
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/*****************************************************************************************//**
* \fn asm void RestoreContext(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/
#define RestoreSP() 		asm("	MOV.W	SPvalue,	R1 \n\t") 

/// Restore Stack Pointer Define
#define OS_RESTORE_SP() RestoreSP()
////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Restore Context Define                      /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// restore top of stack
// restore other CPU registers
// adjust stack pointer value
/*
									*/
#define RestoreContext() 		asm("	POP.W	R4 \n\t" \
                                    "	POP.W	R5 \n\t" \
                                    "	POP.W	R6 \n\t" \
                                    "	POP.W	R7 \n\t" \
                                    "	POP.W	R8 \n\t" \
									"	POP.W	R9 \n\t" \
									"	POP.W	R10 \n\t")

#define OS_RESTORE_CONTEXT() RestoreContext()
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


/*****************************************************************************************//**
* \fn OS_SAVE_ISR() and OS_RESTORE_ISR()
* \brief Used to mimic ISR call
* \return NONE
*********************************************************************************************/
#define OS_SAVE_ISR()            asm("	PUSH.W	R2 \n\t" \
                                     "	PUSH.W	R15 \n\t" \
                                     "	PUSH.W	R14 \n\t" \
                                     "	PUSH.W	R13 \n\t" \
                                     "	PUSH.W	R12 \n\t" \
                                     "	PUSH.W	R11 \n\t")
									 
#define OS_RESTORE_ISR() 		 asm("	POP.W	R11 \n\t" \
                                     "	POP.W	R12 \n\t" \
                                     "	POP.W	R13 \n\t" \
                                     "	POP.W	R14 \n\t" \
                                     "	POP.W	R15 \n\t" \
									 "	BIC	#240,	0(R1)\n\t" \
									 "	POP.W	R2 \n\t")
									 
/*********************************************************************************************/									 
#define CriticalDecNesting() iNesting--      	


 
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


#define BTOSStartFirstTask()      OS_RESTORE_SP();       \
                                  OS_RESTORE_CONTEXT();  \
                                  OS_RESTORE_ISR()



#endif
