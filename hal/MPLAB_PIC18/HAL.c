#include "hardware.h"
#include "BRTOS.h"
#include "serial.h"
#include "usart.h"

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

		CCPR1 = ( configCPU_INT_CLOCK_HZ / configTICK_RATE_HZ );
	
		TMR1H = ( unsigned char ) 0x00;
		TMR1L = ( unsigned char ) 0x00;	
	
		//RCONbits.IPEN = 1;            //enable priority levels
		CCP1CONbits.CCP1M0 = 1;		/*< Compare match mode. */
		CCP1CONbits.CCP1M1 = 1;		/*< Compare match mode. */
		CCP1CONbits.CCP1M2 = 0;		/*< Compare match mode. */
		CCP1CONbits.CCP1M3 = 1;		/*< Compare match mode. */
		PIE1bits.CCP1IE = 1;		/*< Interrupt enable. */
		INTCONbits.PEIE = 1;    	/*< Enable interrupts */
		//IPR1bits.CCP1IP = 0;		/// low priority
		//INTCONbits.GIEL = 1;    	/*< Enable interrupts */

		OpenTimer1( T1_16BIT_RW & T1_SOURCE_INT & T1_PS_1_1 & T1_CCP1_T3_CCP2 );

}  
/*
 INTCON |= 0x20;                //enable TMR0 interrupt
  INTCON2 |= 0x84;               //TMR0 low priority
  RCONbits.IPEN = 1;            //enable priority levels
  TMR0H = 0;                    //clear timer
  TMR0L = 0;                    //clear timer
  T0CON = 0x88;                 //set up timer0 - no prescaler 
*/

/*
  INTCON = 0x20;                //disable global and enable TMR0 interrupt
  INTCON2 = 0x84;               //TMR0 high priority
  RCONbits.IPEN = 1;            //enable priority levels
  TMR0H = 0;                    //clear timer
  TMR0L = 0;                    //clear timer
  T0CON = 0x82;                 //set up timer0 - prescaler 1:8
  INTCONbits.GIEH = 1;          //enable interrupts   
*/

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
    INT8U *stk = &STACK[iStackAddress];
	INT16U address = (INT16U)&STACK[iStackAddress];

	address = address >> 8;
    *stk++  = 0xF1;                     // <-- FSR1 Pointer position on function call.
    *stk++  = 0xA5;                     // STATUS register
    *stk++  = 0x00;                     // W register 
    *stk++  = 0xB0;                     // BSR register
    *stk++  = (INT8U)address;           // FSR2H register
    *stk++  = 0xF2;                     // FSR2L register
    *stk++  = 0xF0;                     // FSR0L register
    *stk++  = 0xF0;                     // FSR0H register
	  *stk++  = 0xB2; 					          // TABLAT
    *stk++  = 0xB3;                     // TBLPTRL Prog Mem Table Pointer Low
    *stk++  = 0xB4;                     // TBLPTRH Prog Mem Table Pointer High
    *stk++  = 0xB5;                     // TBLPTRU Prog Mem Table Pointer Upper
    *stk++  = 0xD0;                     // PRODL Product Reg low
    *stk++  = 0xD1;                     // PRODH Product Reg high
    *stk++  = 0xA3;						// AARGB3
    *stk++  = 0xA2;						// AARGB2
    *stk++  = 0xA1;						// AARGB1
    *stk++  = 0xA0;						// AARGB0
    *stk++  = 0xB3;						// BARGB3
    *stk++  = 0xB2;						// BARGB2 
    *stk++  = 0xB1;						// BARGB1
    *stk++  = 0xB0;						// BARGB0
    *stk++  = 0xE3;						// REMB3
    *stk++  = 0xE2;						// REMB2 
    *stk++  = 0xE1;						// REMB1
    *stk++  = 0xE0;						// REMB0 
    *stk++  = 0xFB;						// __FPFLAGS
    *stk++  = 0xFF;						// SIGN
    *stk++  = 0xAE;						// __AEXP
    *stk++  = 0xBE;						// __BEXP
    *stk++  = 0xD3;						// __TEMPB3
    *stk++  = 0xD2;						// __TEMPB2
    *stk++  = 0xD1;						// __TEMPB1
    *stk++  = 0xD0;						// __TEMPB0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
    *stk++  = 0x10;						// __tmp_0
	  *stk++  = 0xC1;						// PCLATH
	  *stk++  = 0xC2;						// PCLATU

    // first return address, the task address, goes on the hardware return stack in a context switch
    *stk++  = (INT8U)((INT16U)(FctPtr)) & 0x00FF; 	// TOSL
    *stk++  = (INT8U)((INT16U)(FctPtr) >> 8); 		// TOSH
    *stk++  = (INT8U)0;								// TOSU

    *stk++  = 0x01;                     // size of return stack - one level deep only
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
*********************************************************************************************************
*                                             TICK ISR
*********************************************************************************************************
*/

//----------------------------------------------------------------------------
// Clock ISR High priority interrupt routine

// High priority interrupt vector
void TickTimerHandler(void);
void SerialRx(void);
void SerialTx(void);

// High priority interrupt routine
#pragma code highVector=0x008
void HighInterrupt( void )
{
	/* Was the interrupt a timer1 compare? */
	if( PIR1bits.CCP1IF )
	{		
		_asm
			goto TickTimerHandler
		_endasm
	}
	
	/* Was the interrupt a byte being received? */
	if( PIR1bits.RCIF )
	{
		_asm
			goto SerialRx
		_endasm
	}

	/* Was the interrupt a serial transmission completed? */
	if( PIR1bits.TXIF )
	{
		_asm
			goto SerialTx
		_endasm
	}
}
#pragma code
//----------------------------------------------------------------------------


void TickTimerHandler(void)
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


