/**
* \file BRTOS.c
* \brief BRTOS kernel functions
*
* Kernel functions, such as: scheduler, block tasks, unblock tasks, Delay, Change Context
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
*                                           Kernel functions
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
*   Revision: 1.60,         Revision: 1.61
*   Date:     30/11/2010,   Date:     02/12/2010
*
*   Authors:  Douglas França
*   Revision: 1.62
*   Date:     13/12/2010
*
*   Authors:  Carlos Henrique Barriquelo e Gustavo Weber Denardin
*   Revision: 1.63,      ,  Revision: 1.64       ,  Revision: 1.65        ,  Revision: 1.66         ,  Revision: 1.67
*   Date:     15/12/2010 ,  Date:     22/02/2011 ,  Date:     24/03/2011  ,  Date:     30/04/2011   ,  Date:     14/06/2011
*   Revision: 1.68       ,  Revision: 1.69       ,  Revision: 1.70        ,  Revision: 1.75			,  Revision: 1.76
*   Date:     02/09/2011 ,  Date:     05/11/2011 ,  Date:     06/06/2012  ,  Date:     24/08/2012	,  Date: 11/10/2012
*   Revision: 1.77       ,  Revision: 1.78, 		Revision: 1.79
*   Date:     12/01/2013 ,  Date:     06/03/2014, 	Date:     02/09/2014
*
*   Authors:  Gustavo Weber Denardin
*   Revision: 1.80		,	Revision: 1.90
*   Date:     11/11/2015, 	Date: 12/11/2015
*
*
*********************************************************************************************************/



#include "BRTOS.h"

#if (PROCESSOR == COLDFIRE_V1)
#pragma warn_implicitconv off
#endif

#if (PROCESSOR == ATMEGA) && (!DOXYGEN)
const CHAR8 version[] PROGMEM = BRTOS_VERSION;	///< Informs BRTOS version
PGM_P CONST BRTOSStringTable[] PROGMEM = 
{
    version
};
#else
#if (PROCESSOR == PIC18)
const rom CHAR8 *version=                            ///< Informs BRTOS version
{
  BRTOS_VERSION
};
#else
const CHAR8 *version=                            ///< Informs BRTOS version
{
  BRTOS_VERSION
};
#endif
#endif


#if ((PROCESSOR == ATMEGA) || (PROCESSOR == PIC18))
CHAR8 BufferText[TEXT_BUFFER_SIZE];
#endif
                     
INT8U PriorityVector[configMAX_TASK_INSTALL];   ///< Allocate task priorities
INT16U iStackAddress = 0;                       ///< Virtual stack counter - Informs the stack occupation in bytes


INT16U iQueueAddress = 0;                       ///< Queue heap control

#if (!BRTOS_DYNAMIC_TASKS_ENABLED)
#if (SP_SIZE == 32)
INT32U StackAddress = (INT32U)&STACK;           ///< Virtual stack pointer
#endif

#if (SP_SIZE == 16)
INT16U StackAddress = (INT16U)&STACK;           ///< Virtual stack pointer
#endif
#endif



// global variables
// Task Manager Variables
INT8U NumberOfInstalledTasks;                 ///< Number of Installed tasks at the moment
volatile INT8U currentTask;                            ///< Current task being executed
volatile INT8U SelectedTask;

#if (NUMBER_OF_PRIORITIES > 16)
  PriorityType OSReadyList = 0;
  PriorityType OSBlockedList = 0xFFFFFFFF;
#else
  #if (NUMBER_OF_PRIORITIES > 8)
    PriorityType OSReadyList = 0;
    PriorityType OSBlockedList = 0xFFFF;
  #else
    PriorityType OSReadyList = 0;
    PriorityType OSBlockedList = 0xFF;
  #endif
#endif

static   INT16U OSTickCounter;                    ///< Incremented each tick timer - Used in delay and timeout functions
volatile INT32U OSDuty=0;                         ///< Used to compute the CPU load
volatile INT32U OSDutyTmp=0;                      ///< Used to compute the CPU load

#ifdef TICK_TIMER_32BITS
volatile INT32U LastOSDuty = 0;                   ///< Last CPU load computed
#else
volatile INT16U LastOSDuty = 0;                   ///< Last CPU load computed
#endif

INT16U DutyCnt = 0;                               ///< Used to compute the CPU load
INT32U TaskAlloc = 0;                             ///< Used to search a empty task control block
INT8U  iNesting = 0;                              ///< Used to inform if the current code position is an interrupt handler code

ContextType *Tail;
ContextType *Head;

#if (DEBUG == 0)
volatile INT8U flag_load = TRUE;
#endif


#if (NUMBER_OF_PRIORITIES > 16)
  const PriorityType PriorityMask[configMAX_TASK_PRIORITY+1]=
  {
    0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x0100,0x0200,0x0400,0x0800,0x1000,0x2000,0x4000,0x8000,
    0x010000,0x020000,0x040000,0x080000,0x100000,0x200000,0x400000,0x800000,0x01000000,0x02000000,
    0x04000000,0x08000000,0x10000000,0x20000000,0x40000000,0x80000000
  };
