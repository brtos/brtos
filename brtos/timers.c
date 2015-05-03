/**
* \file timers.c
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
*   Authors:  Carlos Henrique Barriquelo, Gustavo Denardin
*   Revision: 1.1
*   Date:     01/05/2015
*********************************************************************************************************/


/*****************************************************************/
/*                          OS SOFT TIMER                        */
/*****************************************************************/
/**
\brief BRTOS soft timer service
*/
#include "timers.h"


#ifdef BRTOS_TMR_EN 
#if (BRTOS_TMR_EN == 1) 

/* private data */
static struct {
    BRTOS_TIMER_T   mem[BRTOS_MAX_TIMER]; /* array of callback structs */            
    BRTOS_TMR_T*    current;              /* keep current timer list */ 
    BRTOS_TMR_T*    future;               /* keep future timer list   */
    INT8U           handling_task;        /* caller Task ID */          
} BRTOS_TIMER_VECTOR;


/* soft timer lists: memory allocation */
static BRTOS_TMR_T BRTOS_TMR_PING,BRTOS_TMR_PONG;    /* Timer lists */

/* local functions */
/* Binary heap of timers */
#define PAI(i)    (INT8U)(i>>1)
#define LEFT(i)   (INT8U)(i<<1)
#define RIGHT(i)  (INT8U)((i<<1) + 1)

static void Subir (BRTOS_TIMER* timers,INT8U i) {
     while (i > 1 && timers[PAI(i)]->timeout > timers[i]->timeout)
     {
         void* tmp = timers[PAI(i)];
         timers[PAI(i)] = timers[i];
         timers[i] = tmp;
         i=PAI(i);
     }
}

static void Descer (BRTOS_TIMER* timers,INT8U i, INT8U n) 
{
  INT8U son;
  do{    
    if (RIGHT(i) <= n && timers[RIGHT(i)]->timeout < timers[LEFT(i)]->timeout)
    {
       son = RIGHT(i);
    }
    else 
	{
		son = LEFT(i);
	}
    if (son <= n  && timers[son]->timeout < timers[i]->timeout)
    {
       void* tmp = timers[son];
       timers[son] = timers[i];
       timers[i] = tmp;
       i=son;     
    }else break;
  }while(1);
}

/* private functions */
static void BRTOS_TimerTaskInit(void)
{
  
  OS_SR_SAVE_VAR 
  INT8U i; 
  
  if (currentTask)
    OSEnterCritical();
        
    BRTOS_TIMER_VECTOR.current = &BRTOS_TMR_PING;
    BRTOS_TIMER_VECTOR.future  = &BRTOS_TMR_PONG;
    
    for(i=0;i<BRTOS_MAX_TIMER;i++)
    {           
      BRTOS_TIMER_VECTOR.mem[i].state = TIMER_NOT_USED;
      BRTOS_TIMER_VECTOR.mem[i].func_cb = NULL;
      BRTOS_TIMER_VECTOR.mem[i].timeout = 0;  
    }  
    
  if (currentTask)
     OSExitCritical();

}

static void BRTOS_TimerTaskSleep(TIMER_CNT next_time_to_wake)
{
  
  OS_SR_SAVE_VAR
  
  ContextType *Task = (ContextType*)&ContextTask[currentTask];      
  
  Task->TimeToWait = next_time_to_wake;
  
  OSEnterCritical();
  
  // Put task into delay list
  IncludeTaskIntoDelayList();
  
  #if (VERBOSE == 1)
    Task->State = SUSPENDED;
    Task->SuspendedType = DELAY;
  #endif
  
  OSReadyList = OSReadyList & ~(PriorityMask[Task->Priority]);
  
  // Change context
  // Return to task when occur delay overflow
  ChangeContext();
  
  OSExitCritical();
    
}

