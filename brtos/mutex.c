/**
* \file mutex.c
* \brief BRTOS Mutex functions
*
* Functions to install and use mutexes
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
*                                          OS Mutex functions
*
*
*   Author:   Gustavo Weber Denardin
*   Revision: 1.1
*   Date:     11/03/2010
*
*   Authors:  Carlos Henrique Barriquelo e Gustavo Weber Denardin
*   Revision: 1.2
*   Date:     01/10/2010
*
*   Authors:  Carlos Henrique Barriquelo e Gustavo Weber Denardin
*   Revision: 1.3
*   Date:     11/10/2010
*
*   Authors:  Carlos Henrique Barriquelo e Gustavo Weber Denardin
*   Revision: 1.4
*   Date:     19/10/2010
*
*   Authors:  Carlos Henrique Barriquelo e Gustavo Weber Denardin
*   Revision: 1.45
*   Date:     20/10/2010
*
*   Authors:  Carlos Henrique Barriquelo e Gustavo Weber Denardin
*   Revision: 1.50
*   Date:     25/10/2010
*
*   Authors:  Carlos Henrique Barriquelo e Gustavo Weber Denardin
*   Revision: 1.61
*   Date:     02/12/2010
*
*   Authors:  Douglas França
*   Revision: 1.62
*   Date:     13/12/2010
*
*   Authors:  Carlos Henrique Barriquelo e Gustavo Weber Denardin
*   Revision: 1.69
*   Date:     05/11/2011
*
*********************************************************************************************************/

#include "BRTOS.h"

#if (PROCESSOR == COLDFIRE_V1)
#pragma warn_implicitconv off
#endif


