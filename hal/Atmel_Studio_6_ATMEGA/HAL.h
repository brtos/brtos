#ifndef OS_HAL_H
#define OS_HAL_H

#include "OS_types.h"
#include "hardware.h"

/// Supported processors
#define COLDFIRE_V1		1
#define HCS08			2
#define MSP430			3
#define ATMEGA			4
#define PIC18			5

/// Define the used processor
#define PROCESSOR 		ATMEGA

/// Define the CPU type
#define OS_CPU_TYPE 	INT8U

/// Define if nesting interrupt is active
#define NESTING_INT 1

/// Define if its necessary to save status register / interrupt info
#define OS_SR_SAVE_VAR

/// Define stack growth direction
#define STACK_GROWTH 0            /// 1 -> down; 0-> up

/// Define the RAM buffer used to copy strings
/// Just to ATMEGA and PIC18 microcontrollers
#define TEXT_BUFFER_SIZE	32

// Define CPU Stack Pointer Size
#define SP_SIZE 16

extern INT16U SPvalue;

// *******************************************************
// * Port Defines                                        *
// *******************************************************
#define ChangeContext() SwitchContext()
#define OSEnterCritical() asm volatile("cli");
#define OSExitCritical() asm volatile("sei");

#define OS_Wait		SMCR |= 1;					\
					asm volatile("sleep");		\
					SMCR &= ~1;

#define OS_ENABLE_NESTING() OSExitCritical()

#define UserEnterCritical()	OSEnterCritical()
#define UserExitCritical()  OSExitCritical()

/// Defines the tick timer interrupt handler code (clear flag) of the choosen microcontroller
#define TICKTIMER_INT_HANDLER
#define TIMER_COUNTER   (INT32U)TCNT0
#define TIMER_MODULE    249


//Stack Defines

/* stacked by the RTI interrupt process */
#define NUMBER_MIN_OF_STACKED_BYTES 36

void CreateVirtualStack(void(*FctPtr)(void), INT16U NUMBER_OF_STACKED_BYTES);
void TickTimerSetup(void);                      
void SwitchContext(void);

#define OS_SAVE_SP() SPvalue = SP


#define OS_RESTORE_SP() SP = SPvalue



#define OS_SAVE_CONTEXT()		asm("PUSH R2 		\n\t" \
									"PUSH R3 		\n\t" \
									"PUSH R4 		\n\t" \
									"PUSH R5 		\n\t" \
									"PUSH R6 		\n\t" \
									"PUSH R7 		\n\t" \
									"PUSH R8 		\n\t" \
									"PUSH R9 		\n\t" \
									"PUSH R10 		\n\t" \
									"PUSH R11 		\n\t" \
									"PUSH R12 		\n\t" \
									"PUSH R13 		\n\t" \
									"PUSH R14 		\n\t" \
									"PUSH R15 		\n\t" \
									"PUSH R16 		\n\t" \
									"PUSH R17 		\n\t")


#define OS_RESTORE_CONTEXT()	asm("POP  R17 		\n\t" \
									"POP  R16 		\n\t" \
									"POP  R15 		\n\t" \
									"POP  R14 		\n\t" \
									"POP  R13 		\n\t" \
									"POP  R12 		\n\t" \
									"POP  R11 		\n\t" \
									"POP  R10 		\n\t" \
									"POP  R9 		\n\t" \
									"POP  R8 		\n\t" \
									"POP  R7 		\n\t" \
									"POP  R6 		\n\t" \
									"POP  R5 		\n\t" \
									"POP  R4 		\n\t" \
									"POP  R3 		\n\t" \
									"POP  R2 		\n\t")



#define OS_SAVE_ISR()			asm("PUSH R1 		\n\t" \
									"PUSH R0 		\n\t" \
									"IN   R0,0x3F 	\n\t" \
									"PUSH R0 		\n\t" \
									"CLR  R1 		\n\t" \
									"PUSH R18 		\n\t" \
									"PUSH R19 		\n\t" \
									"PUSH R20 		\n\t" \
									"PUSH R21 		\n\t" \
									"PUSH R22 		\n\t" \
									"PUSH R23 		\n\t" \
									"PUSH R24 		\n\t" \
									"PUSH R25 		\n\t" \
									"PUSH R26 		\n\t" \
									"PUSH R27 		\n\t" \
									"PUSH R28 		\n\t" \
									"PUSH R29 		\n\t" \
									"PUSH R30 		\n\t" \
									"PUSH R31 		\n\t")


#define OS_RESTORE_ISR()		asm("POP  R31 		\n\t" \
									"POP  R30 		\n\t" \
									"POP  R29 		\n\t" \
									"POP  R28 		\n\t" \
									"POP  R27 		\n\t" \
									"POP  R26 		\n\t" \
									"POP  R25 		\n\t" \
									"POP  R24 		\n\t" \
									"POP  R23 		\n\t" \
									"POP  R22 		\n\t" \
									"POP  R21 		\n\t" \
									"POP  R20 		\n\t" \
									"POP  R19 		\n\t" \
									"POP  R18 		\n\t" \
									"POP  R0 		\n\t" \
									"OUT  0x3F,R0 	\n\t" \
									"POP  R0 		\n\t" \
									"POP  R1 		\n\t" \
									"RETI           \n\t")


#define CriticalDecNesting()        \
{                                   \
  OSEnterCritical();                \
  iNesting--;                       \
}


#define BTOSStartFirstTask()      OS_RESTORE_SP();       \
                                  OS_RESTORE_CONTEXT();  \
                                  OS_RESTORE_ISR()

    

#endif