/* Timer Task */
void BRTOS_TimerTask(void)
{
     
     OS_SR_SAVE_VAR
     BRTOS_TIMER p;
     TIMER_CNT   tickcount;
     TIMER_CNT   repeat;
     INT32U      timeout;
     TIMER_CNT   next_time_to_wake;      /* tick count of next timer */
     BRTOS_TMR_T *list, *list_tmp;          
     
     BRTOS_TIMER_VECTOR.handling_task = currentTask;
     
     list = BRTOS_TIMER_VECTOR.current;

     p=list->timers[1];
     if(p!= NULL)
     {      
       next_time_to_wake = p->timeout;
     }
     else
     {
       next_time_to_wake = TIMER_MAX_COUNTER;    
     }
  
     for(;;)
     {
     
        BRTOS_TimerTaskSleep(next_time_to_wake);

        tickcount = OSGetTickCount();               

timer_loop:         
        list = BRTOS_TIMER_VECTOR.current;
        p=list->timers[1];

        while(p!= NULL && p->timeout <= tickcount)
        {  
            // some timer has expired
            if((p)->func_cb != NULL) 
            {                            
              repeat = (TIMER_CNT)((p)->func_cb()); /* callback */
              
              OSEnterCritical(); 
                           
              if (repeat > 0)
              { /* needs to repeat after "repeat" time ? */
                  timeout = (INT32U)((INT32U)tickcount + (INT32U)repeat);                  
                  if (timeout >= TICK_COUNT_OVERFLOW)
                  {
                    p->timeout = (TIMER_CNT)(timeout - TICK_COUNT_OVERFLOW);                                 
                    list_tmp = BRTOS_TIMER_VECTOR.future; // add into future list
                    list_tmp->timers[++list_tmp->count] = p; // insert in the end
                    Subir(list_tmp->timers,list_tmp->count);                                      
                    list->timers[1]=list->timers[list->count]; // remove from current list
                    list->timers[list->count] = NULL;
                    list->count--;                    
                  }
                  else
                  {
                    p->timeout = (TIMER_CNT)timeout;                  
                  }                  
               } 
               else
               {
                  p->timeout = 0;  
                  p->state = TIMER_NOT_ALLOCATED; 
                  p->func_cb = NULL;                                  
                  list->timers[1]=list->timers[list->count]; // remove from current list
                  list->timers[list->count] = NULL;
                  list->count--;                
               }             
             }
             
             Descer (list->timers, 1, list->count); // order it                         
             p=list->timers[1];                           
             OSExitCritical();                           
        }
                
        if(tickcount == TIMER_MAX_COUNTER)
        {
          if(p==NULL)
          {            
            /* time to switch lists */
            void* tmp = BRTOS_TIMER_VECTOR.current;                               
            BRTOS_TIMER_VECTOR.current = BRTOS_TIMER_VECTOR.future;
            BRTOS_TIMER_VECTOR.future = tmp; 
            list = BRTOS_TIMER_VECTOR.current;
            p=list->timers[1];
          }
          else
          {
            /* there is a delayed timer */
            tickcount++;
            goto timer_loop;            
          }
        }
        
        /* if any timer waiting, set task to wake */
        if(p!= NULL)
        {
            next_time_to_wake = p->timeout;
        }
        else
        {              
            next_time_to_wake = TIMER_MAX_COUNTER;
        }
     }
  
}

/* Public functions */

/**
  \fn void BRTOS_TimerInit(INT16U timertask_stacksize) 
  \brief public function to start Timer Service 
  must be called before any call to the other public functions.
  It only installs "BRTOS_TimerTask".
  \param timertask_stacksize size of stack allocated to the task
  \return nothing if sucess or never if any error  
*/
void OSTimerInit(INT16U timertask_stacksize, INT8U prio){

  BRTOS_TimerTaskInit();
   
   
  if(InstallTask(&BRTOS_TimerTask,"BRTOS Timers Task",timertask_stacksize, prio) != OK)
  {
    while(1){};
  }  
  
}
/**
  \fn INT8U BRTOS_TimerSet (BRTOS_TIMER *cbp, FCN_CALLBACK cb, TIMER_CNT time_wait) 
  \brief public function to create and start a soft timer
   must be called before any call to the other public timer functions.
  \param *cbp  soft timer pointer
  \param cb    callback function
  \param time_wait soft timer expiration time
  \return success (OK) or error codes 
  \return OK success
  \return NULL_EVENT_POINTER
  \return NO_AVAILABLE_EVENT
  \return ERR_EVENT_NO_CREATED
*/

