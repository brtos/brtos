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
*                                     OS HAL Header for ARM Cortex-M4
*
*
*
*   Authors:  Carlos Henrique Barriquelo e Gustavo Weber Denardin
*   Revision: 1.0
*   Date:     30/04/2011
*
*********************************************************************************************************/

#ifndef OS_HAL_H
#define OS_HAL_H

#include "OS_types.h"
#include "LPC11xx.h"                         /* include peripheral declarations */

/// Supported processors
#define COLDFIRE_V1		1u
#define HCS08			2u
#define MSP430			3u
#define ATMEGA			4u
#define PIC18			5u
#define RX600			6u
#define ARM_Cortex_M3	7u
#define ARM_Cortex_M4	8u
#define ARM_Cortex_M0	9u


/// Define the used processor
#define PROCESSOR 		ARM_Cortex_M0

#define OS_CPU_TYPE 	INT32U

/// Define if the optimized scheduler will be used
#define OPTIMIZED_SCHEDULER 0

/// Define if 32 bits register for tick timer will be used
#define TICK_TIMER_32BITS   1

/// Define if nesting interrupt is active
#define NESTING_INT 1

/// Define the Reset Watchdog macro
#define RESET_WATCHDOG()

/// Define if its necessary to save status register / interrupt info
#define OS_SR_SAVE_VAR INT32U CPU_SR = 0;

/// Define stack growth direction
#define STACK_GROWTH 0            /// 1 -> down; 0-> up

/// Define CPU Stack Pointer Size
#define SP_SIZE 32


extern INT8U iNesting;
extern INT32U SPvalue;



/* Constants required to set up the initial stack. */
#define INITIAL_XPSR					0x01000000

/* Cortex-M specific definitions. */
#define KERNEL_INTERRUPT_PRIORITY 		0xFF

/* Constants required to manipulate the NVIC PendSV */
#define NVIC_PENDSVSET      			0x10000000         			// Value to trigger PendSV exception.
#define NVIC_PENDSVCLR      			0x08000000         			// Value to clear PendSV exception.

#define NVIC_INT_CTRL       ((volatile unsigned long *)0xe000ed04)	// Interrupt control state register.
#define NVIC_SYSPRI3		((volatile unsigned long *)0xe000ed20)

// Kernel interrupt priorities
#define NVIC_PENDSV_PRI					( ( ( unsigned long ) KERNEL_INTERRUPT_PRIORITY ) << 16 )
#define NVIC_SYSTICK_PRI				( ( ( unsigned long ) KERNEL_INTERRUPT_PRIORITY ) << 24 )

// Constants required to manipulate the NVIC SysTick
#define NVIC_SYSTICK_CLK        		0x00000004
#define NVIC_SYSTICK_INT        		0x00000002
#define NVIC_SYSTICK_ENABLE     		0x00000001

#define NVIC_SYSTICK_CTRL   ((volatile unsigned long *)0xe000e010 )
#define NVIC_SYSTICK_LOAD   ((volatile unsigned long *)0xe000e014 )

unsigned short int _psp_swap2byte(unsigned short int n);
unsigned long int _psp_swap4byte(unsigned long int n);

#define _PSP_SWAP2BYTE(n)   _psp_swap2byte(n)
#define _PSP_SWAP4BYTE(n)   _psp_swap4byte(n)



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Port Defines                                /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#define ChangeContext()		*(NVIC_INT_CTRL) = NVIC_PENDSVSET;	\
							__asm volatile("CPSIE I")

#define Clear_PendSV(void)	*(NVIC_INT_CTRL) = NVIC_PENDSVCLR

#define OS_INT_EXIT_EXT()	*(NVIC_INT_CTRL) = NVIC_PENDSVSET


INT32U OS_CPU_SR_Save(void);
#define  OSEnterCritical() (CPU_SR = OS_CPU_SR_Save())	 // Disable interrupts    
void OS_CPU_SR_Restore(INT32U);
#define  OSExitCritical()  (OS_CPU_SR_Restore(CPU_SR))	 // Enable interrupts

/// Defines the disable interrupts command of the choosen microcontroller
#define UserEnterCritical() __asm("CPSID I")
/// Defines the enable interrupts command of the choosen microcontroller
#define UserExitCritical()  __asm("CPSIE I")

/// Defines the low power command of the choosen microcontroller
#define OS_Wait __asm("WFI");
/// Defines the tick timer interrupt handler code (clear flag) of the choosen microcontroller
#define TICKTIMER_INT_HANDLER
#define TIMER_MODULE  SysTick->LOAD
#define TIMER_COUNTER SysTick->VAL


