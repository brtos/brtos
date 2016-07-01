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

/// Supported processors
#define COLDFIRE_V1     1u
#define HCS08           2u
#define MSP430          3u
#define ATMEGA          4u
#define PIC18           5u
#define RX600           6u
#define ARM_Cortex_M3   7u
#define ARM_Cortex_M4   8u
#define ARM_Cortex_M0   9u
#define ARM_Cortex_M4F  10u


/// Define the used processor
#define PROCESSOR 		ARM_Cortex_M0

/// Define the CPU type
#define OS_CPU_TYPE 	INT32U

/// Define MCU FPU hardware support
#define FPU_SUPPORT			0

/// Define if the optimized scheduler will be used
#define OPTIMIZED_SCHEDULER 0

/// Define if InstallTask function will support parameters
#define TASK_WITH_PARAMETERS 1

/// Define if 32 bits register for tick timer will be used
#define TICK_TIMER_32BITS   1

/// Define if nesting interrupt is active
#define NESTING_INT 1

/// Define the Reset Watchdog macro
#define RESET_WATCHDOG()	WWDG_SetCounter(92)

/// Define if its necessary to save status register / interrupt info
#define OS_SR_SAVE_VAR INT32U CPU_SR = 0;

/// Define stack growth direction
#define STACK_GROWTH 0            /// 1 -> down; 0-> up

/// Define CPU Stack Pointer Size
#define SP_SIZE 32

#define READY_LIST_VAR read_list

extern INT8U iNesting;
extern INT32U SPvalue;



/* Constants required to set up the initial stack. */
#define INITIAL_XPSR					0x01000000

/* Cortex-M specific definitions. */
#define PRIO_BITS       		        4        					// 15 priority levels
#define LOWEST_INTERRUPT_PRIORITY		0xF
#define KERNEL_INTERRUPT_PRIORITY 		(LOWEST_INTERRUPT_PRIORITY << (8 - PRIO_BITS) )

/* Constants required to manipulate the NVIC PendSV */
#define NVIC_PENDSVSET      			0x10000000         			// Value to trigger PendSV exception.
#define NVIC_PENDSVCLR      			0x08000000         			// Value to clear PendSV exception.

// Constants required to manipulate the NVIC SysTick
#define NVIC_SYSTICK_CLK        		0x00000004
#define NVIC_SYSTICK_INT        		0x00000002
#define NVIC_SYSTICK_ENABLE     		0x00000001

// ARM Cortex-Mx registers
#define NVIC_SYSTICK_CTRL       		( ( volatile unsigned long *) 0xe000e010 )
#define NVIC_SYSTICK_LOAD       		( ( volatile unsigned long *) 0xe000e014 )
#define NVIC_INT_CTRL_B           		( ( volatile unsigned long *) 0xe000ed04 )
#define FPU_FPCCR						( ( volatile unsigned long *) 0xE000EF34 )
#define NVIC_SYSPRI3					( ( volatile unsigned long *) 0xe000ed20 )

// Kernel interrupt priorities
#define NVIC_PENDSV_PRI					( ( ( unsigned long ) KERNEL_INTERRUPT_PRIORITY ) << 16 )
#define NVIC_SYSTICK_PRI				( ( ( unsigned long ) KERNEL_INTERRUPT_PRIORITY ) << 24 )

#define SMC_PMPROT_ALLS_DEF         0x08



unsigned short int _psp_swap2byte(unsigned short int n);
unsigned long int _psp_swap4byte(unsigned long int n);

#define _PSP_SWAP2BYTE(n)   _psp_swap2byte(n)
#define _PSP_SWAP4BYTE(n)   _psp_swap4byte(n)



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Port Defines                                /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


INT32U OS_CPU_SR_Save(void);
#define  OSEnterCritical() (CPU_SR = OS_CPU_SR_Save())	 // Disable interrupts
void OS_CPU_SR_Restore(INT32U);
#define  OSExitCritical()  (OS_CPU_SR_Restore(CPU_SR))	 // Enable interrupts

/// Defines the disable interrupts command of the choosen microcontroller
#define UserEnterCritical() __asm(" CPSID I")
/// Defines the enable interrupts command of the choosen microcontroller
#define UserExitCritical()  __asm(" CPSIE I")

/// Defines the low power command of the choosen microcontroller
#define OS_Wait __asm(" WFI ");

/// Defines the tick timer interrupt handler code (clear flag) of the choosen microcontroller
#define TICKTIMER_INT_HANDLER

#define TIMER_MODULE  SYST_RVR
#define TIMER_COUNTER SYST_CVR


// stacked by the RTI interrupt process
#if (FPU_SUPPORT == 1)
#define NUMBER_MIN_OF_STACKED_BYTES 68
#else
#define NUMBER_MIN_OF_STACKED_BYTES 64
#endif





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Functions Prototypes                        /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#define ChangeContext()		*(NVIC_INT_CTRL_B) = NVIC_PENDSVSET;	\
							UserExitCritical()

#define Clear_PendSV(void)	*(NVIC_INT_CTRL_B) = NVIC_PENDSVCLR

#define OS_INT_EXIT_EXT()	*(NVIC_INT_CTRL_B) = NVIC_PENDSVSET


#if (TASK_WITH_PARAMETERS == 1)
  void CreateVirtualStack(void(*FctPtr)(void*), INT16U NUMBER_OF_STACKED_BYTES, void *parameters);
#else
  void CreateVirtualStack(void(*FctPtr)(void), INT16U NUMBER_OF_STACKED_BYTES);
#endif

#if (TASK_WITH_PARAMETERS == 1)
  unsigned int CreateDVirtualStack(void(*FctPtr)(void*), unsigned int stk, void *parameters);
