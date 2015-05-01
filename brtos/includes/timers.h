/**
* \file timers.h
* \brief OS Soft Timers service functions
*
* Functions to create, start, stop, delete and 
* get remaining time of soft timers
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
*                                     OS Soft Timers functions
*
*
*   Authors:  Carlos Henrique Barriquelo
*   Revision: 1.0
*   Date:     12/01/2013
*********************************************************************************************************/


/*****************************************************************/
/*                          OS SOFT TIMER                        */
/*****************************************************************/
#ifndef TIMERS_H
#define TIMERS_H

#include "OS_types.h"
#include "BRTOSConfig.h"
#include "BRTOS.h"

#ifdef BRTOS_TMR_EN 
#if (BRTOS_TMR_EN == 1)

/// Defines the maximum number of timers by default
/// Limits the memory allocation for timers
#define BRTOS_MAX_TIMER_DEFAULT        8
#ifndef BRTOS_MAX_TIMER  
  #define BRTOS_MAX_TIMER BRTOS_MAX_TIMER_DEFAULT
#endif

/* config defines */ 
// do not change, unless we know what are you doing
#define TIMER_CNT             INT16U                  
#define TIMER_MAX_COUNTER     (TIMER_CNT)(TICK_COUNT_OVERFLOW-1)   

/* typedefs for callback struct */  
typedef TIMER_CNT (*FCN_CALLBACK) (void);  

/* soft timer possible states:
*/
typedef enum
{
  TIMER_NOT_ALLOCATED = 0,
  TIMER_NOT_USED = 1,
  TIMER_STOPPED = 2,
  TIMER_RUNNING = 3,
  TIMER_SEARCH = 4,
} TIMER_STATE;

/* soft timer data struct
*/
typedef struct BRTOS_TIMER_S 
{
      FCN_CALLBACK       func_cb;
      TIMER_CNT          timeout;
      TIMER_STATE        state;
} BRTOS_TIMER_T;

/* soft timer typedef
*/
typedef  BRTOS_TIMER_T*  BRTOS_TIMER; 

/* soft timers list data struct 
*/

typedef struct
{    
    BRTOS_TIMER timers [BRTOS_MAX_TIMER];
    INT8S       count;
}BRTOS_TMR_T;


/* TIMER TASK prototype */  
void BRTOS_TimerTask(void);
 
/************* public API *********************/ 
void OSTimerInit(INT16U timertask_stacksize, INT8U prio);
INT8U OSTimerSet (BRTOS_TIMER *cbp, FCN_CALLBACK cb, TIMER_CNT timeout);
TIMER_CNT OSTimerGet (BRTOS_TIMER p);
INT8U OSTimerStart (BRTOS_TIMER p, TIMER_CNT timeout);  
INT8U OSTimerStop (BRTOS_TIMER p, INT8U del); 

/***************************************/


#endif
#endif
/*****************************************************************/
/*                          OS SOFT TIMER EOF                    */
/*****************************************************************/

#endif
