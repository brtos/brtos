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
#define PROCESSOR 		PIC18

#define OS_CPU_TYPE		INT8U

/// Define if nesting interrupt is active
#define NESTING_INT 0

/// Define if its necessary to save status register / interrupt info
#define OS_SR_SAVE_VAR

// Define CPU Stack Pointer Size
#define SP_SIZE 16

/// Define stack growth direction
#define STACK_GROWTH 1  /// 1 -> down; 0-> up

extern INT16U SPvalue;

#define TEXT_BUFFER_SIZE	32

char *strcpypgm2ram (auto char *s1, auto const rom char far *s2);

// *******************************************************
// * Port Defines                                        *
// *******************************************************
#define ChangeContext() SwitchContext()
#define UserEnterCritical() INTCONbits.GIEH = 0;
#define UserExitCritical()  INTCONbits.GIEH = 1;

#define OSEnterCritical()	UserEnterCritical()
#define OSExitCritical()	UserExitCritical()

	
#define OS_Wait		_asm SLEEP _endasm   

#define OS_ENABLE_NESTING() OSExitCritical()

/// Defines the tick timer interrupt handler code (clear flag) of the choosen microcontroller
#define TICKTIMER_INT_HANDLER	PIR1bits.CCP1IF = 0;
#define TIMER_MODULE			CCPR1
#define TIMER_COUNTER 			(TMR1H << 8) + TMR1L


//Stack Defines

/* stacked by the RTI interrupt process */
// Mínimo de 24 devido ao salvamento dos registradores da CPU
#define NUMBER_MIN_OF_STACKED_BYTES 	56

/* User defined: stacked for user function calls + local variables */
// Ainda, como podem ocorrer interrupções durante as tarefas, alocar 28 bytes a cada
// interrupção ativa
// 4 bytes to Local Variable Allocation
// 4 bytes to Function Call



void CreateVirtualStack(void(*FctPtr)(void), INT16U NUMBER_OF_STACKED_BYTES);
void TickTimerSetup(void);                      
void SwitchContext(void);


/* Save the new top of the software stack. */
#define OS_SAVE_SP() 			_asm																		\
									MOVFF	FSR1L, SPvalue													\
									MOVFF	FSR1H, SPvalue+1												\
								_endasm		


#define OS_RESTORE_SP() 	_asm																		\
								MOVFF	SPvalue, FSR1L										     		\
								MOVFF	SPvalue+1, FSR1H												\
																										\
								/* How many return addresses are there on the hardware stack?  Discard	\
								the first byte as we are pointing to the next free space. */			\
								MOVFF	POSTDEC1, FSR0L													\
								MOVFF	POSTDEC1, FSR0L													\
							_endasm	


/* Store the hardware stack pointer in a temp register before we modify it.    */
												
#define OS_SAVE_CONTEXT()		_asm																		\
									MOVFF	STKPTR, FSR0L													\
								_endasm																		\
																											\
									/* Store each address from the hardware stack. */						\
									while( STKPTR > ( unsigned char ) 0 )							    	\
									{																		\
										_asm																\
											MOVFF	TOSL, PREINC1											\
											MOVFF	TOSH, PREINC1											\
											MOVFF	TOSU, PREINC1											\
											POP																\
										_endasm																\
									}																		\
																											\
								_asm																		\
/* Store the number of addresses on the hardware stack (from the temporary register). */     				\
									MOVFF	FSR0L, PREINC1													\
									MOVF	PREINC1, 1, 0													\
								_endasm																		\

	

/* Fill the hardware stack from our software stack. */
						
#define OS_RESTORE_CONTEXT()	STKPTR = 0;																	\
																											\
								while( STKPTR < FSR0L )														\
								{																			\
									_asm																	\
										PUSH																\
										MOVF	POSTDEC1, 0, 0												\
										MOVWF	TOSU, 0														\
										MOVF	POSTDEC1, 0, 0												\
										MOVWF	TOSH, 0														\
										MOVF	POSTDEC1, 0, 0												\
										MOVWF	TOSL, 0														\
									_endasm																	\
								}																			\