// stacked by the RTI interrupt process
#define NUMBER_MIN_OF_STACKED_BYTES 64






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




/*****************************************************************************************//**
* \fn inline asm void SaveContext(void)
* \brief Save context function
* \return NONE
*********************************************************************************************/
/// Save Context Define
#define OS_SAVE_CONTEXT() __asm(									\
								  "MRS     R0,PSP			\n"		\
								  "SUB     R0, R0, #0x10	\n"		\
								  "STM     R0,{R4-R7}		\n"		\
								  "MOV     R4,R8            \n"		\
								  "MOV     R5,R9            \n"		\
								  "MOV     R6,R10           \n"		\
								  "MOV     R7,R11           \n"		\
								  "SUB     R0, R0, #0x20	\n"		\
								  "STM     R0,{R4-R7}		\n"		\
								  "SUB     R0, R0, #0x10	\n"		\
							)
////////////////////////////////////////////////////////////



/*****************************************************************************************//**
* \fn inline asm void RestoreContext(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/
/// Restore Context Define
#define OS_RESTORE_CONTEXT() __asm volatile(											  \
									/* Restore r4-11 from new process stack */			  \
									"LDM     R0!,{R4-R7}		\n"						  \
									"MOV     R8,R4              \n"						  \
									"MOV     R9,R5              \n"						  \
									"MOV     R10,R6             \n"					  	  \
									"MOV     R11,R7             \n"					  	  \
									"LDM     R0!,{R4-R7}		\n"						  \
									/* Restore the content of LR into R1  */			  \
									"POP	 {R1}				\n"						  \
									/* Load MSP with new process SP */					  \
									"MSR     PSP, R0			\n"						  \
									/* Ensures that int return use the task stack  */	  \
									"MOVS 	 R0,#0x0D			\n"						  \
									"ORR	 R1,R0				\n"						  \
								    /* Exception return will restore remaining context */ \
								    "CPSIE   I					\n"						  \
								    "BX      R1               	\n"						  \
								 )

////////////////////////////////////////////////////////////



/*****************************************************************************************//**
* \fn inline asm void SaveSP(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/
/// Save Stack Pointer Define
#define OS_SAVE_SP() __asm(	"LDR	R1,	=SPvalue	\n"		\
							"STR    R0, [R1]		\n"		\
						  )

////////////////////////////////////////////////////////////

/*****************************************************************************************//**
* \fn inline asm void RestoreContext(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/
/// Restore Stack Pointer Define
#define OS_RESTORE_SP()	__asm volatile (	"LDR	 R1, =SPvalue	\n"		\
											"LDR     R0, [R1]		\n"		\
							 )
////////////////////////////////////////////////////////////



/*****************************************************************************************//**
* \fn inline asm void RestoreSR(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/
/// No ARM o nesting esta habilitado por padrao
/// Restore Status Register Define
#define OS_ENABLE_NESTING()
////////////////////////////////////////////////////////////


extern inline void CriticalDecNesting(void);


#define BTOSStartFirstTask() __asm( /* Call SVC to start the first task. */		\
									"cpsie i				\n"					\
									"svc 0					\n"					\
								  )


/// Save Context Define
#define OS_SAVE_ISR() 	__asm(	" CPSID I			\n"		\
								" PUSH 	{LR}		\n")

#define OS_RESTORE_ISR() __asm(														  \
								/* Restore the content of LR into R1  */			  \
								"POP	 {R1}				\n"						  \
								/* Ensures that int return use the task stack  */	  \
								"MOVS 	 R0,#0x0D			\n"						  \
								"ORR	 R1,R0				\n"						  \
								/* Exception return will restore remaining context */ \
								"CPSIE   I					\n"						  \
								"BX      R1               	\n"						  \
								)


#define OS_EXIT_INT()                                                   \
    SelectedTask = OSSchedule();                                        \
    if (currentTask != SelectedTask){                                   \
        OS_SAVE_CONTEXT();                                              \
        OS_SAVE_SP();                                                   \
        ContextTask[currentTask].StackPoint = SPvalue;                  \
	      currentTask = SelectedTask;                                   \
        SPvalue = ContextTask[currentTask].StackPoint;                  \
        OS_RESTORE_SP();                                                \
        OS_RESTORE_CONTEXT();                                           \
    }


#endif