#if (BRTOS_MUTEX_EN == 1)
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Create Mutex Function                       /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U OSMutexCreate (BRTOS_Mutex **event, INT8U HigherPriority)
{
  OS_SR_SAVE_VAR
  int i=0;

  BRTOS_Mutex *pont_event;

  if (iNesting > 0) {                                // See if caller is an interrupt
      return(IRQ_PEND_ERR);                          // Can't be create by interrupt
  }
    
  // Enter critical Section
  if (currentTask)
     OSEnterCritical();
  
  if (PriorityVector[HigherPriority] != EMPTY_PRIO)
  {
      // Exit critical Section
      if (currentTask)
        OSExitCritical();
      return BUSY_PRIORITY;                          // The priority is busy
  }
  
  // Allocate priority to the mutex
  PriorityVector[HigherPriority] = MUTEX_PRIO;

  // Verifica se ainda há blocos de controle de eventos disponíveis
  for(i=0;i<=BRTOS_MAX_MUTEX;i++)
  {
    
    if(i >= BRTOS_MAX_MUTEX)
    {
      // Caso não haja mais blocos disponíveis, retorna exceção
      
      // Exit critical Section
      if (currentTask)
         OSExitCritical();
      
      return(NO_AVAILABLE_EVENT);
    }
          
    
    if(BRTOS_Mutex_Table[i].OSEventAllocated != TRUE)
    {
      BRTOS_Mutex_Table[i].OSEventAllocated = TRUE;
      pont_event = &BRTOS_Mutex_Table[i];
      break;      
    }
  }  
    

    // Exit Critical
  pont_event->OSEventState = AVAILABLE_RESOURCE;       // Set mutex init value
  pont_event->OSEventWait  = 0;
  pont_event->OSMaxPriority = HigherPriority;          // Determina a tarefa de maior prioridade acessando o mutex

  
  pont_event->OSEventWaitList=0;
  
  *event = pont_event;
  
  // Exit critical Section
  if (currentTask)
     OSExitCritical();  
  
  return(ALLOC_EVENT_OK);
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Delete Mutex Function                       /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U OSMutexDelete (BRTOS_Mutex **event)
{
  OS_SR_SAVE_VAR
  BRTOS_Mutex *pont_event;

  if (iNesting > 0) {                                // See if caller is an interrupt
      return(IRQ_PEND_ERR);                          // Can't be delete by interrupt
  }
    
  // Enter Critical Section
  OSEnterCritical();
  
  pont_event = *event;  
  pont_event->OSEventAllocated   = 0;
  pont_event->OSEventState       = 0;
  pont_event->OSEventOwner       = 0;                        
  pont_event->OSMaxPriority      = 0;                      
  pont_event->OSOriginalPriority = 0;                
  pont_event->OSEventWait        = 0;  
  
  pont_event->OSEventWaitList=0;
  
  *event = NULL;
  
  // Exit Critical Section
  OSExitCritical();
  
  return(DELETE_EVENT_OK);
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Mutex Acquire Function                      /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U OSMutexAcquire(BRTOS_Mutex *pont_event, INT16U time_wait)
{
  OS_SR_SAVE_VAR
  INT8U  iPriority = 0;
  INT32U timeout;
  ContextType *Task;

  
  #if (ERROR_CHECK == 1)
    /// Can not use mutex acquire function from interrupt handling code
    if(iNesting > 0)
    {
      return(IRQ_PEND_ERR);
    }
    
    // Verifies if the pointer is NULL
    if(pont_event == NULL)
    {
      return(NULL_EVENT_POINTER);
    }
  #endif
    
  // Enter Critical Section
  OSEnterCritical();

  #if (ERROR_CHECK == 1)
    // Verifies if the event is allocated
    if(pont_event->OSEventAllocated != TRUE)
    {
      // Exit Critical Section
      OSExitCritical();      
      return(ERR_EVENT_NO_CREATED);
    }
  #endif
  
  // BRTOS TRACE SUPPORT
  #if (OSTRACE == 1)
      #if(OS_TRACE_BY_TASK == 1)
      Update_OSTrace(currentTask, MUTEXPEND);
      #else
      Update_OSTrace(ContextTask[currentTask].Priority, MUTEXPEND);
      #endif 
  #endif    
  
  
  // Verifies if the task is trying to acquire the mutex again
  if (currentTask == pont_event->OSEventOwner) 
  {
    // It is already the mutex owner
    OSExitCritical();
    return OK;
  }
  
  Task = (ContextType*)&ContextTask[currentTask];

  // Verify if the shared resource is available
  if (pont_event->OSEventState == AVAILABLE_RESOURCE)
  {
    // Set shared resource busy
    pont_event->OSEventState = BUSY_RESOURCE;
    
    // Current task becomes the temporary owner of the mutex
    pont_event->OSEventOwner = currentTask;
        
    ///////////////////////////////////////////////////////////////////////////////
    // Performs the temporary exchange of mutex owner priority, if needed        //
    ///////////////////////////////////////////////////////////////////////////////
    
    // Backup the original task priority
    pont_event->OSOriginalPriority = ContextTask[currentTask].Priority;
    
    if (pont_event->OSMaxPriority > ContextTask[currentTask].Priority)
    {
      // Receives the priority ceiling temporarily
      Task->Priority = pont_event->OSMaxPriority;
      
      // Priority vector change       
      PriorityVector[pont_event->OSMaxPriority] = currentTask;
      
      // Remove "original priority current task" from the Ready List
      OSReadyList = OSReadyList & ~(PriorityMask[pont_event->OSOriginalPriority]);
      // Put the "max priority current task" into Ready List
      OSReadyList = OSReadyList | (PriorityMask[pont_event->OSMaxPriority]);
    }
    
    OSExitCritical();
    return OK;
  }
  else
  {
	// If no timeout is used and the mutex is not available, exit the mutex with an error
	if (time_wait == NO_TIMEOUT){
		// Exit Critical Section
		OSExitCritical();
		return EXIT_BY_NO_RESOURCE_AVAILABLE;
	}

	// Copy task priority to local scope
    iPriority = Task->Priority;
    // Increases the mutex wait list counter
    pont_event->OSEventWait++;
    
    // Allocates the current task on the mutex wait list
    pont_event->OSEventWaitList = pont_event->OSEventWaitList | (PriorityMask[iPriority]);
      
    // Task entered suspended state, waiting for mutex release
    #if (VERBOSE == 1)
    ContextTask[currentTask].State = SUSPENDED;
    ContextTask[currentTask].SuspendedType = MUTEX;
    #endif

    // Remove current task from the Ready List
    OSReadyList = OSReadyList & ~(PriorityMask[iPriority]);

    // Set timeout overflow
    if (time_wait)
    {
      timeout = (INT32U)((INT32U)OSGetCount() + (INT32U)time_wait);

      if (timeout >= TICK_COUNT_OVERFLOW)
      {
        Task->TimeToWait = (INT16U)(timeout - TICK_COUNT_OVERFLOW);
      }
      else
      {
        Task->TimeToWait = (INT16U)timeout;
      }

      // Put task into delay list
      IncludeTaskIntoDelayList();
    } else
    {
      Task->TimeToWait = NO_TIMEOUT;
    }
            
    // Change Context - Returns on mutex release
    ChangeContext();
    
    // Exit Critical Section
    OSExitCritical();
    // Enter Critical Section
    OSEnterCritical();    
    
    if (time_wait){
        // Verify if the reason of task wake up was queue timeout
        if(Task->TimeToWait == EXIT_BY_TIMEOUT)
        {
            // Test if both timeout and post have occured before arrive here
            if ((pont_event->OSEventWaitList & PriorityMask[iPriority]))
            {
              // Remove the task from the queue wait list
              pont_event->OSEventWaitList = pont_event->OSEventWaitList & ~(PriorityMask[iPriority]);

              // Decreases the queue wait list counter
              pont_event->OSEventWait--;

              // Exit Critical Section
              OSExitCritical();

              // Indicates resource not available
              return EXIT_BY_NO_RESOURCE_AVAILABLE;
            }
        }
        else
        {
            // Remove the time to wait condition
            Task->TimeToWait = NO_TIMEOUT;

            // Remove from delay list
            RemoveFromDelayList();
        }

    }
    
    // Backup the original task priority
    pont_event->OSOriginalPriority = iPriority;
    
    if (pont_event->OSMaxPriority > iPriority)
    {
      // Receives the priority ceiling temporarily
      Task->Priority = pont_event->OSMaxPriority;
      
      // Priority vector change
      PriorityVector[pont_event->OSMaxPriority] = currentTask;
      
      // Remove "original priority current task" from the Ready List
      OSReadyList = OSReadyList & ~(PriorityMask[iPriority]);
      // Put the "max priority current task" into Ready List
      OSReadyList = OSReadyList | (PriorityMask[pont_event->OSMaxPriority]);
    }
    
    OSExitCritical();
    return OK;
  }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Mutex Release Function                      /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U OSMutexRelease(BRTOS_Mutex *pont_event)
{
  OS_SR_SAVE_VAR
  INT8U iPriority = (INT8U)0;
  #if (VERBOSE == 1)
  INT8U TaskSelect = 0;
  #endif
  
  #if (ERROR_CHECK == 1)      
    /// Can not use mutex pend function from interrupt handling code
    if(iNesting > 0)
    {
      return(IRQ_PEND_ERR);
    }  
  
    // Verifies if the pointer is NULL
    if(pont_event == NULL)
    {
      return(NULL_EVENT_POINTER);
    }
  #endif

  // Enter Critical Section
  OSEnterCritical();
     
  #if (ERROR_CHECK == 1)        
    // Verifies if the event is allocated
    if(pont_event->OSEventAllocated != TRUE)
    {
      // Exit Critical Section
      OSExitCritical();
      return(ERR_EVENT_NO_CREATED);
    }
  #endif
     
  // BRTOS TRACE SUPPORT
  #if (OSTRACE == 1)  
    if(!iNesting){  
      #if(OS_TRACE_BY_TASK == 1)
      Update_OSTrace(currentTask, MUTEXPOST);
      #else
      Update_OSTrace(ContextTask[currentTask].Priority, MUTEXPOST);
      #endif 
    }else{
      Update_OSTrace(0, MUTEXPOST);
    }
  #endif     

  // Verify Mutex Owner
  if (pont_event->OSEventOwner != currentTask)
  {   
    OSExitCritical();
    return ERR_EVENT_OWNER;
  }  
  
  
  // Returns to the original priority, if needed
  // Copy backuped original priority to the task context
  iPriority = ContextTask[currentTask].Priority;
  if (iPriority != pont_event->OSOriginalPriority)
  {              
    // Since current task is executing with another priority, reallocate its priority to the original
    // into the Ready List
    // Remove "max priority current task" from the Ready List
    OSReadyList = OSReadyList & ~(PriorityMask[iPriority]);
    // Put the "original priority current task" into Ready List
    OSReadyList = OSReadyList | (PriorityMask[pont_event->OSOriginalPriority]);
    
    ContextTask[currentTask].Priority = pont_event->OSOriginalPriority;
  }

  // Release mutex ownership
  pont_event->OSEventOwner = 0;
  
  // See if any task is waiting for mutex release
  if (pont_event->OSEventWait != 0)
  {
    // Selects the highest priority task
    iPriority = SAScheduler(pont_event->OSEventWaitList);

    // Remove the selected task from the mutex wait list
    pont_event->OSEventWaitList = pont_event->OSEventWaitList & ~(PriorityMask[iPriority]);
    
    // Decreases the mutex wait list counter
    pont_event->OSEventWait--;
    
    // Changes the task that owns the mutex
    pont_event->OSEventOwner = PriorityVector[iPriority];    
         
    // Indicates that selected task is ready to run
    #if (VERBOSE == 1)
    TaskSelect = PriorityVector[iPriority];
    ContextTask[TaskSelect].State = READY;    
    #endif    
    
    // Put the selected task into Ready List
    OSReadyList = OSReadyList | (PriorityMask[iPriority]);
        
    // Verify if there is a higher priority task ready to run
    ChangeContext();

    // Exit Critical Section
    OSExitCritical();
      
    return OK;
  }
      
  // Release Mutex
  pont_event->OSEventState = AVAILABLE_RESOURCE;
  PriorityVector[pont_event->OSMaxPriority] = MUTEX_PRIO;
      
  // Exit Critical Section
  OSExitCritical();      
  
  return OK;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#endif
