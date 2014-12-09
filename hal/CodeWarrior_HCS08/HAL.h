#ifndef OS_HAL_H
#define OS_HAL_H

#include "OS_types.h"
#include "hardware.h"

/// Supported processors
#define COLDFIRE_V1		1
#define HCS08			    2
#define MSP430			  3
#define ATMEGA			  4
#define PIC18			    5


/// Define the used processor
#define PROCESSOR 		HCS08

/// Define the CPU type
#define OS_CPU_TYPE 	INT8U

/// There is no optimized scheduler for HCS08 MCUs
#define OPTIMIZED_SCHEDULER 0

/// Define if nesting interrupt is active
#define NESTING_INT 0

/// Define if its necessary to save status register / interrupt info
#define OS_SR_SAVE_VAR

/// Define stack growth direction
#define STACK_GROWTH 0            /// 1 -> down; 0-> up

// Define CPU Stack Pointer Size
#define SP_SIZE 16

extern INT16U SPvalue;


// *******************************************************
// * Port Defines                                        *
// *******************************************************
#define ChangeContext() asm ("SWI");
#define OSEnterCritical() asm ("SEI");
#define OSExitCritical() asm ("CLI");
#define OS_Wait _Wait;
#define OS_ENABLE_NESTING() OSExitCritical()
#define UserEnterCritical   OSEnterCritical
#define UserExitCritical   OSExitCritical

/// Defines the tick timer interrupt handler code (clear flag) of the choosen microcontroller
#define TICKTIMER_INT_HANDLER TPM1SC_TOF = 0
#define TIMER_MODULE 
#define TIMER_COUNTER 1


//Stack Defines

/* stacked by the RTI interrupt process */
// Mínimo de 60 devido ao salvamento de 15 registradores de 32 bits da CPU
// D0-D7 (32 bytes) + A0-A6 (28 bytes) + Format (1 byte) + Vector (1 byte) + SR (2 bytes) + SP (4 bytes)
#define NUMBER_MIN_OF_STACKED_BYTES 6

/* User defined: stacked for user function calls + local variables */
// Ainda, como podem ocorrer interrupções durante as tarefas, alocar 28 bytes a cada
// interrupção ativa
// 4 bytes to Local Variable Allocation
// 4 bytes to Function Call



void CreateVirtualStack(void(*FctPtr)(void), INT16U NUMBER_OF_STACKED_BYTES);
void TickTimerSetup(void);                      



#define OS_SAVE_SP()    \
{                       \
  asm("TSX");           \
  asm("STHX SPvalue");  \
}


#define OS_RESTORE_SP()      \
{                            \
  asm("LDHX SPvalue");       \
  asm("TXS");                \
}



#define OS_SAVE_CONTEXT()
#define OS_RESTORE_CONTEXT()


#define OS_RESTORE_ISR()     \
{                            \
  asm("PULH");               \
  asm("RTI");                \
}


#define CriticalDecNesting()        \
{                                   \
  OSEnterCritical();                \
  iNesting--;                       \
}



#define BTOSStartFirstTask()      OS_RESTORE_SP();       \
                                  OS_RESTORE_CONTEXT();  \
                                  OS_RESTORE_ISR()
    

#endif