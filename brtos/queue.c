/**
* \file queue.c
* \brief BRTOS Queue functions
*
* Functions to install and use queues
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
*                                         OS Queue functions
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
*   Authors:  Gustavo Weber Denardin e Carlos Henrique Barriquello
*   Revision: 1.75
*   Date:     24/08/2012
*
*   Authors:  Gustavo Weber Denardin
*   Revision: 1.76
*   Date:     11/10/2012
*
*   Authors:  Gustavo Weber Denardin
*   Revision: 1.80
*   Date:     11/11/2015
*
*   Authors:  Gustavo Weber Denardin
*   Revision: 1.90
*   Date:     14/11/2015
*
*********************************************************************************************************/

#include "BRTOS.h"

#if (PROCESSOR == COLDFIRE_V1 && __CWCC__)
#pragma warn_implicitconv off
#endif


#if (BRTOS_QUEUE_EN == 1)
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Create Queue Function                       /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSQueueCreate(uint16_t size, BRTOS_Queue **event)
{
  OS_SR_SAVE_VAR
  int16_t i=0;
  BRTOS_Queue *pont_event;
  OS_QUEUE *cqueue; 

  if (iNesting > 0) {                                // See if caller is an interrupt
     return(IRQ_PEND_ERR);                           // Can't be create by interrupt
  }
    
  // Enter critical Section
  if (currentTask)
     OSEnterCritical();
  
  // Fix the queue size to the OS_CPU_TYPE
  if (size % sizeof(OS_CPU_TYPE))
  {
    size = (uint16_t)(size + (sizeof(OS_CPU_TYPE) - (size % sizeof(OS_CPU_TYPE))));
  }
  
  if ((iQueueAddress + (size / sizeof(OS_CPU_TYPE))) > (QUEUE_HEAP_SIZE / sizeof(OS_CPU_TYPE)))
  {
      // Exit critical Section
      if (currentTask)
         OSExitCritical();
      
       return NO_MEMORY;
  }  
  
  // Verifica se ainda há blocos de controle de eventos disponíveis
  for(i=0;i<=BRTOS_MAX_QUEUE;i++)
  {
    
    if(i >= BRTOS_MAX_QUEUE)
    {
      // Caso não haja mais blocos disponíveis, retorna exceção
      
      // Exit critical Section
      if (currentTask)
         OSExitCritical();
      
      return(NO_AVAILABLE_EVENT);
    }
          
    
    if(BRTOS_Queue_Table[i].OSEventAllocated != TRUE)
    {
      BRTOS_Queue_Table[i].OSEventAllocated = TRUE;
      pont_event = &BRTOS_Queue_Table[i];
      cqueue = &BRTOS_OS_QUEUE_Table[i];
      break;      
    }
  } 
  
  // Configura dados de evento de lista
  cqueue->OSQStart    = (uint8_t *)&QUEUE_STACK[iQueueAddress];
  iQueueAddress       = (uint16_t)(iQueueAddress + (size / sizeof(OS_CPU_TYPE)));
  cqueue->OSQSize     = size;
  cqueue->OSQEntries  = 0;
  cqueue->OSQEnd      = cqueue->OSQStart + cqueue->OSQSize;
  cqueue->OSQIn       = cqueue->OSQStart;
  cqueue->OSQOut      = cqueue->OSQStart;
  
  // Aloca tipo de evento e dados do evento
  pont_event->OSEventPointer = cqueue;
  pont_event->OSEventWait = 0;
  
  
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
/////      Write Queue Function                        /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSWQueue(OS_QUEUE *cqueue,uint8_t data)
{  
  OS_SR_SAVE_VAR
  
  // Enter Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSEnterCritical();
  
  if (cqueue->OSQEntries < cqueue->OSQSize)
  {  
    cqueue->OSQEntries++;
  }
  else
  { 
     // Exit Critical Section
     #if (NESTING_INT == 0)
     if (!iNesting)
     #endif
       OSExitCritical();
       
     return BUFFER_UNDERRUN;
  }
  
  if (cqueue->OSQIn == cqueue->OSQEnd)
    cqueue->OSQIn = cqueue->OSQStart;
  
  *cqueue->OSQIn = data;
  cqueue->OSQIn++; 
 
   // Exit Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
      OSExitCritical();
  
  return WRITE_BUFFER_OK;
  
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Read Queue Function                         /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSRQueue(OS_QUEUE *cqueue, uint8_t* pdata)
{
  OS_SR_SAVE_VAR
  
  // Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSEnterCritical();
  
  if(cqueue->OSQEntries > 0)
  {
      
    if (cqueue->OSQOut == cqueue->OSQEnd)
      cqueue->OSQOut = cqueue->OSQStart;
  
    *pdata = *(cqueue->OSQOut);
  
    cqueue->OSQOut++;
    cqueue->OSQEntries--;
    
    // Exit Critical Section
    #if (NESTING_INT == 0)
    if (!iNesting)
    #endif
      OSExitCritical();
      
    return READ_BUFFER_OK;
  }
  else
  {
    // Exit Critical Section
    #if (NESTING_INT == 0)
    if (!iNesting)
    #endif
      OSExitCritical();
      
    return NO_ENTRY_AVAILABLE;
  }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Clean Queue Function                        /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSQueueClean(BRTOS_Queue *pont_event)
{
  OS_SR_SAVE_VAR
  OS_QUEUE *cqueue = pont_event->OSEventPointer;
  
  // Enter Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSEnterCritical();
  
  cqueue->OSQEntries = 0;
  
  cqueue->OSQIn = cqueue->OSQStart;
  cqueue->OSQOut = cqueue->OSQStart;
  
  // Exit Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
    OSExitCritical();  

  return CLEAN_BUFFER_OK;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Queue Pend Function                         /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSQueuePend (BRTOS_Queue *pont_event, uint8_t* pdata, ostick_t time_wait)
{
  OS_SR_SAVE_VAR
  uint8_t iPriority = 0;
  osdtick_t timeout;
  ContextType *Task;
  OS_QUEUE *cqueue = pont_event->OSEventPointer;
   
  #if (ERROR_CHECK == 1)
    /// Can not use Queue pend function from interrupt handling code
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
      Update_OSTrace(currentTask, QUEUEPEND);
      #else
      Update_OSTrace(ContextTask[currentTask].Priority, QUEUEPEND);
      #endif
  #endif   
    
  // Verify if there is data in the queue
  if(cqueue->OSQEntries > 0)
  {
    // Verify for output pointer overflow
    if (cqueue->OSQOut == cqueue->OSQEnd)
      cqueue->OSQOut = cqueue->OSQStart;
  
    // Copy data from queue
    *pdata = *cqueue->OSQOut;
  
    // Increases output pointer
    cqueue->OSQOut++;
    
    // Decreases queue entries
    cqueue->OSQEntries--;
    
    // Exit Critical Section
    OSExitCritical();
    return READ_BUFFER_OK;
  }
  else
  {
  	// If no timeout is used and the queue is empty, exit the queue with an error
	if (time_wait == NO_TIMEOUT){
		// Exit Critical Section
	    OSExitCritical();
	    return EXIT_BY_NO_ENTRY_AVAILABLE;
	}  	
  	
    Task = (ContextType*)&ContextTask[currentTask];
    
    // Copy task priority to local scope
    iPriority = Task->Priority;
  
    // Increases the queue wait list counter
    pont_event->OSEventWait++;
    
    // Allocates the current task on the queue wait list
    pont_event->OSEventWaitList = pont_event->OSEventWaitList | (PriorityMask[iPriority]);
  
    // Task entered suspended state, waiting for queue post
    #if (VERBOSE == 1)
    Task->State = SUSPENDED;
    Task->SuspendedType = QUEUE;
    #endif

    // Remove current task from the Ready List
    OSReadyList = OSReadyList & ~(PriorityMask[iPriority]);
  
    // Set timeout overflow
    if (time_wait)
    {  
  	  timeout = (osdtick_t)((osdtick_t)OSGetCount() + (osdtick_t)time_wait);
      
  	  if (sizeof_ostick_t < 8){
  		  if (timeout >= TICK_COUNT_OVERFLOW)
  		  {
  			  Task->TimeToWait = (ostick_t)(timeout - TICK_COUNT_OVERFLOW);
  		  }
  		  else
  		  {
  			  Task->TimeToWait = (ostick_t)timeout;
  		  }
  	  }else{
  		  Task->TimeToWait = (ostick_t)timeout;
  	  }
    
      // Put task into delay list
      IncludeTaskIntoDelayList();
    } else
    {
      Task->TimeToWait = NO_TIMEOUT;
    }
  
    // Change Context - Returns on time overflow or queue post
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
    
    // Verify for output pointer overflow
    if (cqueue->OSQOut == cqueue->OSQEnd)
      cqueue->OSQOut = cqueue->OSQStart;
  
    // Copy data from queue
    *pdata = *cqueue->OSQOut;
  
    // Increases the output pointer
    cqueue->OSQOut++;
    
    // Decreases queue entries
    cqueue->OSQEntries--;
    
    // Exit Critical Section
    OSExitCritical();
    return READ_BUFFER_OK;
  }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Post Queue Function                        /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSQueuePost(BRTOS_Queue *pont_event, uint8_t data)
{
  OS_SR_SAVE_VAR
  uint8_t iPriority = (uint8_t)0;
  #if (VERBOSE == 1)
  uint8_t TaskSelect = 0;
  #endif
  OS_QUEUE *cqueue = pont_event->OSEventPointer;
  
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
     
  // BRTOS TRACE SUPPORT
  #if (OSTRACE == 1)  
    if(!iNesting){ 
      #if(OS_TRACE_BY_TASK == 1)
      Update_OSTrace(currentTask, QUEUEPOST);
      #else
      Update_OSTrace(ContextTask[currentTask].Priority, QUEUEPOST);
      #endif
    }else{
      Update_OSTrace(0, QUEUEPOST);
    }
  #endif       
  
  // Checks for queue overflow
  if (cqueue->OSQEntries < cqueue->OSQSize)
  {  
    // If no, increases the queue entries
    cqueue->OSQEntries++;
  }
  else
  { 
     // Exit Critical Section
     #if (NESTING_INT == 0)
     if (!iNesting)
     #endif
       OSExitCritical();
     
     // Indicates queue overflow
     return BUFFER_UNDERRUN;
  }
  
  // Verify for input pointer overflow
  if (cqueue->OSQIn == cqueue->OSQEnd)
    cqueue->OSQIn = cqueue->OSQStart;
  
  // copy data into the queue
  *cqueue->OSQIn = data;
  
  // increases the input pointer
  cqueue->OSQIn++;
  
  // See if any task is waiting for new data in the queue
  if (pont_event->OSEventWait != 0)
  {
    // Selects the highest priority task
    iPriority = SAScheduler(pont_event->OSEventWaitList);    

    // Remove the selected task from the queue wait list
    pont_event->OSEventWaitList = pont_event->OSEventWaitList & ~(PriorityMask[iPriority]);
    
    // Decreases the queue wait list counter
    pont_event->OSEventWait--;
    
    // Put the selected task into Ready List
    #if (VERBOSE == 1)
    TaskSelect = PriorityVector[iPriority];
    ContextTask[TaskSelect].State = READY;
    #endif
    
    OSReadyList = OSReadyList | (PriorityMask[iPriority]);
    
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
    
    return WRITE_BUFFER_OK;
  }
  else
  {
    // Exit Critical Section
    #if (NESTING_INT == 0)
    if (!iNesting)
    #endif
       OSExitCritical();
    
    return WRITE_BUFFER_OK;
  }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#endif



#if (BRTOS_QUEUE_16_EN == 1)
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Create Queue Function                       /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSQueue16Create(OS_QUEUE_16 *cqueue, uint16_t size)
{
  OS_SR_SAVE_VAR

  if (iNesting > 0) {                                // See if caller is an interrupt
     return(IRQ_PEND_ERR);                           // Can't be create by interrupt
  }
    
  // Enter critical Section
  if (currentTask)
     OSEnterCritical();
  
  // Fix the queue size to the OS_CPU_TYPE
  if ((size*sizeof(uint16_t)) % sizeof(OS_CPU_TYPE))
  {
    size = (uint16_t)(size + 1);
  }
  
  if ((iQueueAddress + ((size*sizeof(uint16_t)) / sizeof(OS_CPU_TYPE))) > (QUEUE_HEAP_SIZE / sizeof(OS_CPU_TYPE)))
  {
      // Exit critical Section
      if (currentTask)
         OSExitCritical();
      
       return NO_MEMORY;
  }
  
  // Configura dados de evento de lista
  cqueue->OSQStart    = (uint16_t *)&QUEUE_STACK[iQueueAddress];
  iQueueAddress       = (uint16_t)(iQueueAddress + ((size*sizeof(uint16_t)) / sizeof(OS_CPU_TYPE)));
  cqueue->OSQSize     = size;
  cqueue->OSQEntries  = 0;
  cqueue->OSQEnd      = cqueue->OSQStart + cqueue->OSQSize;
  cqueue->OSQIn       = cqueue->OSQStart;
  cqueue->OSQOut      = cqueue->OSQStart;
  
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
/////      Write Queue Function                        /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSWQueue16(OS_QUEUE_16 *cqueue,uint16_t data)
{
  OS_SR_SAVE_VAR
  
  // Enter Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSEnterCritical();
  
  if (cqueue->OSQEntries < cqueue->OSQSize)
  {  
    cqueue->OSQEntries++;
  }
  else
  { 
     // Exit Critical Section
     #if (NESTING_INT == 0)
     if (!iNesting)
     #endif
       OSExitCritical();
       
     return BUFFER_UNDERRUN;
  }
  
  if (cqueue->OSQIn == cqueue->OSQEnd)
    cqueue->OSQIn = cqueue->OSQStart;
  
  *cqueue->OSQIn = data;
  cqueue->OSQIn++;
  
   // Exit Critical Section
   #if (NESTING_INT == 0)
   if (!iNesting)
   #endif
      OSExitCritical();
  
  return WRITE_BUFFER_OK;
  
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Read Queue Function                         /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSRQueue16(OS_QUEUE_16 *cqueue, uint16_t *pdata)
{
  OS_SR_SAVE_VAR
    
  // Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSEnterCritical();
  
  if(cqueue->OSQEntries > 0)
  {
      
    if (cqueue->OSQOut == cqueue->OSQEnd)
      cqueue->OSQOut = cqueue->OSQStart;
  
    *pdata = *cqueue->OSQOut;
  
    cqueue->OSQOut++;
    cqueue->OSQEntries--;
    
    // Exit Critical Section
    #if (NESTING_INT == 0)
    if (!iNesting)
    #endif
      OSExitCritical();
      
    return READ_BUFFER_OK;
  }
  else
  {
    // Exit Critical Section
    #if (NESTING_INT == 0)
    if (!iNesting)
    #endif
      OSExitCritical();
      
    return NO_ENTRY_AVAILABLE;
  }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Clean Queue Function                        /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSCleanQueue16(OS_QUEUE_16 *cqueue)
{
  OS_SR_SAVE_VAR
  OS_QUEUE_16 *tmp_queue = cqueue;
  
  // Enter Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSEnterCritical();
  
  cqueue->OSQEntries = 0;
  
  cqueue->OSQIn = tmp_queue->OSQStart;
  cqueue->OSQOut = tmp_queue->OSQStart;
  
  // Exit Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
    OSExitCritical();  

  return CLEAN_BUFFER_OK;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#endif






#if (BRTOS_QUEUE_32_EN == 1)
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Create Queue Function                       /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSQueue32Create(OS_QUEUE_32 *cqueue, uint16_t size)
{
  OS_SR_SAVE_VAR

  if (iNesting > 0) {                                // See if caller is an interrupt
     return(IRQ_PEND_ERR);                           // Can't be create by interrupt
  }
    
  // Enter critical Section
  if (currentTask)
     OSEnterCritical();
  
  if ((iQueueAddress + ((size*sizeof(uint32_t)) / sizeof(OS_CPU_TYPE))) > (QUEUE_HEAP_SIZE / sizeof(OS_CPU_TYPE)))
  {
      // Exit critical Section
      if (currentTask)
         OSExitCritical();
      
       return NO_MEMORY;
  }  
  
  // Configura dados de evento de lista
  cqueue->OSQStart    = (uint32_t *)&QUEUE_STACK[iQueueAddress];
  iQueueAddress       = (uint16_t)(iQueueAddress + ((size*sizeof(uint32_t)) / sizeof(OS_CPU_TYPE)));
  cqueue->OSQSize     = size;
  cqueue->OSQEntries  = 0;
  cqueue->OSQEnd      = cqueue->OSQStart + cqueue->OSQSize;
  cqueue->OSQIn       = cqueue->OSQStart;
  cqueue->OSQOut      = cqueue->OSQStart;
  
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
/////      Write Queue Function                        /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSWQueue32(OS_QUEUE_32 *cqueue,uint32_t data)
{
  OS_SR_SAVE_VAR
  
  // Enter Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSEnterCritical();
  
  if (cqueue->OSQEntries < cqueue->OSQSize)
  {  
    cqueue->OSQEntries++;
  }
  else
  { 
     // Exit Critical Section
     #if (NESTING_INT == 0)
     if (!iNesting)
     #endif
       OSExitCritical();
       
     return BUFFER_UNDERRUN;
  }
  
  if (cqueue->OSQIn == cqueue->OSQEnd)
    cqueue->OSQIn = cqueue->OSQStart;
  
  *cqueue->OSQIn = data;
  cqueue->OSQIn++;
  
   // Exit Critical Section
   #if (NESTING_INT == 0)
   if (!iNesting)
   #endif
      OSExitCritical();
  
  return WRITE_BUFFER_OK;
  
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Read Queue Function                         /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSRQueue32(OS_QUEUE_32 *cqueue, uint32_t *pdata)
{
  OS_SR_SAVE_VAR
    
  // Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSEnterCritical();
  
  if(cqueue->OSQEntries > 0)
  {
      
    if (cqueue->OSQOut == cqueue->OSQEnd)
      cqueue->OSQOut = cqueue->OSQStart;
  
    *pdata = *cqueue->OSQOut;
  
    cqueue->OSQOut++;
    cqueue->OSQEntries--;
    
    // Exit Critical Section
    #if (NESTING_INT == 0)
    if (!iNesting)
    #endif
      OSExitCritical();
      
    return READ_BUFFER_OK;
  }
  else
  {
    // Exit Critical Section
    #if (NESTING_INT == 0)
    if (!iNesting)
    #endif
      OSExitCritical();
      
    return NO_ENTRY_AVAILABLE;
  }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Clean Queue Function                        /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSCleanQueue32(OS_QUEUE_32 *cqueue)
{
  OS_SR_SAVE_VAR
  OS_QUEUE_32 *tmp_queue = cqueue;
  
  // Enter Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSEnterCritical();
  
  cqueue->OSQEntries = 0;
  
  cqueue->OSQIn = tmp_queue->OSQStart;
  cqueue->OSQOut = tmp_queue->OSQStart;
  
  // Exit Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
    OSExitCritical();  

  return CLEAN_BUFFER_OK;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#endif


#if (BRTOS_DYNAMIC_QUEUE_ENABLED == 1)

///// Memory allocation definition tests
#ifndef BRTOS_ALLOC
	#error("You must define the BRTOS memory allocation method in BRTOSConfig.h file !!!")
#endif

#ifndef BRTOS_DEALLOC
	#error("You must define the BRTOS memory deallocation method in BRTOSConfig.h file !!!")
#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Create Dynamic Queue Function               /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSDQueueCreate(uint16_t queue_length, OS_CPU_TYPE type_size, BRTOS_Queue **event)
{
  OS_SR_SAVE_VAR
  uint16_t      size_in_bytes = 0;
  BRTOS_Queue *pont_event   = NULL;
  OS_DQUEUE   *cqueue       = NULL;

  if (iNesting > 0) {                                // See if caller is an interrupt
     return(IRQ_PEND_ERR);                           // Can't be create by interrupt
  }
    
  // Enter critical Section
  if (currentTask)
     OSEnterCritical();

	if((queue_length > 0) && (type_size > 0))
	{
		// Allocate the queue handler
		cqueue = (OS_DQUEUE*)BRTOS_ALLOC(sizeof(OS_DQUEUE));
		if( cqueue != NULL )
		{
			// Calculate the queue size in bytes
			size_in_bytes = (uint16_t)(queue_length * type_size);

			// Allocate the queue in the heap
			cqueue->OSQStart = (uint8_t*)BRTOS_ALLOC(size_in_bytes);
			
			if(cqueue->OSQStart != NULL)
			{  
				// Verifies if there is available event control block
				pont_event = (BRTOS_Queue*)BRTOS_ALLOC(sizeof(BRTOS_Queue));

				if(pont_event == NULL){
					// If there is not, deallocate data and return exception
					BRTOS_DEALLOC(cqueue->OSQStart);
					BRTOS_DEALLOC(cqueue);

					// Exit critical Section
					if (currentTask)
					   OSExitCritical();

					return(NO_AVAILABLE_EVENT);
				}
			}else 
			{
				// Deallocate queue handler
				BRTOS_DEALLOC(cqueue);

				// Exit critical Section
				if (currentTask)
				   OSExitCritical();

				return(NO_AVAILABLE_MEMORY);
			}
		}else 
		{
        // Exit critical Section
        if (currentTask)
           OSExitCritical();
        
        return(NO_AVAILABLE_MEMORY);
		}
	}else 
	{
      // If queue length or type size equal to zero, do not allocate the queue
      
      // Exit critical Section
      if (currentTask)
         OSExitCritical();
      
      return(INVALID_PARAMETERS);	
	}
  
  // Configura dados de evento de lista
  cqueue->OSQLength  = queue_length;
  cqueue->OSQTSize   = (uint16_t)type_size;
  cqueue->OSQEntries = 0;
  cqueue->OSQEnd     = cqueue->OSQStart + size_in_bytes;
  cqueue->OSQIn      = cqueue->OSQStart;
  cqueue->OSQOut     = cqueue->OSQStart;
  
  // Aloca tipo de evento e dados do evento
  pont_event->OSEventPointer = cqueue;
  pont_event->OSEventWait = 0;
  
  
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
/////      Delete Dynamic Queue Function               /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSDQueueDelete (BRTOS_Queue **event)
{
  OS_SR_SAVE_VAR
  BRTOS_Queue   *pont_event = *event;
  OS_DQUEUE     *cqueue     = pont_event->OSEventPointer;

  if (iNesting > 0) {                                // See if caller is an interrupt
      return(IRQ_PEND_ERR);                          // Can't be delete by interrupt
  }
    
  // Enter Critical Section
  OSEnterCritical();
  
  BRTOS_DEALLOC(cqueue->OSQStart);
  BRTOS_DEALLOC(cqueue);
    
  pont_event->OSEventAllocated = 0;
  pont_event->OSEventCount     = 0;                      
  pont_event->OSEventWait      = 0;
  pont_event->OSEventWaitList=0;
  
  BRTOS_DEALLOC(pont_event);
  
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
/////      Clean Dynamic Queue Function                /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSDQueueClean(BRTOS_Queue *pont_event)
{
  OS_SR_SAVE_VAR
  OS_DQUEUE *cqueue = pont_event->OSEventPointer;
  
  // Enter Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSEnterCritical();
  
  cqueue->OSQEntries  = 0;
  cqueue->OSQIn       = cqueue->OSQStart;
  cqueue->OSQOut      = cqueue->OSQStart;
  
  // Exit Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
    OSExitCritical();  

  return CLEAN_BUFFER_OK;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Dynamic Queue Pend Function                 /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSDQueuePend (BRTOS_Queue *pont_event, void *pdata, ostick_t time_wait)
{
  OS_SR_SAVE_VAR
  uint8_t       iPriority = 0;
  osdtick_t   timeout;
  uint16_t      n;  
  ContextType *Task;
  OS_DQUEUE   *cqueue;
  uint8_t       *dst;  
   
  #if (ERROR_CHECK == 1)
    /// Can not use Queue pend function from interrupt handling code
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
  cqueue  = pont_event->OSEventPointer;
  n       = cqueue->OSQTSize;
  dst     = (uint8_t*)pdata;

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
      Update_OSTrace(currentTask, QUEUEPEND);
      #else
      Update_OSTrace(ContextTask[currentTask].Priority, QUEUEPEND);
      #endif
  #endif   
    
  // Verify if there is data in the queue
  if(cqueue->OSQEntries > 0)
  {
    // Verify for output pointer overflow
    if (cqueue->OSQOut == cqueue->OSQEnd)
      cqueue->OSQOut = cqueue->OSQStart;
  
    // Copy data from queue
    while(n) 
    {
      // copy data and increase the input pointer
      *dst++ = *cqueue->OSQOut++;
      n--;
    }
    
    // Decreases queue entries
    cqueue->OSQEntries--;
    
    // Exit Critical Section
    OSExitCritical();
    return READ_BUFFER_OK;
  }
  else
  {
  	// If no timeout is used and the queue is empty, exit the queue with an error
	if (time_wait == NO_TIMEOUT){
		// Exit Critical Section
	    OSExitCritical();
	    return EXIT_BY_NO_ENTRY_AVAILABLE;
	}    	
  	
    Task = (ContextType*)&ContextTask[currentTask];
    
    // Copy task priority to local scope
    iPriority = Task->Priority;
  
    // Increases the queue wait list counter
    pont_event->OSEventWait++;
    
    // Allocates the current task on the queue wait list
    pont_event->OSEventWaitList = pont_event->OSEventWaitList | (PriorityMask[iPriority]);
  
    // Task entered suspended state, waiting for queue post
    #if (VERBOSE == 1)
    Task->State = SUSPENDED;
    Task->SuspendedType = QUEUE;
    #endif

    // Remove current task from the Ready List
    OSReadyList = OSReadyList & ~(PriorityMask[iPriority]);
  
    // Set timeout overflow
    if (time_wait)
    {  
  	  timeout = (osdtick_t)((osdtick_t)OSGetCount() + (osdtick_t)time_wait);
      
  	  if (sizeof_ostick_t < 8){
  		  if (timeout >= TICK_COUNT_OVERFLOW)
  		  {
  			  Task->TimeToWait = (ostick_t)(timeout - TICK_COUNT_OVERFLOW);
  		  }
  		  else
  		  {
  			  Task->TimeToWait = (ostick_t)timeout;
  		  }
  	  }else{
  		  Task->TimeToWait = (ostick_t)timeout;
  	  }
    
      // Put task into delay list
      IncludeTaskIntoDelayList();
    } else
    {
      Task->TimeToWait = NO_TIMEOUT;
    }
  
    // Change Context - Returns on time overflow or queue post
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
    
    // Verify for output pointer overflow
    if (cqueue->OSQOut == cqueue->OSQEnd)
      cqueue->OSQOut = cqueue->OSQStart;
  
    // Copy data from queue
    while(n) 
    {
      // copy data and increase the input pointer
      *dst++ = *cqueue->OSQOut++;
      n--;
    }
    
    // Decreases queue entries
    cqueue->OSQEntries--;
    
    // Exit Critical Section
    OSExitCritical();
    return READ_BUFFER_OK;
  }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Post Dynamic Queue Function                 /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uint8_t OSDQueuePost(BRTOS_Queue *pont_event, void *pdata)
{
  OS_SR_SAVE_VAR
  uint8_t iPriority = (uint8_t)0;
  
  #if (VERBOSE == 1)
  uint8_t TaskSelect = 0;
  #endif
  
  uint16_t    n;
  uint8_t     *src;
  OS_DQUEUE *cqueue;
  
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
     
  cqueue  = pont_event->OSEventPointer;
  src     = (uint8_t*)pdata;
  n       = cqueue->OSQTSize;

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
     
  // BRTOS TRACE SUPPORT
  #if (OSTRACE == 1)  
    if(!iNesting){ 
      #if(OS_TRACE_BY_TASK == 1)
      Update_OSTrace(currentTask, QUEUEPOST);
      #else
      Update_OSTrace(ContextTask[currentTask].Priority, QUEUEPOST);
      #endif
    }else{
      Update_OSTrace(0, QUEUEPOST);
    }
  #endif       
  
  // Checks for queue overflow
  if (cqueue->OSQEntries < cqueue->OSQLength)
  {  
    // If no, increases the queue entries
    cqueue->OSQEntries++;
  }
  else
  { 
     // Exit Critical Section
     #if (NESTING_INT == 0)
     if (!iNesting)
     #endif
       OSExitCritical();
     
     // Indicates queue overflow
     return BUFFER_UNDERRUN;
  }
  
  // Verify for input pointer overflow
  if (cqueue->OSQIn == cqueue->OSQEnd)
    cqueue->OSQIn = cqueue->OSQStart;
  
  // copy data into the queue
  while(n) 
  {
    // copy data and increase the input pointer
    *cqueue->OSQIn++ = *src++;
    n--;
  } 
  
  // See if any task is waiting for new data in the queue
  if (pont_event->OSEventWait != 0)
  {
    // Selects the highest priority task
    iPriority = SAScheduler(pont_event->OSEventWaitList);    

    // Remove the selected task from the queue wait list
    pont_event->OSEventWaitList = pont_event->OSEventWaitList & ~(PriorityMask[iPriority]);
    
    // Decreases the queue wait list counter
    pont_event->OSEventWait--;
    
    // Put the selected task into Ready List
    #if (VERBOSE == 1)
    TaskSelect = PriorityVector[iPriority];
    ContextTask[TaskSelect].State = READY;
    #endif
    
    OSReadyList = OSReadyList | (PriorityMask[iPriority]);
    
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
    
    return WRITE_BUFFER_OK;
  }
  else
  {
    // Exit Critical Section
    #if (NESTING_INT == 0)
    if (!iNesting)
    #endif
       OSExitCritical();
    
    return WRITE_BUFFER_OK;
  }
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


#endif


