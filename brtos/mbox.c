/**
* \file mbox.c
* \brief BRTOS MailBox functions
*
* Functions to install and use MailBoxes
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
*                                           OS MailBox functions
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
*   Revision: 1.60
*   Date:     30/11/2010
*
*   Authors:  Carlos Henrique Barriquelo e Gustavo Weber Denardin
*   Revision: 1.61
*   Date:     02/12/2010
*
*   Authors:  Douglas França
*   Revision: 1.62
*   Date:     13/12/2010
*
*   Authors:  Carlos Henrique Barriquelo
*   Revision: 1.64
*   Date:     22/02/2011
*
*   Authors:  Gustavo Weber Denardin
*   Revision: 1.76
*   Date:     11/10/2012
*
*   Authors:  Gustavo Weber Denardin
*   Revision: 1.80
*   Date:     11/11/2015
*
*********************************************************************************************************/

#include "BRTOS.h"

#if (PROCESSOR == COLDFIRE_V1)
#pragma warn_implicitconv off
#endif


#if (BRTOS_MBOX_EN == 1)
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Create MailBox Function                     /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U OSMboxCreate (BRTOS_Mbox **event, void *message)
{
  OS_SR_SAVE_VAR
  INT16S i = 0;  
  BRTOS_Mbox *pont_event;

  if (iNesting > 0) {                                // See if caller is an interrupt
      return(IRQ_PEND_ERR);                          // Can't be create by interrupt
  }
    
  // Enter critical Section
  if (currentTask)
     OSEnterCritical();
  
  // Verifica se ainda há blocos de controle de eventos disponíveis
  for(i=0;i<=BRTOS_MAX_MBOX;i++)
  {
    
    if(i >= BRTOS_MAX_MBOX)
    {
      // Caso não haja mais blocos disponíveis, retorna exceção
      
      // Exit critical Section
      if (currentTask)
         OSExitCritical();      
      
      return(NO_AVAILABLE_EVENT);
    }
          
    
    if(BRTOS_Mbox_Table[i].OSEventAllocated != TRUE)
    {
      BRTOS_Mbox_Table[i].OSEventAllocated = TRUE;
      pont_event = &BRTOS_Mbox_Table[i];
      break;      
    }
  }    
    
  if (message != NULL)
  {
    pont_event->OSEventState = AVAILABLE_MESSAGE;
  }
  else
  {
    pont_event->OSEventState = NO_MESSAGE;
  }
  
  pont_event->OSEventPointer   = message;
  pont_event->OSEventWait      = 0;  
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

INT8U OSMboxDelete (BRTOS_Mbox **event)
{
  OS_SR_SAVE_VAR
  BRTOS_Mbox *pont_event;

  if (iNesting > 0) {                                // See if caller is an interrupt
      return(IRQ_PEND_ERR);                          // Can't be delete by interrupt
  }
    
  // Enter Critical Section
  OSEnterCritical();
  
  pont_event = *event;
  pont_event->OSEventAllocated   = 0;
  pont_event->OSEventPointer     = NULL;
  pont_event->OSEventWait        = 0;
  pont_event->OSEventState       = NO_MESSAGE;
  
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
/////      Mailbox Pend Function                       /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U OSMboxPend (BRTOS_Mbox *pont_event, void **Mail, INT16U time_wait)
{
  OS_SR_SAVE_VAR
  INT8U  iPriority = 0;
  INT32U  timeout;
  ContextType *Task;  
  
  #if (ERROR_CHECK == 1)
    /// Can not use mailbox pend function from interrupt handling code
    if(iNesting > 0)
    {
      // Return NULL message
      *Mail = (void *)NULL;
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
  
  // Verify if there was a message post
  if (pont_event->OSEventState == AVAILABLE_MESSAGE)
  {
    // Copy message pointer
    *Mail = pont_event->OSEventPointer;
    
    // Free message slot
    pont_event->OSEventState = NO_MESSAGE;
    
    // Exit Critical Section
    OSExitCritical();
    return OK;
  }
  else
  {
  	// If no timeout is used and the mailbox is empty, exit the mailbox with an error
	if (time_wait == NO_TIMEOUT){
		// Exit Critical Section
	    OSExitCritical();
	    return EXIT_BY_NO_ENTRY_AVAILABLE;
	}
  	
    Task = (ContextType*)&ContextTask[currentTask];
      
    // Copy task priority to local scope
    iPriority = Task->Priority;
    
    // Increases the semaphore wait list counter
    pont_event->OSEventWait++;
    
    // Allocates the current task on the mailbox wait list
    pont_event->OSEventWaitList = pont_event->OSEventWaitList | (PriorityMask[iPriority]);
    
    // Task entered suspended state, waiting for mailbox post
    #if (VERBOSE == 1)
    Task->State = SUSPENDED;
    Task->SuspendedType = MAILBOX;
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
    
    // Change Context - Returns on time overflow or mailbox post
    ChangeContext();

    // Exit Critical Section
    OSExitCritical();
    // Enter Critical Section
    OSEnterCritical();
    
    if (time_wait)
    {    
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
              
              // Return NULL message
              *Mail = (void *)NULL;              
              
              // Exit Critical Section
              OSExitCritical();
              
              // Indicates queue timeout
              return TIMEOUT;
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
    
    // Copy message pointer
    *Mail = pont_event->OSEventPointer;
    
    // Free message slot
    pont_event->OSEventState = NO_MESSAGE;
    
    // Exit Critical Section
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
/////      Mailbox Post Function                       /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U OSMboxPost(BRTOS_Mbox *pont_event, void *message)
{
  OS_SR_SAVE_VAR
  INT8U iPriority = (INT8U)0;
  #if (VERBOSE == 1)
  INT8U TaskSelect = 0;  
  #endif
  
  #if (ERROR_CHECK == 1)    
    // Verifies if the pointer is NULL
    if(pont_event == NULL)
    {
      return(NULL_EVENT_POINTER);
    }
  #endif

  // Enter Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSEnterCritical();
     
  #if (ERROR_CHECK == 1)        
    // Verifies if the event is allocated
    if(pont_event->OSEventAllocated != TRUE)
    {
      // Exit Critical Section
      #if (NESTING_INT == 0)
      if (!iNesting)
      #endif
         OSExitCritical();
      return(ERR_EVENT_NO_CREATED);
    }
  #endif
       
  // See if any task is waiting for a message
  if (pont_event->OSEventWait != 0)
  {
    // Selects the highest priority task
    iPriority = SAScheduler(pont_event->OSEventWaitList);

    // Remove the selected task from the mailbox wait list
    pont_event->OSEventWaitList = pont_event->OSEventWaitList & ~(PriorityMask[iPriority]);
    
    // Decreases the mailbox wait list counter
    pont_event->OSEventWait--;
    
    // Put the selected task into Ready List
    #if (VERBOSE == 1)
    TaskSelect = PriorityVector[iPriority];
    ContextTask[TaskSelect].State = READY;
    #endif
    
    OSReadyList = OSReadyList | (PriorityMask[iPriority]);
    
    // Copy message pointer
    pont_event->OSEventPointer = message;
    
    // Free message slot
    pont_event->OSEventState = AVAILABLE_MESSAGE;
    
    // If outside of an interrupt service routine, change context to the highest priority task
    // If inside of an interrupt, the interrupt itself will change the context to the highest priority task
    if (!iNesting)
    {
      // Verify if there is a higher priority task ready to run
      ChangeContext();      
    }

    // Exit Critical Section
    #if (NESTING_INT == 0)
    if (!iNesting)
    #endif
      OSExitCritical();

    return OK;
  }
  else
  {
    // Copy message pointer
    pont_event->OSEventPointer = message;
    
    // Free message slot
    pont_event->OSEventState = AVAILABLE_MESSAGE;
      
  // Exit Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
      OSExitCritical();
    
    return OK;
  }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#endif