INT8U OSTimerSet (BRTOS_TIMER *cbp, FCN_CALLBACK cb, TIMER_CNT time_wait)
{
    
    OS_SR_SAVE_VAR
    
    INT8U i;     
    BRTOS_TIMER p;
    INT32U timeout;
    BRTOS_TMR_T* list;
    
    if((cb == NULL) || (cbp == NULL)) return NULL_EVENT_POINTER;    /* return error code */        
    
    if (currentTask)     
     OSEnterCritical();  
    
    // Search available timer control block
    for(i=0;i<=BRTOS_MAX_TIMER;i++)
    {
      
      if(i >= BRTOS_MAX_TIMER)
      {        
        // Exit critical Section
        if (currentTask)
           OSExitCritical();
        
        // Return error code
        return(NO_AVAILABLE_EVENT);
      }
      if(BRTOS_TIMER_VECTOR.mem[i].state == TIMER_NOT_ALLOCATED){
        
        // Exit critical Section
        if (currentTask)
           OSExitCritical();
        
        // Return error code
        return(ERR_EVENT_NO_CREATED);
      }
      
      if(BRTOS_TIMER_VECTOR.mem[i].state == TIMER_NOT_USED)
      {        
        p = &BRTOS_TIMER_VECTOR.mem[i];
        break;      
      }
    }
    
    p->state = TIMER_STOPPED;
    p->func_cb = cb;  // store callback function
    
       
    if(time_wait > 0)
    {      
    
      timeout = (INT32U)((INT32U)OSGetCount() + (INT32U)time_wait);
      
      if (timeout >= TICK_COUNT_OVERFLOW)
      {
        p->timeout = (INT16U)(timeout - TICK_COUNT_OVERFLOW);
        list = BRTOS_TIMER_VECTOR.future;   // add into future list
        list->timers[++list->count] = p; // insert in the end                            
        Subir (list->timers, list->count); // order it 
      }
      else
      {
        p->timeout = (INT16U)timeout;
        list = BRTOS_TIMER_VECTOR.current;  // add into current list
        list->timers[++list->count] = p; // insert in the end                            
        Subir (list->timers, list->count); // order it 
        
        // may need to change wake time of timer task
        if(currentTask)
        {          
          if(p->timeout == (list->timers[1])->timeout)
          {
            ContextTask[BRTOS_TIMER_VECTOR.handling_task].TimeToWait = p->timeout;
          }
        }
                         
      }      
              
      p->state = TIMER_RUNNING;      
                                            
    }
    
    *cbp = p;  
                
    if (currentTask)
        OSExitCritical(); 
    
    return OK;
}

/**
  \fn TIMER_CNT BRTOS_TimerGet (BRTOS_TIMER p)
  \brief public function to get remaining time of a soft timer
  \param p  soft timer
  \return timeout value or "0" as error code
*/
TIMER_CNT OSTimerGet (BRTOS_TIMER p)
{
     
     OS_SR_SAVE_VAR
     TIMER_CNT timeout;
     TIMER_CNT tickcount;
     
     if((p!= NULL) && (p->state == TIMER_RUNNING))
     {
     
        if (currentTask)
            OSEnterCritical();                      
             
            tickcount =  OSGetCount();  
            if(p->timeout >= tickcount)
            {                
                timeout = (TIMER_CNT)(p->timeout - tickcount);                   
            }
            else
            {
                timeout =  (TIMER_CNT)(TIMER_MAX_COUNTER - tickcount +  p->timeout + 1); 
            }  
                          
        if (currentTask)               
            OSExitCritical(); 
        
        return timeout;
      } 
      return 0;  /* "0" null event pointer or timer not running */
    
}