#define OS_SAVE_ISR()    _asm																		\
							MOVFF	STATUS, PREINC1													\
							MOVFF	WREG, PREINC1													\
							MOVFF	BSR, PREINC1  											    	\
							MOVFF   FSR2H, PREINC1													\
							MOVFF   FSR1H, FSR2H													\
							MOVFF   FSR2L, PREINC1													\
							MOVFF	FSR0L, PREINC1													\
							MOVFF	FSR0H, PREINC1													\
							MOVFF	TABLAT, PREINC1													\
							MOVFF	TBLPTRL, PREINC1												\
							MOVFF	TBLPTRH, PREINC1												\
							MOVFF	TBLPTRU, PREINC1												\
							MOVFF	PRODL, PREINC1													\
							MOVFF	PRODH, PREINC1													\
							MOVFF	__AARGB3, PREINC1												\
							MOVFF	__AARGB2, PREINC1												\
							MOVFF	__AARGB1, PREINC1												\
							MOVFF	__AARGB0, PREINC1  											    \
							MOVFF	__BARGB3, PREINC1												\
							MOVFF	__BARGB2, PREINC1												\
							MOVFF	__BARGB1, PREINC1												\
							MOVFF	__BARGB0, PREINC1  											    \
							MOVFF	__REMB3, PREINC1												\
							MOVFF	__REMB2, PREINC1												\
							MOVFF	__REMB1, PREINC1												\
							MOVFF	__REMB0, PREINC1  											    \
							MOVFF	__FPFLAGS, PREINC1 											    \
							MOVFF	__FPFLAGS-1, PREINC1 											\
							MOVFF	__AEXP, PREINC1 	 											\
							MOVFF	__BEXP, PREINC1 											    \
							MOVFF	__TEMPB3, PREINC1												\
							MOVFF	__TEMPB2, PREINC1												\
							MOVFF	__TEMPB1, PREINC1												\
							MOVFF	__TEMPB0, PREINC1  											    \
							MOVFF	__TEMPB0+1, PREINC1											    \
							MOVFF	__TEMPB0+2, PREINC1  										    \
							MOVFF	__TEMPB0+3, PREINC1  										    \
							MOVFF	__TEMPB0+4, PREINC1  										    \
							MOVFF	__TEMPB0+5, PREINC1  										    \
							MOVFF	__TEMPB0+6, PREINC1  										    \
							MOVFF	__TEMPB0+7, PREINC1  										    \
							MOVFF	__TEMPB0+8, PREINC1  										    \
							MOVFF	__TEMPB0+9, PREINC1  										    \
							MOVFF	__TEMPB0+10, PREINC1  										    \
							MOVFF	__TEMPB0+11, PREINC1  										    \
							MOVFF	__TEMPB0+12, PREINC1  										    \
							MOVFF	__TEMPB0+13, PREINC1  										    \
							MOVFF	__TEMPB0+14, PREINC1  										    \
							MOVFF	__TEMPB0+15, PREINC1  										    \
							MOVFF	__TEMPB0+16, PREINC1  										    \
							MOVFF	PCLATH, PREINC1													\
							MOVFF	PCLATU, PREINC1													\
						_endasm																		\


/* Restore the other registers forming the tasks context. */
#define OS_RESTORE_ISR() _asm																		\
							MOVFF	POSTDEC1, PCLATU												\
							MOVFF	POSTDEC1, PCLATH												\
							MOVFF	POSTDEC1, __TEMPB0+16											\
							MOVFF	POSTDEC1, __TEMPB0+15											\
							MOVFF	POSTDEC1, __TEMPB0+14											\
							MOVFF	POSTDEC1, __TEMPB0+13											\
							MOVFF	POSTDEC1, __TEMPB0+12											\
							MOVFF	POSTDEC1, __TEMPB0+11											\
							MOVFF	POSTDEC1, __TEMPB0+10											\
							MOVFF	POSTDEC1, __TEMPB0+9											\
							MOVFF	POSTDEC1, __TEMPB0+8											\
							MOVFF	POSTDEC1, __TEMPB0+7											\
							MOVFF	POSTDEC1, __TEMPB0+6											\
							MOVFF	POSTDEC1, __TEMPB0+5											\
							MOVFF	POSTDEC1, __TEMPB0+4											\
							MOVFF	POSTDEC1, __TEMPB0+3											\
							MOVFF	POSTDEC1, __TEMPB0+2											\
							MOVFF	POSTDEC1, __TEMPB0+1											\
							MOVFF	POSTDEC1, __TEMPB0												\
							MOVFF	POSTDEC1, __TEMPB1												\
							MOVFF	POSTDEC1, __TEMPB2												\
							MOVFF	POSTDEC1, __TEMPB3												\
							MOVFF	POSTDEC1, __BEXP												\
							MOVFF	POSTDEC1, __AEXP												\
							MOVFF	POSTDEC1, __FPFLAGS-1											\
							MOVFF	POSTDEC1, __FPFLAGS												\
							MOVFF	POSTDEC1, __REMB0												\
							MOVFF	POSTDEC1, __REMB1												\
							MOVFF	POSTDEC1, __REMB2												\
							MOVFF	POSTDEC1, __REMB3												\
							MOVFF	POSTDEC1, __BARGB0												\
							MOVFF	POSTDEC1, __BARGB1												\
							MOVFF	POSTDEC1, __BARGB2												\
							MOVFF	POSTDEC1, __BARGB3												\
							MOVFF	POSTDEC1, __AARGB0												\
							MOVFF	POSTDEC1, __AARGB1												\
							MOVFF	POSTDEC1, __AARGB2												\
							MOVFF	POSTDEC1, __AARGB3												\
							MOVFF	POSTDEC1, PRODH													\
							MOVFF	POSTDEC1, PRODL													\
							MOVFF	POSTDEC1, TBLPTRU												\
							MOVFF	POSTDEC1, TBLPTRH												\
							MOVFF	POSTDEC1, TBLPTRL												\
							MOVFF	POSTDEC1, TABLAT												\
							MOVFF	POSTDEC1, FSR0H													\
							MOVFF	POSTDEC1, FSR0L													\
							MOVFF	POSTDEC1, FSR2L													\
							MOVFF	POSTDEC1, FSR2H													\
							MOVFF   FSR2H, FSR1H													\
							MOVFF	POSTDEC1, BSR													\
							MOVFF	POSTDEC1, WREG													\
							MOVFF	POSTDEC1, STATUS												\
							RETFIE  0 																\
						_endasm																		\


#define CriticalDecNesting()        \
{                                   \
  /*UserEnterCritical();*/          \
  iNesting--;                       \
}									\


#define BTOSStartFirstTask()      OS_RESTORE_SP();       \
                                  OS_RESTORE_CONTEXT();  \
                                  OS_RESTORE_ISR()
                                  

#endif