#else
  #if (NUMBER_OF_PRIORITIES > 8)
    const PriorityType PriorityMask[configMAX_TASK_PRIORITY+1]=
    {
      0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,0x0100,0x0200,0x0400,0x0800,0x1000,0x2000,0x4000,0x8000
    };
  #else
    const PriorityType PriorityMask[configMAX_TASK_PRIORITY+1]=
    {
      0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80
    };  
  #endif
#endif


////////////////////////////////////////////////////////////
/////      Semaphore Control Block Declaration         /////
////////////////////////////////////////////////////////////
#if (BRTOS_SEM_EN == 1)
  /// Semahore Control Block
  BRTOS_Sem        BRTOS_Sem_Table[BRTOS_MAX_SEM];      // Table of EVENT control blocks
#endif


////////////////////////////////////////////////////////////
/////      Mutex Control Block Declaration             /////
////////////////////////////////////////////////////////////
#if (BRTOS_MUTEX_EN == 1)
  /// Mutex Control Block
  BRTOS_Mutex      BRTOS_Mutex_Table[BRTOS_MAX_MUTEX];    // Table of EVENT control blocks
#endif


////////////////////////////////////////////////////////////
/////      Mbox Control Block Declaration              /////
////////////////////////////////////////////////////////////
#if (BRTOS_MBOX_EN == 1)
  /// MailBox Control Block
  BRTOS_Mbox       BRTOS_Mbox_Table[BRTOS_MAX_MBOX];     // Table of EVENT control blocks
#endif


////////////////////////////////////////////////////////////
/////      Queue Control Block Declaration             /////
////////////////////////////////////////////////////////////
#if (BRTOS_QUEUE_EN == 1)
  /// Queue Control Block
  BRTOS_Queue      BRTOS_Queue_Table[BRTOS_MAX_QUEUE];    	// Table of EVENT control blocks
  OS_QUEUE	       BRTOS_OS_QUEUE_Table[BRTOS_MAX_QUEUE];	// Table of QUEUE control blocks
#endif


///// RAM definitions
#ifdef OS_CPU_TYPE
#if (!BRTOS_DYNAMIC_TASKS_ENABLED)
  #if (PROCESSOR == PIC18)
  #pragma udata stackram
  #endif
  OS_CPU_TYPE STACK[(HEAP_SIZE / sizeof(OS_CPU_TYPE))];  			       ///< Virtual Task stack
#endif

  #if (PROCESSOR == PIC18)
  #pragma udata queueram
  #endif
  OS_CPU_TYPE QUEUE_STACK[(QUEUE_HEAP_SIZE / sizeof(OS_CPU_TYPE))];  ///< Queue heap
#else
	#error("You must define the OS_CPU_TYPE !!!")
#endif

#if (PROCESSOR == PIC18)
#pragma udata ctxram
#endif
ContextType ContextTask[NUMBER_OF_TASKS + 1];          ///< Task context info
                                                       ///< ContextTask[0] not used
                                                       ///< Last ContexTask is the Idle Task


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Priority Preemptive Scheduler               /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/************************************************************//**
* \fn void OSSchedule(void)
* \brief Priority Preemptive Scheduler (Internal kernel function).
****************************************************************/