/**
  \fn INT8U BRTOS_TimerStart (BRTOS_TIMER p, TIMER_CNT time_wait)
  \brief public function to start or restart a soft timer
  \param p  soft timer
  \param time_wait soft timer expiration time
  \return OK success
  \return NULL_EVENT_POINTER error code
*/
INT8U OSTimerStart (BRTOS_TIMER p, TIMER_CNT time_wait){
 
  OS_SR_SAVE_VAR
  INT32U timeout;
  BRTOS_TMR_T* list;
  
  if(p!= NULL && time_wait != 0)
  {
      
      if(time_wait> TIMER_MAX_COUNTER) time_wait = TIMER_MAX_COUNTER;
      
      if (currentTask)
          OSEnterCritical();      
        
      if(time_wait > 0)
      {      
    
        timeout = (INT32U)((INT32U)OSGetCount() + (INT32U)time_wait);
        
        if (timeout >= TICK_COUNT_OVERFLOW)
        {
          p->timeout = (TIMER_CNT)(timeout - TICK_COUNT_OVERFLOW);
          list = BRTOS_TIMER_VECTOR.future;   // add into future list
          list->timers[++list->count] = p; // insert in the end                            
          Subir (list->timers, list->count); // order it 
        }
        else
        {
          p->timeout = (TIMER_CNT)timeout;
          list = BRTOS_TIMER_VECTOR.current;  // add into current list
          list->timers[++list->count] = p; // insert in the end                            
          Subir (list->timers, list->count); // order it 
          
          // may need to change wake time of timer task
          if(currentTask)
          {          
            if(p->timeout == (list->timers[1])->timeout)
            {
              ContextTask[BRTOS_TIMER_VECTOR.handling_task].TimeToWait = p->timeout;
            }
          }
                           
        }      
                
        p->state = TIMER_RUNNING;     
                                              
      }                                       
             
      if (currentTask)               
          OSExitCritical();  
      
      return OK;
  } 
  return NULL_EVENT_POINTER; /* any error number */
}


/**
  \fn INT8U BRTOS_TimerStop (BRTOS_TIMER p, INT8U del)
  \brief public function to stop or (stop and delete) a soft timer
  \param p  soft timer
  \param del if "> 0", timer is also deleted
  \return OK success
  \return NULL_EVENT_POINTER error code
*/

INT8U OSTimerStop (BRTOS_TIMER p, INT8U del){
  
  OS_SR_SAVE_VAR
  BRTOS_TMR_T* list;
  INT8U pos_timer = 0;
  
  if(p != NULL)
  {
  
      if (currentTask)
          OSEnterCritical();
     
        
        if(p->timeout >= OSGetCount())
        {                
           list = BRTOS_TIMER_VECTOR.current;  // remove from current list   
        }
        else
        {
           list = BRTOS_TIMER_VECTOR.future;   // remove from future list
        }  
        
        /* search timer index */
        p->state = TIMER_SEARCH;
        for(pos_timer = 1; pos_timer <= list->count; pos_timer++)
        {
        	if(list->timers[pos_timer]->state == TIMER_SEARCH)
        	{
        		break;
        	}
        }
        
        p->timeout = 0;
        Subir (list->timers, pos_timer); // order it 
        list->timers[1]=list->timers[list->count]; // remove from current list
        list->timers[list->count] = NULL;
        list->count--; 
        Descer (list->timers, 1, list->count); // order it
        
        if(del > 0)
        {                     
          p->state = TIMER_NOT_ALLOCATED; 
          p->func_cb = NULL; 
        }
        else
        {
          p->state = TIMER_STOPPED; 
        }
         
      if (currentTask)               
          OSExitCritical();
      
      return OK;
  }
  return NULL_EVENT_POINTER;
}


#endif 
#endif

/*****************************************************************/
/*                          OS SOFT TIMER EOF                    */
/*****************************************************************/
