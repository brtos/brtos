#include "hardware.h"
#include "BRTOS.h"

INT16U SPvalue = 0;


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Tick Timer Setup                            /////
/////                                                  /////
/////                                                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
void TickTimerSetup(void)
{
  // Initialize ATMega368 Timer/Counter0 Peripheral
  INT16U compare = (configCPU_CLOCK_HZ / configTICK_RATE_HZ) / configTIMER_PRE_SCALER_VALUE;  
  OCR0A = (INT8U)(compare - 1);     // Adjust for correct value
  TCCR0A=0x02;                      // Clear Timer on Compare mode
  TCCR0B = configTIMER_PRE_SCALER;	// Set PreScaler
  TCNT0=0x00;                       // Reset counter, overflow at 1 mSec
  TIMSK0=(1<<OCIE0A);               // Enable Output Compare Interrupt
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
ISR(TIMER0_COMPA_vect, __attribute__ ( ( naked ) ))
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
    // Pointer to Task Entry
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 35] = 17;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 34] = 16;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 33] = 15;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 32] = 14;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 31] = 13;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 30] = 12;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 29] = 11;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 28] = 10;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 27] = 9;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 26] = 8;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 25] = 7;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 24] = 6;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 23] = 5;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 22] = 4;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 21] = 3;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 20] = 2;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 19] = 31;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 18] = 30;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 17] = 29;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 16] = 28;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 15] = 27;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 14] = 26;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 13] = 25;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 12] = 24;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 11] = 23;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 10] = 22;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 9] = 21;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 8] = 20;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 7] = 19;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 6] = 18;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 4] = 0x00;
   
   /* The compiler expects R1 to be 0. */
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 3] = 0x00;
    
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)	
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 0] = ((unsigned int) (FctPtr)) & 0x00FF;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 1] = ((unsigned int) (FctPtr)) >> 8;  
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 2] = ((unsigned long int)(FctPtr)) >> 16;
#else    
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 1] = ((unsigned int) (FctPtr)) & 0x00FF;
   STACK[iStackAddress + NUMBER_OF_STACKED_BYTES - 2] = ((unsigned int) (FctPtr)) >> 8;  
#endif   
   
   
}