#else
  unsigned int CreateDVirtualStack(void(*FctPtr)(void), unsigned int stk);
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
#if (FPU_SUPPORT == 1)
#define OS_SAVE_CONTEXT() __asm(										\
		  	  	  	  	  	  	  "POP      {LR}                \n"		\
								  "MRS      R0, PSP				\n"		\
								  "TST 		R14,#0x10			\n"		\
								  "IT		EQ					\n"		\
								  "VSTMDBEQ R0!,{S16-S31}		\n"		\
								  "STMDB 	R0!, {r4-r11, R14}	\n"		\
							)
#else
#if (PROCESSOR != ARM_Cortex_M0)
#define OS_SAVE_CONTEXT() __asm(										\
								  "MRS      R0, PSP				\n"		\
								  "SUBS     R0, R0, #0x20		\n"		\
								  "STM      R0, {R4-R11}		\n"		\
							)
#else
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
#endif
#endif

////////////////////////////////////////////////////////////
/*

*/

/*****************************************************************************************//**
* \fn inline asm void RestoreContext(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/
/// Restore Context Define
#if (FPU_SUPPORT == 1)
#define OS_RESTORE_CONTEXT() __asm(														  \
									/* Restore r4-11 from new process stack */			  \
									"LDMIA    R0!, {R4-R11, R14}	\n"					  \
									"TST	  R14, #0x10			\n"					  \
									"IT 	  EQ					\n"					  \
									"VLDMIAEQ R0!, {S16-S31}		\n"					  \
									/* Load PSP with new process SP */					  \
									"MSR      PSP, R0				\n"					  \
									"ORR      LR,LR,#0x04     		\n"					  \
								    /* Exception return will restore remaining context */ \
	    							"CPSIE    I						\n"					  \
								    "BX       LR               		\n"					  \
								 )
#else
#if (PROCESSOR != ARM_Cortex_M0)
#define OS_RESTORE_CONTEXT() __asm(														  \
									/* Restore r4-11 from new process stack */			  \
									"LDM     R0, {R4-R11}		\n"					  	  \
									"ADDS    R0, R0, #0x20		\n"						  \
									/* Load PSP with new process SP */					  \
									"MSR     PSP, R0			\n"						  \
									"LDR     LR,=0xFFFFFFFD     \n"						  \
								    /* Exception return will restore remaining context */ \
	    							"CPSIE   I					\n"						  \
								    "BX     LR               	\n"						  \
								 )
#else
#define OS_RESTORE_CONTEXT() __asm volatile(											  \
									/* Restore r4-11 from new process stack */			  \
									"LDM     R0!,{R4-R7}		\n"						  \
									"MOV     R8,R4              \n"						  \
									"MOV     R9,R5              \n"						  \
									"MOV     R10,R6             \n"					  	  \
									"MOV     R11,R7             \n"					  	  \
									"LDM     R0!,{R4-R7}		\n"						  \
									/* Load MSP with new process SP */					  \
									"MSR     PSP, R0			\n"						  \
									/* Ensures that int return use the task stack  */	  \
									"LDR     R1,=0xFFFFFFFD     \n"						  \
								    /* Exception return will restore remaining context */ \
								    "CPSIE   I					\n"						  \
								    "BX      R1               	\n"						  \
								 )
#endif
#endif
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
#define OS_RESTORE_SP()__asm(	"LDR	 R1, =SPvalue	\n"		\
								"LDR     R0, [R1]		\n"		\
							 )

////////////////////////////////////////////////////////////



/*****************************************************************************************//**
* \fn inline asm void RestoreSR(void)
* \brief Restore context function
* \return NONE
*********************************************************************************************/
/// No ARM o nesting estar� habilitado por padr�o
/// Restore Status Register Define
#define OS_ENABLE_NESTING()
////////////////////////////////////////////////////////////

#define CriticalDecNesting()

#define BTOSStartFirstTask() __asm( /* Call SVC to start the first task. */		\
									"cpsie i				\n"					\
									"svc 0					\n"					\
								  )


/// Save Context Define
#if (FPU_SUPPORT == 1)
#define OS_SAVE_ISR()	__asm(" CPSID I");	\
						__asm("PUSH {LR}")
#else
#define OS_SAVE_ISR()
#endif


#if (FPU_SUPPORT == 1)
#define OS_RESTORE_ISR()  __asm(							  \
		"POP     {LR}               \n"					      \
		"ORR     LR,LR,#0x04     	\n"						  \
		/* Exception return will restore remaining context */ \
		"CPSIE   I					\n"						  \
		"BX      LR               	\n"						  \
		)
#else
#if (PROCESSOR != ARM_Cortex_M0)
#define OS_RESTORE_ISR()  __asm(							  \
		"LDR     LR,=0xFFFFFFFD     \n"						  \
		/* Exception return will restore remaining context */ \
		"CPSIE   I					\n"						  \
		"BX      LR               	\n"						  \
		)
#else
#define OS_RESTORE_ISR()  __asm(							  \
		"LDR     R1,=0xFFFFFFFD     \n"						  \
		/* Exception return will restore remaining context */ \
		"CPSIE   I					\n"						  \
		"BX      R1               	\n"						  \
		)
#endif
#endif

#if (OPTIMIZED_SCHEDULER == 1)
#define Optimezed_Scheduler()				\
INT8U priority;								\
__asm volatile   ("CLZ %1,%1      \n\t"		\
				  "RSB %0,%1,0x1F \n\t"		\
				  : "=r" (priority)			\
				  : "r" (READY_LIST_VAR));	\
return priority

#endif
#endif
