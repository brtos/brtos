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
#include "iodefine.h"


/// Supported processors
#define COLDFIRE_V1		1
#define HCS08			2
#define MSP430			3
#define ATMEGA			4
#define PIC18			5
#define RX600			6


/// Define the used processor
#define PROCESSOR 		RX600

/// Define the CPU type
#define OS_CPU_TYPE 	INT32U

#define PSW_INIT     	0x00010000u

/// Define if nesting interrupt is active
#define NESTING_INT 1

/// Define if its necessary to save status register / interrupt info
#if NESTING_INT == 1
  #define OS_SR_SAVE_VAR INT32U CPU_SR = 0;
#else
  #define OS_SR_SAVE_VAR
#endif

/// Define stack growth direction
#define STACK_GROWTH 0            /// 1 -> down; 0-> up

/// Define CPU Stack Pointer Size
#define SP_SIZE 32


extern INT8U iNesting;
extern INT32U SPvalue;





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Port Defines                                /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/// Defines the change context command of the choosen processor
#define ChangeContext(void) asm ("INT #0x01");


void   set_ipl(INT32U ipl);
INT32U get_ipl(void);
INT32U get_set_ipl(INT32U ipl);

/// Defines the disable interrupts command of the choosen microcontroller
#define UserEnterCritical()	set_ipl(12);
/// Defines the enable interrupts command of the choosen microcontroller
#define UserExitCritical()  set_ipl(0);

/// Defines the disable interrupts command of the choosen microcontroller
//#define OSEnterCritical() CPU_SR = get_ipl(); set_ipl(15)
#define OSEnterCritical() CPU_SR = get_set_ipl(12)

/// Defines the enable interrupts command of the choosen microcontroller
#define OSExitCritical()  set_ipl(CPU_SR)


/// Defines the low power command of the choosen microcontroller
#define OS_Wait	asm("WAIT");
/// Defines the tick timer interrupt handler code (clear flag) of the choosen microcontroller
#define TICKTIMER_INT_HANDLER
#define TIMER_MODULE	CMT0.CMCOR
#define TIMER_COUNTER	CMT0.CMCNT


#define RESET_WATCHDOG()    IWDT.IWDTRR = 0x00; IWDT.IWDTRR = 0xFF


// stacked by the RTI interrupt process
// Mínimo de 60 devido ao salvamento de 15 registradores de 32 bits da CPU
// D0-D7 (32 bytes) + A0-A6 (28 bytes) + Format (1 byte) + Vector (1 byte) + SR (2 bytes) + SP (4 bytes)
/// Minimum size of a task stack.
/// Example in the coldfire processor: D0-D7 (32 bytes) + A0-A6 (28 bytes) + Format (1 byte) + Vector (1 byte) + SR (2 bytes) + SP (4 bytes) = 68 bytes

#define NUMBER_MIN_OF_STACKED_BYTES 80


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

/*****************************************************************************************//**
* \fn inline asm void SaveContext(void)
* \brief Save context function
* \return NONE
*********************************************************************************************/

/// Save Context Define
#define OS_SAVE_CONTEXT()
////////////////////////////////////////////////////////////


	
	


/*****************************************************************************************//**
* \fn inline asm void RestoreContext(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/

/// Restore Context Define
#define OS_RESTORE_CONTEXT()
////////////////////////////////////////////////////////////



/*****************************************************************************************//**
* \fn inline asm void SaveSP(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/

/// Save Stack Pointer Define
#define OS_SAVE_SP()		asm("MOV.L   #_SPvalue, R2 	\n\t" \
								"MOV.L   R0, [R2] 		\n\t" )
								
////////////////////////////////////////////////////////////



/*****************************************************************************************//**
* \fn inline asm void RestoreContext(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/

/// Restore Stack Pointer Define
#define OS_RESTORE_SP()		asm("MOV.L   #_SPvalue, R2 	\n\t" \
								"MOV.L   [R2], R0 		\n\t" )
////////////////////////////////////////////////////////////




/*****************************************************************************************//**
* \fn inline asm void RestoreSR(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/

/// Restore Status Register Define
#define OS_ENABLE_NESTING()	__asm volatile( "SETPSW	I" );
	/*asm("PUSH R1 			\n\t" \
									"PUSH R2 			\n\t" \
									"MVFC PSW, R2 		\n\t" \
									"MOV #0x10000, R1 	\n\t" \
									"OR   R1, R2 		\n\t" \
									"MVTC R2, PSW	 	\n\t" \
									"POP  R2	 		\n\t" \
									"POP  R1 			\n\t" )*/
////////////////////////////////////////////////////////////


#define CriticalDecNesting() set_ipl(12); iNesting--



#define OS_SAVE_ISR()		asm("PUSHC   FPSW 		\n\t" \
								"PUSHM   R1-R15 	\n\t" \
								"MVFACHI R1 		\n\t" \
								"MVFACMI R2 		\n\t" \
								"PUSHM   R1-R2 		\n\t")

#define OS_RESTORE_ISR()	asm("POPM    R1-R2 		\n\t" \
								"SHLL    #16, R2 	\n\t" \
								"MVTACLO R2 		\n\t" \
								"MVTACHI R1 		\n\t" \
								"POPM    R1-R15 	\n\t" \
								"POPC    FPSW	 	\n\t" \
								"RTE 				\n\t" )



                                                                /* ------------------- BIT DEFINES -------------------- */
#define  DEF_BIT_NONE                                   0x00uL

#define  DEF_BIT_00                                     0x01uL
#define  DEF_BIT_01                                     0x02uL
#define  DEF_BIT_02                                     0x04uL
#define  DEF_BIT_03                                     0x08uL
#define  DEF_BIT_04                                     0x10uL
#define  DEF_BIT_05                                     0x20uL
#define  DEF_BIT_06                                     0x40uL
#define  DEF_BIT_07                                     0x80uL

#define  DEF_BIT_08                                   0x0100uL
#define  DEF_BIT_09                                   0x0200uL
#define  DEF_BIT_10                                   0x0400uL
#define  DEF_BIT_11                                   0x0800uL
#define  DEF_BIT_12                                   0x1000uL
#define  DEF_BIT_13                                   0x2000uL
#define  DEF_BIT_14                                   0x4000uL
#define  DEF_BIT_15                                   0x8000uL

#define  DEF_BIT_16                               0x00010000uL
#define  DEF_BIT_17                               0x00020000uL
#define  DEF_BIT_18                               0x00040000uL
#define  DEF_BIT_19                               0x00080000uL
#define  DEF_BIT_20                               0x00100000uL
#define  DEF_BIT_21                               0x00200000uL
#define  DEF_BIT_22                               0x00400000uL
#define  DEF_BIT_23                               0x00800000uL

#define  DEF_BIT_24                               0x01000000uL
#define  DEF_BIT_25                               0x02000000uL
#define  DEF_BIT_26                               0x04000000uL
#define  DEF_BIT_27                               0x08000000uL
#define  DEF_BIT_28                               0x10000000uL
#define  DEF_BIT_29                               0x20000000uL
#define  DEF_BIT_30                               0x40000000uL
#define  DEF_BIT_31                               0x80000000uL



#define BTOSStartFirstTask()      OS_RESTORE_SP();       \
                                  OS_RESTORE_CONTEXT();  \
                                  OS_RESTORE_ISR()
                                  
                                  

#endif