INT8U OSSchedule(void)
{
	INT8U TaskSelect = 0xFF;
	INT8U Priority   = 0;
	
  Priority = SAScheduler(OSReadyList & OSBlockedList);
  TaskSelect = PriorityVector[Priority];
  
	return TaskSelect;
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Get the current tick count                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
INT16U OSGetTickCount(void) 
{
  OS_SR_SAVE_VAR
  INT16U cnt;
  
  OSEnterCritical();
  cnt = OSTickCounter;
  OSExitCritical();
  return cnt;
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Get the current tick count                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
INT16U OSGetCount(void)
{
  return OSTickCounter;
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Update the tick count                       /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
void OSIncCounter(void)
{
	  OSTickCounter++;
	  if (OSTickCounter == TickCountOverFlow) OSTickCounter = 0;
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Task Delay Function in Tick Times           /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

// Atraso em passos de TickCount
INT8U OSDelayTask(INT16U time_wait)
{
  OS_SR_SAVE_VAR
  INT32U timeout;
  ContextType *Task = (ContextType*)&ContextTask[currentTask];
   
  if (iNesting > 0) {                                // See if caller is an interrupt
     return(IRQ_PEND_ERR);                           // Can't be blocked by interrupt
  }

  if (currentTask)
  {
    
    if (time_wait > 0)
    {
        OSEnterCritical();
        
        // BRTOS TRACE SUPPORT
        #if (OSTRACE == 1) 
            #if(OS_TRACE_BY_TASK == 1)
            Update_OSTrace(currentTask, DELAYTASK);
            #else
            Update_OSTrace(Task->Priority, DELAYTASK);
            #endif
        #endif    

        timeout = (INT32U)((INT32U)OSTickCounter + (INT32U)time_wait);
        
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
        
        #if (VERBOSE == 1)
        Task->State = SUSPENDED;
        Task->SuspendedType = DELAY;
        #endif
        
        OSReadyList = OSReadyList & ~(PriorityMask[Task->Priority]);
        
        // Change context
        // Return to task when occur delay overflow
        ChangeContext();
        
        OSExitCritical();
        
        return OK;
    }
    else
    {
        return NO_TASK_DELAY;
    }
  }
  else
  {
    return NOT_VALID_TASK;
  }
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////   Task Delay Function in miliseconds, seconds,   /////
/////   minutes and hours                              /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
// Miliseconds, seconds, minutes and hours delay
INT8U OSDelayTaskHMSM(INT8U hours, INT8U minutes, INT8U seconds, INT16U miliseconds)
{
  INT32U ticks=0;
  INT32U loops=0;
  
  if (minutes > 59)
    return INVALID_TIME;
  
  if (seconds > 59)
    return INVALID_TIME;
  
  if (miliseconds > 999)
    return INVALID_TIME;  
  
  ticks = (INT32U)hours   * 3600L * configTICK_RATE_HZ
        + (INT32U)minutes * 60L   * configTICK_RATE_HZ
        + (INT32U)seconds *         configTICK_RATE_HZ
        + ((INT32U)miliseconds    * configTICK_RATE_HZ)/1000L;
  
  // Task Delay limit = TickCounterOverflow
  if (ticks > 0)
  {
      loops = ticks / 60000L;
      ticks = ticks % 60000L;
      
      (void)DelayTask((INT16U)ticks);
      
      while(loops > 0)
      {
        (void)DelayTask(60000);
        loops--;
      }
      return OK;
  }
  else
  {
      return NO_TASK_DELAY;
  }
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      OS Tick Timer Function                      /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void OS_TICK_HANDLER(void)
{
  OS_SR_SAVE_VAR
  INT8U  iPrio = 0;  
  ContextType *Task = Head;  
   
  ////////////////////////////////////////////////////
  // Put task with delay overflow in the ready list //
  ////////////////////////////////////////////////////  
  while(Task != NULL)
  {      
      if (Task->TimeToWait == OSTickCounter)
      {

        iPrio = Task->Priority;
        
        #if (NESTING_INT == 1)
        OSEnterCritical();
        #endif        

        // Put the task into the ready list
        OSReadyList = OSReadyList | (PriorityMask[iPrio]);
        
        #if (VERBOSE == 1)
            Task->State = READY;        
        #endif
        
        Task->TimeToWait = EXIT_BY_TIMEOUT;
        
        #if (NESTING_INT == 1)
        OSExitCritical();
        #endif                  
          
        // Remove from delay list
        RemoveFromDelayList();

		#if ((PROCESSOR == ARM_Cortex_M0) || (PROCESSOR == ARM_Cortex_M3) || (PROCESSOR == ARM_Cortex_M4) || (PROCESSOR == ARM_Cortex_M4F))
		OS_INT_EXIT_EXT();
		#endif
      }
 
      Task = Task->Next;
  }

  //////////////////////////////////////////
  // System Load                          //
  //////////////////////////////////////////  
  #if (COMPUTES_CPU_LOAD == 1)
     if (DutyCnt >= 1000)
     {
       DutyCnt = 0;
       LastOSDuty = OSDuty;
       OSDuty = 0;
     }else
     {    
       if (!OSDutyTmp) OSDuty++;
       OSDutyTmp = 0;
       DutyCnt++;
     }
  #endif
  //////////////////////////////////////////
	
  #if (TIMER_HOOK_EN == 1)
    BRTOS_TimerHook();
  #endif
}





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      OS Init Task Scheduler Function             /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U BRTOSStart(void)
{
 #if (TASK_WITH_PARAMETERS == 1)
  if (InstallTask(&Idle, "Idle Task", IDLE_STACK_SIZE, 0, (void*)NULL, NULL) != OK)
 #else
  if (InstallTask(&Idle, "Idle Task", IDLE_STACK_SIZE, 0, NULL) != OK)
 #endif
  {
    return NO_MEMORY;
  };

  currentTask = OSSchedule();
  SPvalue = ContextTask[currentTask].StackPoint;
  BTOSStartFirstTask();
  return OK;
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      OS Function to Initialize RTOS Variables    /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void PreInstallTasks(void)
{
  INT8U i=0;
  OSTickCounter = 0;
  currentTask = 0;
  NumberOfInstalledTasks = 0;
  TaskAlloc = 0;
  iStackAddress = 0;
  
  for(i=0;i<configMAX_TASK_INSTALL;i++)
  {
    PriorityVector[i]=EMPTY_PRIO;
  }

  for(i=1;i<=NUMBER_OF_TASKS;i++)
  {
	  ContextTask[i].Priority = EMPTY_PRIO;
  }
    
  Tail = NULL;
  Head = NULL;
  
  #if (OSRTCEN == 1)
    OSRTCSetup();
  #endif
  
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Block Priority Function                     /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U OSBlockPriority(INT8U iPriority)
{
  OS_SR_SAVE_VAR
  INT8U BlockedTask = 0;
  
  if (iNesting > 0) {                                // See if caller is an interrupt
     return(IRQ_PEND_ERR);                           // Can't be blocked by interrupt
  }
      
  // Enter critical Section
  if (currentTask)  
    OSEnterCritical();


  // Detects the task priority
  BlockedTask = PriorityVector[iPriority];  
  // Block task with priority iPriority
  #if (VERBOSE == 1)
  ContextTask[BlockedTask].Blocked = TRUE;
  #endif
  
  OSBlockedList = OSBlockedList & ~(PriorityMask[iPriority]);
   
  
  if (currentTask == BlockedTask)
  {
     ChangeContext();
  }

  // Exit critical Section
  if (currentTask)
    OSExitCritical();
  
  return OK;
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      UnBlock Priority Function                   /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U OSUnBlockPriority(INT8U iPriority)
{
  OS_SR_SAVE_VAR
  #if (VERBOSE == 1)
  INT8U BlockedTask = 0;
  #endif
  
  
  // Enter Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSEnterCritical();
    
  // Detects the task priority
  #if (VERBOSE == 1)  
  BlockedTask = PriorityVector[iPriority];  
  ContextTask[BlockedTask].Blocked = FALSE;
  #endif
  
  OSBlockedList = OSBlockedList | (PriorityMask[iPriority]);
  
  // check if we have unblocked a higher priority task  
  if (currentTask)
  {
    if (!iNesting)
    {
       ChangeContext();
    }
  }
  
  // Exit Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSExitCritical();

  return OK;
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Block Task Function                         /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U OSBlockTask(BRTOS_TH TaskHandle)
{
  OS_SR_SAVE_VAR
  INT8U iPriority = 0;
  
  if (iNesting > 0) {                                // See if caller is an interrupt
     return(IRQ_PEND_ERR);                           // Can't be blocked by interrupt
  }
    
  // Enter critical Section
  if (currentTask)
    OSEnterCritical();

  // Checks whether the task is uninstalling itself
  if (!TaskHandle){
	  // If so, verify if the currentTask is valid
	  if (currentTask){
		  //If true, currentTask is the task being uninstall
		  TaskHandle = currentTask;
	  }else{
		  // If not, not valid task
		  // Exit Critical Section
		  OSExitCritical();

		  return NOT_VALID_TASK;
	  }
  }

  // Determina a prioridade da função
  #if (VERBOSE == 1)
  ContextTask[TaskHandle].Blocked = TRUE;
  #endif
  iPriority = ContextTask[TaskHandle].Priority;
  
  OSBlockedList = OSBlockedList & ~(PriorityMask[iPriority]);
  
  if (currentTask == TaskHandle)
  {
     ChangeContext();     
  }
  
  // Exit critical Section
  if (currentTask)
    OSExitCritical();  

  return OK;
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      UnBlock Task Function                       /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U OSUnBlockTask(BRTOS_TH TaskHandle)
{
  OS_SR_SAVE_VAR
  INT8U iPriority = 0;
  
  // Enter Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSEnterCritical();

  #if (VERBOSE == 1)
  ContextTask[TaskHandle].Blocked = FALSE;
  #endif
  
  // Determina a prioridade da função  
  iPriority = ContextTask[TaskHandle].Priority;

  OSBlockedList = OSBlockedList | (PriorityMask[iPriority]);
  
  // check if we have unblocked a higher priority task  
  if (currentTask)
  {
    if (!iNesting)
    {
       ChangeContext();
    }
  }
  
  // Exit Critical Section
  #if (NESTING_INT == 0)
  if (!iNesting)
  #endif
     OSExitCritical();

  return OK;  
  
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Block Multiple Task Function                /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U OSBlockMultipleTask(INT8U TaskStart, INT8U TaskNumber)
{
  OS_SR_SAVE_VAR
  INT8U iTask = 0;
  INT8U TaskFinish = 0;
  INT8U iPriority = 0;  
  
  if (iNesting > 0) {                                // See if caller is an interrupt
     return(IRQ_PEND_ERR);                           // Can't be blocked by interrupt
  }
    
  // Enter critical Section
  if (currentTask)
    OSEnterCritical();
  
  TaskFinish = (INT8U)(TaskStart + TaskNumber);
  
  for (iTask = TaskStart; iTask <TaskFinish; iTask++)
  {
    if (iTask != currentTask)
    {      
      #if (VERBOSE == 1)
      ContextTask[iTask].Blocked = TRUE;
      #endif
      // Determina a prioridade da função
      iPriority = ContextTask[iTask].Priority;   
      
      OSBlockedList = OSBlockedList & ~(PriorityMask[iPriority]);
    }
  }
  
  // Exit critical Section
  if (currentTask)
    OSExitCritical();

  return OK;
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      unBlock Multiple Task Function              /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

INT8U OSUnBlockMultipleTask(INT8U TaskStart, INT8U TaskNumber)
{
  OS_SR_SAVE_VAR
  INT8U iTask = 0;
  INT8U TaskFinish = 0;
  INT8U iPriority = 0;    
  
  if (iNesting > 0) {                                // See if caller is an interrupt
     return(IRQ_PEND_ERR);                           // Can't be blocked by interrupt
  }
    
  // Enter critical Section
  if (currentTask)
    OSEnterCritical();
  
  TaskFinish = (INT8U)(TaskStart + TaskNumber);
  
  for (iTask = TaskStart; iTask <TaskFinish; iTask++)
  {
    // Determina a prioridade da função
    if (iTask != currentTask)
    {
      iPriority = ContextTask[iTask].Priority;
      
      #if (VERBOSE == 1)
      ContextTask[iTask].Blocked = FALSE;
      #endif
      
      OSBlockedList = OSBlockedList | (PriorityMask[iPriority]);
    }
  }
  
  // check if we have unblocked a higher priority task  
  if (currentTask)
  {
    if (!iNesting)
    {
       ChangeContext();
    }
	// Exit critical Section
	OSExitCritical();
  } 

  return OK;
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////    OS Idle Task                                  /////
/////                                                  /////
/////    You must put the processor in standby mode    /////
/////                                                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#if (TASK_WITH_PARAMETERS == 1) 
  void Idle(void *parameters)
#else
  void Idle(void)
#endif
{
  /* task setup */
  #if (TASK_WITH_PARAMETERS == 1)  
  (void)parameters;
  #endif
  
  /* task main loop */
  for (;;)
  {
     #if (IDLE_HOOK_EN == 1)
        IdleHook();
     #endif
     
     #if (COMPUTES_CPU_LOAD == 1)
        OSDutyTmp = 1;
     #endif            
     
     OS_Wait;
  }
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////  Tasks Installation Function                 		/////
/////                                                  /////
/////  Parameters:                                     /////
/////  Function pointer, task name, task priority,     /////
/////  parameters and task handler					   /////
/////                                                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#if (!BRTOS_DYNAMIC_TASKS_ENABLED)
#if (TASK_WITH_PARAMETERS == 1)
  INT8U OSInstallTask(void(*FctPtr)(void *),const CHAR8 *TaskName, INT16U USER_STACKED_BYTES,INT8U iPriority, void *parameters, OS_CPU_TYPE *TaskHandle)
#else
  INT8U OSInstallTask(void(*FctPtr)(void),const CHAR8 *TaskName, INT16U USER_STACKED_BYTES,INT8U iPriority, OS_CPU_TYPE *TaskHandle)
#endif
{
  OS_SR_SAVE_VAR
  INT8U i = 0; 
  INT8U TaskNumber = 0;
  ContextType * Task;    
  
   if (currentTask)
    // Enter Critical Section
    OSEnterCritical();
    
   // Fix the stack size to the cpu type
   USER_STACKED_BYTES = USER_STACKED_BYTES - (USER_STACKED_BYTES % sizeof(OS_CPU_TYPE));

   if (USER_STACKED_BYTES < NUMBER_MIN_OF_STACKED_BYTES)
   {
       if (currentTask)
        // Exit Critical Section
        OSExitCritical();
       return STACK_SIZE_TOO_SMALL;
   }
   
   if ((iStackAddress + (USER_STACKED_BYTES / sizeof(OS_CPU_TYPE))) > (HEAP_SIZE / sizeof(OS_CPU_TYPE)))
   {
       if (currentTask)
        // Exit Critical Section
        OSExitCritical();
       return NO_MEMORY;
   }

   if (iPriority)
   {
     if (iPriority > configMAX_TASK_PRIORITY)
     {
        if (currentTask)
         // Exit Critical Section
         OSExitCritical();        
        return END_OF_AVAILABLE_PRIORITIES;
     }
     
     if (PriorityVector[iPriority] != EMPTY_PRIO)
     {
        if (currentTask)
         // Exit Critical Section
         OSExitCritical();        
        return BUSY_PRIORITY;
     }
   }
   else
   {
	   // Verify if trying to install a user task at priority 0
	   if (FctPtr != Idle){
		   if (currentTask)
	       // Exit Critical Section
	       OSExitCritical();
		   return CANNOT_ASSIGN_IDLE_TASK_PRIO;
	   }
   }
      
   // Number Task Discovery
   for(i=0;i<NUMBER_OF_TASKS;i++)
   {
      INT32U teste = 1;
      teste = teste<<i;
    
      if (!(teste & TaskAlloc))
      {
         TaskNumber = i+1;
         TaskAlloc = TaskAlloc | teste;
         break;
      }
   }   
   
   // Verifica se encontrou lugar para o contexto da tarefa
   if (TaskNumber == 0) 
   {    
      if (currentTask)
      {        
        // Exit Critical Section
        OSExitCritical();   
      }
      return END_OF_AVAILABLE_TCB;
   }
   
    NumberOfInstalledTasks++;
     
   // Copy task handle id
   if (TaskHandle != NULL) 
   {
      *TaskHandle = (OS_CPU_TYPE)TaskNumber;
   }
   
   Task = (ContextType*)&ContextTask[TaskNumber];      
   Task->TaskName = TaskName;

   // Posiciona o inicio do stack da tarefa
   // no inicio da disponibilidade de RAM do HEAP
	#if STACK_GROWTH == 1
	 Task->StackPoint = StackAddress + NUMBER_MIN_OF_STACKED_BYTES;
	#else
	 Task->StackPoint = StackAddress + (USER_STACKED_BYTES - NUMBER_MIN_OF_STACKED_BYTES);
  #endif
                                                                      
  // Virtual Stack Init
	#if STACK_GROWTH == 1
	Task->StackInit = StackAddress;
	#else
	Task->StackInit = StackAddress + USER_STACKED_BYTES;
	#endif
    

   // Determina a prioridade da função
   Task->Priority = iPriority;

   // Determina a tarefa que irá ocupar esta prioridade
   PriorityVector[iPriority] = TaskNumber;
   // set the function entry address in the context
   
   // Fill the virtual task stack
   #if (TASK_WITH_PARAMETERS == 1)
      CreateVirtualStack(FctPtr, USER_STACKED_BYTES, parameters);
   #else
      CreateVirtualStack(FctPtr, USER_STACKED_BYTES);   
   #endif
   
   // Incrementa o contador de bytes do stack virtual (HEAP)
   iStackAddress = iStackAddress + (USER_STACKED_BYTES / sizeof(OS_CPU_TYPE));
   
   // Posiciona o endereço de stack virtual p/ a próxima tarefa instalada
   StackAddress = StackAddress + USER_STACKED_BYTES;
   
   Task->TimeToWait = NO_TIMEOUT;
   Task->Next     =  NULL;
   Task->Previous =  NULL;
   
   #if (VERBOSE == 1)
   Task->Blocked = FALSE;
   Task->State = READY;
   #endif   
   
   OSReadyList = OSReadyList | (PriorityMask[iPriority]);   
   
   if (currentTask)
    // Exit Critical Section
    OSExitCritical();   
   
   return OK;
}
#else
#if (TASK_WITH_PARAMETERS == 1)
  INT8U OSInstallTask(void(*FctPtr)(void *),const CHAR8 *TaskName, INT16U USER_STACKED_BYTES,INT8U iPriority, void *parameters, OS_CPU_TYPE *TaskHandle)
#else
  INT8U OSInstallTask(void(*FctPtr)(void),const CHAR8 *TaskName, INT16U USER_STACKED_BYTES,INT8U iPriority, OS_CPU_TYPE *TaskHandle)
#endif
{
  OS_SR_SAVE_VAR
  INT8U i = 0;
  INT8U TaskNumber = 0;
  ContextType *Task;
  void *Stack = NULL;

   if (currentTask)
    // Enter Critical Section
    OSEnterCritical();

   // Fix the stack size to the cpu type
   USER_STACKED_BYTES = USER_STACKED_BYTES - (USER_STACKED_BYTES % sizeof(OS_CPU_TYPE));

   if (USER_STACKED_BYTES < NUMBER_MIN_OF_STACKED_BYTES)
   {
       if (currentTask)
        // Exit Critical Section
        OSExitCritical();
       return STACK_SIZE_TOO_SMALL;
   }

   if (iPriority)
   {
     if (iPriority > configMAX_TASK_PRIORITY)
     {
        if (currentTask)
         // Exit Critical Section
         OSExitCritical();
        return END_OF_AVAILABLE_PRIORITIES;
     }

     if (PriorityVector[iPriority] != EMPTY_PRIO)
     {
        if (currentTask)
         // Exit Critical Section
         OSExitCritical();
        return BUSY_PRIORITY;
     }
   }
   else
   {
	   // Verify if trying to install a user task at priority 0
	   if (FctPtr != Idle){
		   if (currentTask)
	       // Exit Critical Section
	       OSExitCritical();
		   return CANNOT_ASSIGN_IDLE_TASK_PRIO;
	   }
   }

   // Allocate the task virtual stack
   Stack = BRTOS_ALLOC(USER_STACKED_BYTES);

   if (Stack == NULL)
   {
       if (currentTask)
        // Exit Critical Section
        OSExitCritical();
       return NO_MEMORY;
   }

   // Number Task Discovery
   for(i=0;i<NUMBER_OF_TASKS;i++)
   {
      INT32U teste = 1;
      teste = teste<<i;

      if (!(teste & TaskAlloc))
      {
         TaskNumber = i+1;
         TaskAlloc = TaskAlloc | teste;
         break;
      }
   }

   // Verify if there is space for the task in the TCB Table
   if (TaskNumber == 0)
   {
	  BRTOS_DEALLOC(Stack);
	  if (currentTask)
      {
        // Exit Critical Section
        OSExitCritical();
      }
      return END_OF_AVAILABLE_TCB;
   }

   NumberOfInstalledTasks++;

   // Copy task handle id
   if (TaskHandle != NULL)
   {
      *TaskHandle = (OS_CPU_TYPE)TaskNumber;
   }

   Task = (ContextType*)&ContextTask[TaskNumber];
   Task->TaskName = TaskName;

   // Posiciona o inicio do stack da tarefa
   Task->StackInit = (unsigned int)Stack;

   // Determina a prioridade da função
   Task->Priority = iPriority;

   // Determina a tarefa que irá ocupar esta prioridade
   PriorityVector[iPriority] = TaskNumber;
   // set the function entry address in the context

   // Fill the virtual task stack
   #if (TASK_WITH_PARAMETERS == 1)
   Task->StackPoint = CreateDVirtualStack(FctPtr, (OS_CPU_TYPE)Stack + USER_STACKED_BYTES, parameters);
   #else
   Task->StackPoint = CreateDVirtualStack(FctPtr, (OS_CPU_TYPE)Stack + USER_STACKED_BYTES);
   #endif

   Task->StackSize = USER_STACKED_BYTES;
   Task->TimeToWait = NO_TIMEOUT;
   Task->Next     =  NULL;
   Task->Previous =  NULL;

   #if (VERBOSE == 1)
   Task->Blocked = FALSE;
   Task->State = READY;
   #endif

   OSReadyList = OSReadyList | (PriorityMask[iPriority]);

   if (currentTask)
    // Exit Critical Section
    OSExitCritical();

   return OK;
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////  Tasks Uninstall Function             		   /////
/////                                                  /////
/////  Parameters:                                     /////
/////  Task handler									   /////
/////                                                  /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
INT8U OSUninstallTask(BRTOS_TH TaskHandle){
	  OS_SR_SAVE_VAR
	  ContextType *Task;

	  if (currentTask)
		  // Enter Critical Section
		  OSEnterCritical();

	  // Checks whether the task is uninstalling itself
	  if (!TaskHandle){
		  // If so, verify if the currentTask is valid
		  if (currentTask){
			  //If true, currentTask is the task being uninstall
			  TaskHandle = currentTask;
		  }else{
			  // If not, not valid task
			  // Exit Critical Section
			  OSExitCritical();

			  return NOT_VALID_TASK;
		  }
	  }

	  Task = (ContextType*)&ContextTask[TaskHandle];

	  // Verify if is trying to uninstall the idle task
	  if (!(Task->Priority)){
		  if (currentTask)
			  // Exit Critical Section
			  OSExitCritical();

		  return CANNOT_UNINSTALL_IDLE_TASK;
	  }

	  // Checks whether the task handler is valid
	  if (Task != NULL){
		  // Verify if the task is waiting for an event
		  if ((OSReadyList & PriorityMask[Task->Priority]) == PriorityMask[Task->Priority]){
			  // If not, it is possible to proceed with the uninstall
			  TaskAlloc = TaskAlloc & ~(1 << (TaskHandle-1));
			  OSReadyList = OSReadyList & ~(PriorityMask[Task->Priority]);
			  PriorityVector[Task->Priority] = EMPTY_PRIO;

			  BRTOS_DEALLOC((void*)Task->StackInit);

			  Task->StackInit = 0;
			  Task->StackPoint = 0;
			  Task->StackSize = 0;
			  Task->Priority = EMPTY_PRIO;
			  Task->TimeToWait = NO_TIMEOUT;
			  Task->Next     =  NULL;
			  Task->Previous =  NULL;

			  NumberOfInstalledTasks--;

			  // If uninstalled task if the current task, change context
			  /* OBS.: In the switch context, the context of the uninstalled task will be
			  saved at the deallocated memory. That is not a problem, because the memory will be
			  reused by another task and the current task will never be called again by the system */
			  if (TaskHandle == currentTask) ChangeContext();

			  if (currentTask)
				  // Exit Critical Section
				  OSExitCritical();

			  return OK;
		  }else{
			  if (currentTask)
				  // Exit Critical Section
				  OSExitCritical();

			  return TASK_WAITING_EVENT;
		  }

	  }

	  if (currentTask)
		  // Exit Critical Section
		  OSExitCritical();

	  return NOT_VALID_TASK;
}
#endif
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





void BRTOSInit(void)
{  
  ////////////////////////////////////////////////////////////  
  /////      Initialize Event Control Blocks             /////
  ////////////////////////////////////////////////////////////
  initEvents();
  
  ////////////////////////////////////////////////////////////  
  /////          Initialize global variables             /////
  ////////////////////////////////////////////////////////////  
  PreInstallTasks();  
  
  ////////////////////////////////////////////////////////////  
  /////            Initialize Tick Timer                 /////
  ////////////////////////////////////////////////////////////  
  TickTimerSetup(); 
}





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Initialize Block List Control               /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void initEvents(void)
{
  INT8U i=0;
  
  #if (BRTOS_SEM_EN == 1)
    for(i=0;i<BRTOS_MAX_SEM;i++)
      BRTOS_Sem_Table[i].OSEventAllocated = 0;
  #endif
  
  #if (BRTOS_MUTEX_EN == 1)
    for(i=0;i<BRTOS_MAX_MUTEX;i++)
      BRTOS_Mutex_Table[i].OSEventAllocated = 0;
  #endif
    
  #if (BRTOS_MBOX_EN == 1)
    for(i=0;i<BRTOS_MAX_MBOX;i++)
      BRTOS_Mbox_Table[i].OSEventAllocated = 0;    
  #endif
  
  #if (BRTOS_QUEUE_EN == 1)
    for(i=0;i<BRTOS_MAX_QUEUE;i++)
      BRTOS_Queue_Table[i].OSEventAllocated = 0;    
  #endif
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////    Sucessive Aproximation Scheduler Algorithm    /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#if (OPTIMIZED_SCHEDULER == 1)

INT8U SAScheduler(PriorityType READY_LIST_VAR)
{
  Optimezed_Scheduler();
}

#else

INT8U SAScheduler(PriorityType ReadyList)
{
  INT8U prio = 0;
  
  #if (NUMBER_OF_PRIORITIES > 16)
  
  if (ReadyList > 0xFFFF)
  {
    if (ReadyList > 0xFFFFFF)
    {
      if (ReadyList > 0xFFFFFFF)
      {        
        if (ReadyList > 0x3FFFFFFF)
        {
          if (ReadyList > 0x7FFFFFFF)
          {
            prio = 31;
          }
          else
          {
            prio = 30;
          }
        }
        else
        {
          if (ReadyList > 0x1FFFFFFF)
          {
            prio = 29; 
          }
          else
          {
            prio = 28;
          }
        }
      }
      else
      {
        if (ReadyList > 0x3FFFFFF)
        {
          if (ReadyList > 0x7FFFFFF)
          {
            prio = 27;
          }
          else
          {
            prio = 26;
          }
        }
        else
        {
          if (ReadyList > 0x1FFFFFF)
          {
            prio = 25;
          }
          else
          {
            prio = 24;
          }
        }
      }    
    }
    else
    {
      if (ReadyList > 0xFFFFF)
      {
        if (ReadyList > 0x3FFFFF)
        {
          if (ReadyList > 0x7FFFFF)
          {
            prio = 23;
          }
          else
          {
            prio = 22;
          }
        }
        else
        {
          if (ReadyList > 0x1FFFFF)
          {
            prio = 21; 
          }
          else
          {
            prio = 20;
          }
        }
      }
      else
      {
        if (ReadyList > 0x3FFFF)
        {
          if (ReadyList > 0x7FFFF)
          {
            prio = 19;
          }
          else
          {
            prio = 18;
          }
        }
        else
        {
          if (ReadyList > 0x1FFFF)
          {
            prio = 17;
          }
          else
          {
            prio = 16;
          }
        }
      }
    }  
  }
  else
  {
  #endif
    #if (NUMBER_OF_PRIORITIES > 8)
    if (ReadyList > 0xFF)
    {
      if (ReadyList > 0xFFF)
      {        
        if (ReadyList > 0x3FFF)
        {
          if (ReadyList > 0x7FFF)
          {
            prio = 15;
          }
          else
          {
            prio = 14;
          }
        }
        else
        {
          if (ReadyList > 0x1FFF)
          {
            prio = 13; 
          }
          else
          {
            prio = 12;
          }
        }
      }
      else
      {
        if (ReadyList > 0x3FF)
        {
          if (ReadyList > 0x7FF)
          {
            prio = 11;
          }
          else
          {
            prio = 10;
          }
        }
        else
        {
          if (ReadyList > 0x1FF)
          {
            prio = 9;
          }
          else
          {
            prio = 8;
          }
        }
      }    
    }
    else
    {
    #endif
      if (ReadyList > 0x0F)
      {
        if (ReadyList > 0x3F)
        {
          if (ReadyList > 0x7F)
          {
            prio = 7;
          }
          else
          {
            prio = 6;
          }
        }
        else
        {
          if (ReadyList > 0x1F)
          {
            prio = 5; 
          }
          else
          {
            prio = 4;
          }
        }
      }
      else
      {
        if (ReadyList > 0x03)
        {
          if (ReadyList > 0x07)
          {
            prio = 3;
          }
          else
          {
            prio = 2;
          }
        }
        else
        {
          if (ReadyList > 0x1)
          {
            prio = 1;
          }
          else
          {
            prio = 0;
          }
        }
      }
    #if (NUMBER_OF_PRIORITIES > 8)
    }
    #endif
  #if (NUMBER_OF_PRIORITIES > 16)
  }
  #endif
  return prio;
}

#endif
