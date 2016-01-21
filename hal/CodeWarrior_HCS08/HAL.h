#ifndef OS_HAL_H
#define OS_HAL_H

#include "OS_types.h"
#include "IO_Map.h"
#include "BRTOSConfig.h"

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
#define PROCESSOR 		HCS08

/// Define the CPU type
#define OS_CPU_TYPE 	INT8U

/// Define if InstallTask function will support parameters
#define TASK_WITH_PARAMETERS 1

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
#define OS_Wait 		asm("WAIT");
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


#if (!BRTOS_DYNAMIC_TASKS_ENABLED)
#if (TASK_WITH_PARAMETERS == 1)
  void CreateVirtualStack(void(*FctPtr)(void*), INT16U NUMBER_OF_STACKED_BYTES, void *parameters);
#else
  void CreateVirtualStack(void(*FctPtr)(void), INT16U NUMBER_OF_STACKED_BYTES);
#endif
#endif
  
#if (BRTOS_DYNAMIC_TASKS_ENABLED == 1)
#if (TASK_WITH_PARAMETERS == 1)
  unsigned int CreateDVirtualStack(void(*FctPtr)(void*), unsigned int stk, void *parameters);
#else
  unsigned int CreateDVirtualStack(void(*FctPtr)(void), unsigned int stk);
#endif
#endif
  
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
