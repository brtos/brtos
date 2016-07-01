
/**
* \file BRTOS.h
* \brief BRTOS kernel main defines, functions prototypes and structs declaration.
*
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
*                                          BRTOS Kernel Header
*
*
*   Author:   Gustavo W. Denardin   ,   Authors:  Carlos H. Barriquelo e Gustavo W. Denardin
*   Revision: 1.0                   ,   Revision: 1.63 
*   Date:     20/03/2009            ,   Date:     15/12/2010
*
*   Authors:  Carlos H. Barriquelo  ,   Authors:  Gustavo W. Denardin
*   Revision: 1.64                  ,   Revision: 1.70
*   Date:     22/02/2011            ,   Date:     06/06/2012
*
*   Author:   Gustavo W. Denardin	,	Authors:  Gustavo W. Denardin
*   Revision: 1.75					,   Revision: 1.76        ,   Revision: 1.78
*   Date:     24/08/2012	  		,	Date:     11/10/2012  ,	  Date:     06/03/2014
*
*   Authors:  Gustavo Weber Denardin
*   Revision: 1.80					,	Revision: 1.90
*   Date:     11/11/2015			, 	Date: 12/11/2015
*
*   Author:  Carlos H. Barriquello
*   Revision: 1.91					
*   Date:     04/12/2015			
*
*   Author:  Gustavo Weber Denardin
*   Revision: 2.00
*   Date:     18/05/2016
*
*********************************************************************************************************/

#ifndef OS_BRTOS_H
#define OS_BRTOS_H

#include "OS_types.h"
#include "HAL.h"
#include "OSTime.h"
#include "BRTOSConfig.h"


///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////
/////                                                     /////
/////                 OS BRTOS Defines                    /////
/////                                                     /////
/////  !Do not change unless you know what you're doing!  /////
/////                                                     /////
///////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////


// Brtos version
#define BRTOS_VERSION   "BRTOS Ver. 2.00"

/// False and True defines
#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE  1
#endif

#ifndef NULL
#define NULL  (void*)0
#endif

#ifndef READY_LIST_VAR
#define READY_LIST_VAR
#endif

#define BRTOS_BIG_ENDIAN              (0)
#define BRTOS_LITTLE_ENDIAN           (1)

#ifndef BRTOS_TH
#define BRTOS_TH                      OS_CPU_TYPE
#endif

#if (!defined(ostick_t) && !defined(osdtick_t))
#define ostick_t					  uint16_t
#define osdtick_t					  uint32_t
#endif
#define sizeof_ostick_t			sizeof(ostick_t)


/// Task States
#define READY                        (uint8_t)0     ///< Task is ready to be executed - waiting for the scheduler authorization
#define SUSPENDED                    (uint8_t)1     ///< Task is suspended
#define BLOCKED                      (uint8_t)2     ///< Task is blocked - Will not run until be released
#define MUTEX_PRIO                   (uint8_t)0xFE
#define EMPTY_PRIO                   (uint8_t)0xFF


/// Timer defines
#if  (ostick_t == uint64_t)
#define MAX_TIMER					0xffffffffffffffff
#elif (ostick_t == uint32_t)
#define MAX_TIMER					0xffffffff
#else
#define MAX_TIMER					0xffff
#endif
#define NO_TIMEOUT                  (ostick_t)(MAX_TIMER - 1)
#define EXIT_BY_TIMEOUT             (ostick_t)(MAX_TIMER - 2)
#define TICK_COUNT_OVERFLOW         (ostick_t)(MAX_TIMER - 3)       ///< Determines the tick timer overflow
#define TickCountOverFlow           TICK_COUNT_OVERFLOW ///< Compatibility with BRTOS less than or equal to 1.7

/// Error codes
#define OK                           (uint8_t)0     ///< OK define
#define NO_MEMORY                    (uint8_t)1     ///< Error - Lack of memory to allocate a task
#define STACK_SIZE_TOO_SMALL         (uint8_t)2     ///< Error - Stack size too small to allocate a task
#define END_OF_AVAILABLE_PRIORITIES  (uint8_t)3     ///< Error - There are no more priorities available
#define BUSY_PRIORITY                (uint8_t)4     ///< Error - Priority is being used by another task
#define INVALID_TIME                 (uint8_t)5     ///< Error - Informed time is out of the limits
#define TIMEOUT                      (uint8_t)6     ///< Error - Timeout
#define CANNOT_ASSIGN_IDLE_TASK_PRIO (uint8_t)7     ///< Error - A task can not be assigned into the idle task slot
#define NOT_VALID_TASK               (uint8_t)8     ///< There current task number is not valid for this function
#define NO_TASK_DELAY                (uint8_t)9     ///< Error - No valid time to wait
#define END_OF_AVAILABLE_TCB         (uint8_t)10    ///< Error - There are no more task control blocks (Context task)
#define EXIT_BY_NO_ENTRY_AVAILABLE	 (uint8_t)11	  ///< Error - There are no data into queues and mailboxes or semaphore value is zero with no timeout option
#define TASK_WAITING_EVENT			 (uint8_t)12	  ///< Error - The task being uninstalled is waiting for an event (uninstall aborted)
#define CANNOT_UNINSTALL_IDLE_TASK   (uint8_t)13    ///< Error - It is not be allow to uninstall the idle task
#define EXIT_BY_NO_RESOURCE_AVAILABLE (uint8_t)14	  ///< Error - The resource is not available with no timeout option

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////



// Return defines to events control blocks
#define ALLOC_EVENT_OK          (uint8_t)0      ///< Event allocated with success
#define NO_AVAILABLE_EVENT      (uint8_t)1      ///< No event control blocks available
#define NO_AVAILABLE_MEMORY     (uint8_t)2      ///< Error - Lack of memory to allocate an event
#define INVALID_PARAMETERS      (uint8_t)3      ///< There is at least one invalid parameter
#define IRQ_PEND_ERR            (uint8_t)4      ///< Function can not be called inside an interrupt
#define ERR_SEM_OVF             (uint8_t)5      ///< Semaphore counter overflow
#define ERR_MUTEX_OVF           (uint8_t)6      ///< Mutex counter overflow
#define ERR_EVENT_NO_CREATED    (uint8_t)7      ///< There are no task waiting for the event
#define NULL_EVENT_POINTER      (uint8_t)8      ///< The passed event pointer is NULL
#define ERR_EVENT_OWNER         (uint8_t)9      ///< Function caller is not the owner of the event control block. Used to mutex implementation
#define DELETE_EVENT_OK         (uint8_t)10     ///< Event deleted with success
#define AVAILABLE_RESOURCE      (uint8_t)11     ///< The resource is available
#define BUSY_RESOURCE           (uint8_t)12     ///< The resource is busy
#define AVAILABLE_MESSAGE       (uint8_t)13     ///< There is a message
#define NO_MESSAGE              (uint8_t)14     ///< There is no message

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Queue Defines                               /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

// Enable Queue Structures
#define READ_BUFFER_OK       0            ///< New data successfully read
#define WRITE_BUFFER_OK      0            ///< New data successfully written
#define BUFFER_UNDERRUN      1            ///< Queue overflow
#define CLEAN_BUFFER_OK      2            ///< Queue successfully cleaned
#define NO_ENTRY_AVAILABLE   3            ///< Queue is empty

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////



/// Suspended Types
#define DELAY     0                               ///< Task suspended by delay
#define SEMAPHORE 1                               ///< Task suspended by semaphore
#define MAILBOX   2                               ///< Task suspended by mailbox
#define QUEUE     3                               ///< Task suspended by queue
#define MUTEX     4                               ///< Task suspended by mutex



/// Task Defines

#if (NUMBER_OF_PRIORITIES > 16)
  #define configMAX_TASK_INSTALL  32                 ///< Defines the maximum number of tasks that can be installed
  #define configMAX_TASK_PRIORITY 31  
  typedef uint32_t PriorityType;
#else
  #if (NUMBER_OF_PRIORITIES > 8)
    #define configMAX_TASK_INSTALL  16                 ///< Defines the maximum number of tasks that can be installed
    #define configMAX_TASK_PRIORITY 15
    typedef uint16_t PriorityType;
  #else
    #define configMAX_TASK_INSTALL  8                 ///< Defines the maximum number of tasks that can be installed
    #define configMAX_TASK_PRIORITY 7  
    typedef uint8_t PriorityType;
  #endif
#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Context Tasks Structure Prototypes          /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/**
* \struct ContextType
* Context Task Structure
* Used by the task control block
*/
struct Context
{
   const CHAR8 * TaskName;  ///< Task name
  #if SP_SIZE == 32
   uint32_t StackPoint;       ///< Current position of virtual stack pointer
   uint32_t StackInit;        ///< Virtual stack pointer init
  #else
   uint16_t StackPoint;       ///< Current position of virtual stack pointer
   uint16_t StackInit;        ///< Virtual stack pointer init  
  #endif
#if (BRTOS_DYNAMIC_TASKS_ENABLED == 1)
 uint16_t StackSize;
#endif
#if (COMPUTES_TASK_LOAD == 1)
   uint32_t Runtime;
#endif
   ostick_t TimeToWait;     ///< Time to wait - could be used by delay or timeout
  #if (VERBOSE == 1)
   uint8_t  State;            ///< Task states
   uint8_t  Blocked;          ///< Task blocked state
   uint8_t  SuspendedType;    ///< Task suspended type
  #endif
   uint8_t  Priority;         ///< Task priority
   struct Context *Next;
   struct Context *Previous;
};

typedef struct Context ContextType;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////    Semaphore Control Block Structure             /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/**
* \struct BRTOS_Sem
* Semaphore Control Block Structure
*/
typedef struct {
  uint8_t        OSEventAllocated;              ///< Indicate if the event is allocated or not
  uint8_t        OSEventCount;                  ///< Semaphore Count - This value is increased with a post and decremented with a pend
  uint8_t        OSEventWait;                   ///< Counter of waiting Tasks
#if (BRTOS_BINARY_SEM_EN == 1)
  uint8_t		   Binary;						  ///< Defines if semaphore is binary or counting
#endif
  PriorityType OSEventWaitList;               ///< Task wait list for event to occur
} BRTOS_Sem;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////    Mutex Control Block Structure                 /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/**
* \struct BRTOS_Mutex
* Mutex Control Block Structure
*/
typedef struct {
  uint8_t        OSEventAllocated;              ///< Indicate if the event is allocated or not
  uint8_t        OSEventState;                  ///< Mutex state - Defines if the resource is available or not
  uint8_t        OSEventOwner;                  ///< Defines mutex owner
  uint8_t        OSMaxPriority;                 ///< Defines max priority accessing resource
  uint8_t        OSOriginalPriority;            ///< Save original priority of Mutex owner task - used to the priority ceiling implementation
  uint8_t        OSEventWait;                   ///< Counter of waiting Tasks
  PriorityType OSEventWaitList;               ///< Task wait list for event to occur
} BRTOS_Mutex;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////    MailBox Control Block Structure               /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/**
* \struct BRTOS_Mbox
* MailBox Control Block Structure
*/
typedef struct {
  uint8_t        OSEventAllocated;              ///< Indicate if the event is allocated or not
  uint8_t        OSEventWait;                   ///< Counter of waiting Tasks
  uint8_t        OSEventState;                  ///< Mailbox state - Defines if the message is available or not
  PriorityType OSEventWaitList;               ///< Task wait list for event to occur
  void         *OSEventPointer;               ///< Pointer to the message structure / type
} BRTOS_Mbox;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////    Queue Control Block Structure                 /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/**
* \struct BRTOS_Queue
* Queue Control Block Structure
*/
typedef struct {
  uint8_t        OSEventAllocated;              ///< Indicate if the event is allocated or not
  uint8_t        OSEventCount;                  ///< Queue Event Count - This value is increased with a post and decremented with a pend
  uint8_t        OSEventWait;                   ///< Counter of waiting Tasks
  void         *OSEventPointer;               ///< Pointer to queue structure
  PriorityType OSEventWaitList;               ///< Task wait list for event to occur
} BRTOS_Queue;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Queue Structure                             /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/**
* \struct OS_QUEUE
* Queue Control Block Structure
*/
typedef struct
{
  uint8_t        *OSQStart;               ///< Pointer to the queue start
  uint8_t        *OSQEnd;                 ///< Pointer to the queue end
  uint8_t        *OSQIn;                  ///< Pointer to the next queue entry
  uint8_t        *OSQOut;                 ///< Pointer to the next data in the queue output
  uint16_t       OSQSize;                 ///< Size of the queue - Defined in the create queue function
  uint16_t       OSQEntries;              ///< Size of data inside the queue
} OS_QUEUE;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Dynamic Queue Structure                     /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/**
* \struct OS_QUEUE
* Dynamic Queue Control Block Structure
*/
typedef struct
{
  uint8_t        *OSQStart;               ///< Pointer to the queue start
  uint8_t        *OSQEnd;                 ///< Pointer to the queue end
  uint8_t        *OSQIn;                  ///< Pointer to the next queue entry
  uint8_t        *OSQOut;                 ///< Pointer to the next data in the queue output
  uint16_t       OSQTSize;                ///< Size of the queue type - Defined in the create queue function
  uint16_t       OSQLength;               ///< Length of the queue - Defined in the create queue function
  uint16_t       OSQEntries;              ///< Size of data inside the queue
} OS_DQUEUE;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Queue 16 Structure                          /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/**
* \struct OS_QUEUE
* Queue Control Block Structure
*/
typedef struct
{
  uint16_t  *OSQStart;               ///< Pointer to the queue start
  uint16_t  *OSQEnd;                 ///< Pointer to the queue end
  uint16_t  *OSQIn;                  ///< Pointer to the next queue entry
  uint16_t  *OSQOut;                 ///< Pointer to the next data in the queue output
  uint16_t  OSQSize;                 ///< Size of the queue - Defined in the create queue function
  uint16_t  OSQEntries;              ///< Size of data inside the queue
} OS_QUEUE_16;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Queue 32 Structure                          /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/**
* \struct OS_QUEUE
* Queue Control Block Structure
*/
typedef struct
{
  uint32_t  *OSQStart;               ///< Pointer to the queue start
  uint32_t  *OSQEnd;                 ///< Pointer to the queue end
  uint32_t  *OSQIn;                  ///< Pointer to the next queue entry
  uint32_t  *OSQOut;                 ///< Pointer to the next data in the queue output
  uint16_t  OSQSize;                 ///< Size of the queue - Defined in the create queue function
  uint16_t  OSQEntries;              ///< Size of data inside the queue
} OS_QUEUE_32;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Functions Prototypes                        /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

/*****************************************************************************************//**
* \fn uint8_t OSInstallTask(void(*FctPtr)(void),const char *TaskName, uint16_t USER_STACKED_BYTES,uint8_t iPriority, void *parameters, OS_CPU_TYPE *TaskHandle)
* \brief Install a task. Initial state = running.
* \param *FctPtr Pointer to the task to be installed
* \param *TaskName Task Name or task description
* \param USER_STACKED_BYTES Size of the task virtual stack. Depends on the user code and used interrupts.
* \param iPriority Desired task priority
* \param *parameters Task init parameters
* \param *TaskHandle Pointer to the task handle id
* \return OK Task successfully installed
* \return NO_MEMORY Not enough memory available to install the task
* \return END_OF_AVAILABLE_PRIORITIES All the available priorities are busy
* \return BUSY_PRIORITY Desired priority busy
*********************************************************************************************/
#if (TASK_WITH_PARAMETERS == 1)
  uint8_t OSInstallTask(void(*FctPtr)(void*),const CHAR8 *TaskName, uint16_t USER_STACKED_BYTES,uint8_t iPriority, void *parameters, OS_CPU_TYPE *TaskHandle);
  #define InstallTask OSInstallTask
#else
  uint8_t OSInstallTask(void(*FctPtr)(void),const CHAR8 *TaskName, uint16_t USER_STACKED_BYTES,uint8_t iPriority, OS_CPU_TYPE *TaskHandle);
  #define InstallTask OSInstallTask
#endif

/*****************************************************************************************//**
* \fn uint8_t OSUninstallTask(BRTOS_TH TaskHandle)
* \brief Uninstall a task from the dynamic memory
* \param TaskHandle The task handle id
* \return OK Task successfully uninstalled
* \return NOT_VALID_TASK Not valid task id or task is waiting for an event
*********************************************************************************************/
uint8_t OSUninstallTask(BRTOS_TH TaskHandle);
#define UninstallTask OSUninstallTask

/*****************************************************************************************//**
* \fn void Idle(void)
* \brief Idle Task. May be used to implement low power commands.
* \return NONE
*********************************************************************************************/
#if (TASK_WITH_PARAMETERS == 1)
  void Idle(void *parameters);
#else
  void Idle(void);
#endif

/*****************************************************************************************//**
* \fn void BRTOS_TimerHook(void)
* \brief Provide to the user a function sincronized with the timer tick
*  This function can be used to perform simple tests syncronized with the timer tick.
* \return NONE
*********************************************************************************************/  
#if (TIMER_HOOK_EN == 1)
void BRTOS_TimerHook(void);
#endif

/*****************************************************************************************//**
* \fn void IdleHook(void)
* \brief Provide to the user a function sincronized with the idle task
* \return NONE
*********************************************************************************************/  
#if (IDLE_HOOK_EN == 1)
void IdleHook(void);
#endif

/**************************************************************************//**
* \fn void OS_TICK_HANDLER(void)
* \brief Tick timer interrupt handler routine (Internal kernel function).
******************************************************************************/
void OS_TICK_HANDLER(void);

/*****************************************************************************************//**
* \fn uint8_t BRTOSStart(void)
* \brief Start the Operating System Scheduler
*  The user must call this function to start the tasks execution.
* \return OK Success
* \return NO_MEMORY There was not enough memory to start all tasks
*********************************************************************************************/
uint8_t BRTOSStart(void);

/*****************************************************************************************//**
* \fn uint8_t OSDelayTask(ostick_t time_wait)
* \brief Wait for a specified period.
*  A task that calling this function will be suspended for a certain time.
*  When this time is reached the task back to ready state.
* \param time_wait Time in ticks to delay. System default = 1ms. The user can change the time value.
* \return OK Success
* \return IRQ_PEND_ERR - Can not use block priority function from interrupt handler code
*********************************************************************************************/
uint8_t OSDelayTask(ostick_t time_wait);
#define DelayTask OSDelayTask

/*****************************************************************************************//**
* \fn uint8_t OSDelayTaskHMSM(uint8_t hours, uint8_t minutes, uint8_t seconds, uint16_t miliseconds)
* \brief Wait for a specified period (in hours, minutes, seconds and miliseconds).
*  A task that calling this function will be suspended for a certain time.
*  When this time is reached the task back to ready state.
* \param hours Hours to delay
* \param minutes Minutes to delay
* \param seconds Seconds to delay
* \param miliseconds Miliseconds to delay
* \return OK Success
* \return INVALID_TIME The specified parameters are outside of the permitted range
*********************************************************************************************/  
uint8_t OSDelayTaskHMSM(uint8_t hours, uint8_t minutes, uint8_t seconds, uint16_t miliseconds);
#define DelayTaskHMSM OSDelayTaskHMSM

/*****************************************************************************************//**
* \fn ostick_t OSGetTickCount(void)
* \brief Return current tick count.
*  The user must call this function in order to receive the current tick count.
* \return current tick count
*********************************************************************************************/
ostick_t OSGetTickCount(void);

/*****************************************************************************************//**
* \fn ostick_t OSGetCount(void)
* \brief Return current tick count.
*  Internal BRTOS function.
* \return current tick count
*********************************************************************************************/
ostick_t OSGetCount(void);

/*****************************************************************************************//**
* \fn void OSIncCounter(void)
* \brief Update the tick counter.
* \return NONE
*********************************************************************************************/
void OSIncCounter(void);

/*****************************************************************************************//**
* \fn void PreInstallTasks(void)
* \brief Function that initialize the kernel main variables.
*  This function resets the kernel main variables, preparing the system to be started.
* \return NONE
*********************************************************************************************/  
void PreInstallTasks(void);

/*****************************************************************************************//**
* \fn uint8_t OSBlockPriority(uint8_t iPriority)
* \brief Blocks a specific priority
*  Blocks the task that is associated with the specified priority.
*  The user must be careful when using this function in together with mutexes.
*  This can lead to undesired results due the "cealing priority" property used in the mutex.
* \param iPriority Priority to be blocked
* \return OK - Success
* \return IRQ_PEND_ERR - Can not use block priority function from interrupt handler code
*********************************************************************************************/  
uint8_t OSBlockPriority(uint8_t iPriority);
#define BlockPriority OSBlockPriority

/*****************************************************************************************//**
* \fn uint8_t OSUnBlockPriority(uint8_t iPriority)
* \brief UnBlock a specific priority
*  UnBlocks the task that is associated with the specified priority.
*  The user must be careful when using this function in together with mutexes.
*  This can lead to undesired results due the "cealing priority" property used in the mutex.
* \param iPriority Priority to be unblocked
* \return OK - Success
* \return IRQ_PEND_ERR - Can not use unblock priority function from interrupt handler code
*********************************************************************************************/
uint8_t OSUnBlockPriority(uint8_t iPriority);
#define UnBlockPriority OSUnBlockPriority
/*****************************************************************************************//**
* \fn uint8_t OSBlockTask(uint8_t iTaskNumber)
* \brief Blocks a specific task
* \param iTaskNumber Task number to be blocked
* \return OK - Success
* \return IRQ_PEND_ERR - Can not use block task function from interrupt handler code
*********************************************************************************************/
uint8_t OSBlockTask(BRTOS_TH iTaskNumber);
#define BlockTask OSBlockTask

/*****************************************************************************************//**
* \fn uint8_t OSUnBlockTask(uint8_t iTaskNumber)
* \brief UnBlocks a specific task
* \param iTaskNumber Task number to be unblocked
* \return OK - Success
* \return IRQ_PEND_ERR - Can not use unblock task function from interrupt handler code
*********************************************************************************************/
uint8_t OSUnBlockTask(BRTOS_TH iTaskNumber);
#define UnBlockTask OSUnBlockTask

/*****************************************************************************************//**
* \fn uint8_t OSBlockMultipleTask(uint8_t TaskStart, uint8_t TaskNumber)
* \brief Blocks a set of tasks
* \param TaskStart Number of the first task to be blocked
* \param TaskNumber Number of tasks to be blocked from the specified task start
* \return OK - Success
* \return IRQ_PEND_ERR - Can not use block multiple tasks function from interrupt handler code
*********************************************************************************************/
uint8_t OSBlockMultipleTask(uint8_t TaskStart, uint8_t TaskNumber);
#define BlockMultipleTask OSBlockMultipleTask

/*****************************************************************************************//**
* \fn uint8_t OSUnBlockMultipleTask(uint8_t TaskStart, uint8_t TaskNumber)
* \brief UnBlocks a set of tasks
* \param TaskStart Number of the first task to be unblocked
* \param TaskNumber Number of tasks to be unblocked from the specified task start
* \return OK - Success
* \return IRQ_PEND_ERR - Can not use unblock multiple tasks function from interrupt handler code
*********************************************************************************************/
uint8_t OSUnBlockMultipleTask(uint8_t TaskStart, uint8_t TaskNumber);
#define UnBlockMultipleTask OSUnBlockMultipleTask

/*********************************************************************************//**
* \fn void BRTOSInit(void)
* \brief Initialize BRTOS control blocks and tick timer (Internal kernel function).
*************************************************************************************/
void BRTOSInit(void);
#define BRTOS_Init BRTOSInit

/*****************************************************************//**
* \fn uint8_t OSSchedule(void)
* \brief BRTOS Scheduler function (Internal kernel function).
*********************************************************************/
uint8_t OSSchedule(void);

/*****************************************************************//**
* \fn uint8_t SAScheduler(PriorityType ReadyList)
* \brief Sucessive Aproximation Scheduler (Internal kernel function).
* \param ReadyList List of the tasks ready to run
* \return The priority of the highest priority task ready to run
*********************************************************************/
uint8_t SAScheduler(PriorityType ReadyList);



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Event Variables Extern Declarations         /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#if (BRTOS_SEM_EN == 1)
  /// Semahore Control Block
  extern BRTOS_Sem BRTOS_Sem_Table[BRTOS_MAX_SEM];
#endif

#if (BRTOS_MUTEX_EN == 1)
  /// Mutex Control Block
  extern BRTOS_Mutex BRTOS_Mutex_Table[BRTOS_MAX_MUTEX];
#endif

#if (BRTOS_MBOX_EN == 1)
  /// MailBox Control Block
  extern BRTOS_Mbox BRTOS_Mbox_Table[BRTOS_MAX_MBOX];
#endif

#if (BRTOS_QUEUE_EN == 1)
  /// Queue Control Block
  extern BRTOS_Queue BRTOS_Queue_Table[BRTOS_MAX_QUEUE];
  extern OS_QUEUE	 BRTOS_OS_QUEUE_Table[BRTOS_MAX_QUEUE];
#endif


/*****************************************************************************************//**
* \fn void initEvents(void)
* \brief Initialize event control blocks
* \return NONE
*********************************************************************************************/  
void initEvents(void);


#if (BRTOS_SEM_EN == 1)

  /*****************************************************************************************//**
  * \fn uint8_t OSSemCreate (uint8_t cnt, BRTOS_Sem **event)
  * \brief Allocates a semaphore control block
  * \param cnt Initial Semaphore counter - default = 0
  * \param **event Address of the semaphore control block pointer
  * \return IRQ_PEND_ERR Can not use semaphore create function from interrupt handler code
  * \return NO_AVAILABLE_EVENT No semaphore control blocks available
  * \return ALLOC_EVENT_OK Semaphore control block successfully allocated
  *********************************************************************************************/
  uint8_t OSSemCreate (uint8_t cnt, BRTOS_Sem **event);

#if (BRTOS_BINARY_SEM_EN == 1)
  /*****************************************************************************************//**
  * \fn uint8_t OSSemBinaryCreate (uint8_t cnt, BRTOS_Sem **event)
  * \brief Allocates a semaphore control block
  * \param bit Initial Semaphore bit value - default = 0
  * \param **event Address of the semaphore control block pointer
  * \return IRQ_PEND_ERR Can not use semaphore create function from interrupt handler code
  * \return NO_AVAILABLE_EVENT No semaphore control blocks available
  * \return ALLOC_EVENT_OK Semaphore control block successfully allocated
  *********************************************************************************************/
  uint8_t OSSemBinaryCreate(uint8_t bit, BRTOS_Sem **event);
#endif
  
  /*****************************************************************************************//**
  * \fn uint8_t OSSemDelete (BRTOS_Sem **event)
  * \brief Releases a semaphore control block
  * \param **event Address of the semaphore control block pointer
  * \return IRQ_PEND_ERR Can not use semaphore delete function from interrupt handler code
  * \return DELETE_EVENT_OK Semaphore control block released with success
  *********************************************************************************************/
  uint8_t OSSemDelete (BRTOS_Sem **event);

  /*****************************************************************************************//**
  * \fn uint8_t OSSemPend (BRTOS_Sem *pont_event, ostick_t timeout)
  * \brief Wait for a semaphore post
  *  Semaphore pend may be used to syncronize tasks or wait for an event occurs.
  *  A task exits a pending state with a semaphore post or by timeout.
  * \param *pont_event Semaphore pointer
  * \param timeout Timeout to the semaphore pend exits
  * \return OK Success
  * \return TIMEOUT There was no post for this semaphore in the specified time
  * \return IRQ_PEND_ERR Can not use semaphore pend function from interrupt handler code
  * \return NO_EVENT_SLOT_AVAILABLE Full Event list
  *********************************************************************************************/
  uint8_t OSSemPend (BRTOS_Sem *pont_event, ostick_t timeout);
  
  /*****************************************************************************************//**
  * \fn uint8_t OSSemPost(BRTOS_Sem *pont_event)
  * \brief Semaphore post
  *  Semaphore Post may be used to syncronize tasks or to inform that an event occurs.
  * \param *pont_event Semaphore pointer
  * \return OK Success
  * \return ERR_SEM_OVF Semaphore counter overflow
  *********************************************************************************************/  
  uint8_t OSSemPost(BRTOS_Sem *pont_event);
#endif

#if (BRTOS_MUTEX_EN == 1)
  /*****************************************************************************************//**
  * \fn uint8_t OSMutexCreate (BRTOS_Mutex **event, uint8_t HigherPriority)
  * \brief Allocates a mutex control block
  * \param **event Address of the mutex control block pointer
  * \param HigherPriority Higher priority of the tasks that will share a resource
  * \return IRQ_PEND_ERR Can not use mutex create function from interrupt handler code
  * \return NO_AVAILABLE_EVENT No mutex control blocks available
  * \return ALLOC_EVENT_OK Mutex control block successfully allocated
  *********************************************************************************************/
  uint8_t OSMutexCreate (BRTOS_Mutex **event, uint8_t HigherPriority);
  
  /*****************************************************************************************//**
  * \fn uint8_t OSMutexDelete (BRTOS_Mutex **event)
  * \brief Releases a mutex control block
  * \param **event Address of the mutex control block pointer
  * \return IRQ_PEND_ERR Can not use mutex delete function from interrupt handler code
  * \return DELETE_EVENT_OK Mutex control block released with success
  *********************************************************************************************/  
  uint8_t OSMutexDelete (BRTOS_Mutex **event);

  /*****************************************************************************************//**
  * \fn uint8_t OSMutexAcquire(BRTOS_Mutex *pont_event, ostick_t time_wait)
  * \brief Wait for a mutex release
  *  Mutex release may be used to manage shared resources, for exemple, a LCD.
  *  A acquired state exits with a mutex owner release
  * \param *pont_event Mutex pointer
  * \param time_wait Timeout to the mutex acquire exits
  * \return OK Success
  * \return IRQ_PEND_ERR Can not use mutex pend function from interrupt handler code
  * \return NO_EVENT_SLOT_AVAILABLE Full Event list
  *********************************************************************************************/
  uint8_t OSMutexAcquire(BRTOS_Mutex *pont_event, ostick_t time_wait);

  /*****************************************************************************************//**
  * \fn uint8_t OSMutexRelease(BRTOS_Mutex *pont_event)
  * \brief Release Mutex
  *  Mutex release must be used to release a shared resource.
  *  Only the mutex owner can executed the mutex post function with success.
  * \param *pont_event Mutex pointer
  * \return OK Success
  * \return ERR_EVENT_OWNER The function caller is not the mutex owner
  * \return ERR_MUTEX_OVF Mutex counter overflow
  *********************************************************************************************/  
  uint8_t OSMutexRelease(BRTOS_Mutex *pont_event);
#endif

#if (BRTOS_MBOX_EN == 1)

  /*****************************************************************************************//**
  * \fn uint8_t OSMboxCreate (BRTOS_Mbox **event, void *message)
  * \brief Allocates a mailbox control block
  * \param **event Address of the mailbox control block pointer
  * \param *message Specifies an initial message for the allocated mailbox control block
  * \return IRQ_PEND_ERR Can not use mailbox create function from interrupt handler code
  * \return NO_AVAILABLE_EVENT No mailbox control blocks available
  * \return ALLOC_EVENT_OK Mailbox control block successfully allocated
  *********************************************************************************************/
  uint8_t OSMboxCreate (BRTOS_Mbox **event, void *message);
  
  /*****************************************************************************************//**
  * \fn uint8_t OSMboxDelete (BRTOS_Mbox **event)
  * \brief Releases a mailbox control block
  * \param **event Address of the mailbox control block pointer
  * \return IRQ_PEND_ERR Can not use mailbox delete function from interrupt handler code
  * \return DELETE_EVENT_OK Mailbox control block released with success
  *********************************************************************************************/  
  uint8_t OSMboxDelete (BRTOS_Mbox **event);
  
  /*****************************************************************************************//**
  * \fn void *OSMboxPend (BRTOS_Mbox *pont_event, ostick_t timeout)
  * \brief Wait for a message post
  *  Mailbox pend may be used to receive messages from tasks and interrupts.
  *  A task exits a pending state with a mailbox post or by timeout.
  *  A message could be of any kind of data type.
  * \param *pont_event Mailbox pointer
  * \param *Mail Mail content pointer
  * \param timeout Timeout to the mailbox pend exits
  * \return OK Success
  * \return TIMEOUT There was no post for this semaphore in the specified time
  * \return IRQ_PEND_ERR Can not use semaphore pend function from interrupt handler code
  *********************************************************************************************/  
  uint8_t OSMboxPend (BRTOS_Mbox *pont_event, void **Mail, ostick_t timeout);
  
  /*****************************************************************************************//**
  * \fn uint8_t OSMboxPost(BRTOS_Mbox *pont_event, void *message)
  * \brief Mailbox post
  *  Mailbox post may be used to send messages to tasks.
  *  A message could be of any kind of data type.
  * \param *pont_event Semaphore pointer
  * \param *message Pointer to the message to be sent
  * \return OK Success
  * \return ERR_EVENT_NO_CREATED No tasks waiting for the message
  *********************************************************************************************/  
  uint8_t OSMboxPost(BRTOS_Mbox *pont_event, void *message);
#endif


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Queue Prototypes                            /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#if (BRTOS_QUEUE_EN == 1)

  /*****************************************************************************************//**
  * \fn uint8_t OSQueueCreate(OS_QUEUE *cqueue, uint16_t size, BRTOS_Queue **event)
  * \brief Allocates a queue control block  
  * \param size Queue size
  * \param **event Queue event pointer
  * \return IRQ_PEND_ERR Can not use queue create function from interrupt handler code
  * \return NO_AVAILABLE_EVENT No queue control blocks available
  * \return ALLOC_EVENT_OK Queue control block successfully allocated
  *********************************************************************************************/
  uint8_t OSQueueCreate(uint16_t size, BRTOS_Queue **event);
 
  /*****************************************************************************************//**
  * \fn OSWQueue(OS_QUEUE *cqueue,uint8_t data)
  * \brief Writes new data in the specified queue
  * \param *cqueue Pointer to a queue
  * \param data Data to be written in the queue
  * \return BUFFER_UNDERRUN Queue overflow - There is no more available for new data
  * \return READ_BUFFER_OK New data successfully written
  *********************************************************************************************/
  uint8_t OSWQueue(OS_QUEUE *cqueue,uint8_t data);

  /*****************************************************************************************//**
  * \fn uint8_t OSRQueue(OS_QUEUE *cqueue, uint8_t* pdata)
  * \brief Reads new data from the specified queue
  * \param *cqueue Pointer to a queue
  * \param *pdata Pointer to data read - first data in the output buffer of the specified queue
  * \return READ_BUFFER_OK Data successfully read
  * \return NO_ENTRY_AVAILABLE There is no more available entry in queue
  *********************************************************************************************/
  uint8_t OSRQueue(OS_QUEUE *cqueue, uint8_t* pdata);
  
  /*****************************************************************************************//**
  * \fn uint8_t OSQueueClean(OS_QUEUE *cqueue)
  * \brief Clean data in the specified queue
  * \param *cqueue Pointer to a queue
  * \return CLEAN_BUFFER_OK Queue successfully cleaned
  *********************************************************************************************/  
  uint8_t OSQueueClean(BRTOS_Queue *pont_event);
  
  /*****************************************************************************************//**
  * \fn uint8_t OSQueuePend (BRTOS_Queue *pont_event, OS_QUEUE *cqueue, ostick_t timeout)
  * \brief Wait for a queue post 
  *  A task exits a pending state with a queue post or by timeout.
  * \param *pont_event Queue event pointer
  * \param *cqueue Queue pointer
  * \param timeout Timeout to the queue pend exits
  * \return First data in the output buffer of the specified queue
  *********************************************************************************************/
  uint8_t OSQueuePend (BRTOS_Queue *pont_event, uint8_t* pdata, ostick_t timeout);
  
  /*****************************************************************************************//**
  * \fn uint8_t OSQueuePost(BRTOS_Queue *pont_event, OS_QUEUE *cqueue,uint8_t data)
  * \brief Queue post
  *  A task exits a pending state with a queue post or by timeout.
  * \param *pont_event Queue event pointer
  * \param *cqueue Queue pointer
  * \param data Data to be written in the queue
  * \param timeout Timeout to the queue pend exits
  * \return First data in the output buffer of the specified queue
  *********************************************************************************************/
  uint8_t OSQueuePost(BRTOS_Queue *pont_event, uint8_t data); 
#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Queue 16 bits Prototypes                    /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#if (BRTOS_QUEUE_16_EN == 1)

  /*****************************************************************************************//**
  * \fn uint8_t OSQueueCreate(OS_QUEUE *cqueue, uint16_t size, BRTOS_Queue **event)
  * \brief Allocates a queue control block  
  * \param *cqueue Queue pointer
  * \param size Queue size
  * \return IRQ_PEND_ERR Can not use queue create function from interrupt handler code
  * \return NO_AVAILABLE_EVENT No queue control blocks available
  * \return ALLOC_EVENT_OK Queue control block successfully allocated
  *********************************************************************************************/
  uint8_t OSQueue16Create(OS_QUEUE_16 *cqueue, uint16_t size);
  
  /*****************************************************************************************//**
  * \fn OSWQueue(OS_QUEUE *cqueue,uint8_t data)
  * \brief Writes new data in the specified queue
  * \param *cqueue Pointer to a queue
  * \param data Data to be written in the queue
  * \return BUFFER_UNDERRUN Queue overflow - There is no more available for new data
  * \return READ_BUFFER_OK New data successfully written
  *********************************************************************************************/
  uint8_t OSWQueue16(OS_QUEUE_16 *cqueue,uint16_t data);

  /*****************************************************************************************//**
  * \fn uint8_t OSRQueue(OS_QUEUE *cqueue)
  * \brief Reads new data from the specified queue
  * \param *cqueue Pointer to a queue
  * \param *pdata Pointer to data read - first data in the output buffer of the specified queue  
  * \return First data in the output buffer of the specified queue
  *********************************************************************************************/
  uint8_t OSRQueue16(OS_QUEUE_16 *cqueue, uint16_t *pdata);
  
  /*****************************************************************************************//**
  * \fn uint8_t OSCleanQueue(OS_QUEUE *cqueue)
  * \brief Clean data in the specified queue
  * \param *cqueue Pointer to a queue
  * \return CLEAN_BUFFER_OK Queue successfully cleaned
  *********************************************************************************************/  
  uint8_t OSCleanQueue16(OS_QUEUE_16 *cqueue);
    
#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////




////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      Queue 32 bits Prototypes                    /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#if (BRTOS_QUEUE_32_EN == 1)

  /*****************************************************************************************//**
  * \fn uint8_t OSQueueCreate(OS_QUEUE *cqueue, uint16_t size, BRTOS_Queue **event)
  * \brief Allocates a queue control block  
  * \param *cqueue Queue pointer
  * \param size Queue size
  * \return IRQ_PEND_ERR Can not use queue create function from interrupt handler code
  * \return NO_AVAILABLE_EVENT No queue control blocks available
  * \return ALLOC_EVENT_OK Queue control block successfully allocated
  *********************************************************************************************/
  uint8_t OSQueue32Create(OS_QUEUE_32 *cqueue, uint16_t size);
  
  /*****************************************************************************************//**
  * \fn OSWQueue(OS_QUEUE *cqueue,uint8_t data)
  * \brief Writes new data in the specified queue
  * \param *cqueue Pointer to a queue
  * \param data Data to be written in the queue
  * \return BUFFER_UNDERRUN Queue overflow - There is no more available for new data
  * \return READ_BUFFER_OK New data successfully written
  *********************************************************************************************/
  uint8_t OSWQueue32(OS_QUEUE_32 *cqueue,uint32_t data);

  /*****************************************************************************************//**
  * \fn uint8_t OSRQueue(OS_QUEUE *cqueue)
  * \brief Reads new data from the specified queue
  * \param *cqueue Pointer to a queue
  * \param *pdata Pointer to data read - first data in the output buffer of the specified queue  
  * \return First data in the output buffer of the specified queue
  *********************************************************************************************/
  uint8_t OSRQueue32(OS_QUEUE_32 *cqueue, uint32_t *pdata);
  
  /*****************************************************************************************//**
  * \fn uint8_t OSCleanQueue(OS_QUEUE *cqueue)
  * \brief Clean data in the specified queue
  * \param *cqueue Pointer to a queue
  * \return CLEAN_BUFFER_OK Queue successfully cleaned
  *********************************************************************************************/  
  uint8_t OSCleanQueue32(OS_QUEUE_32 *cqueue);
    
#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





#if (BRTOS_DYNAMIC_QUEUE_ENABLED == 1)

  /*****************************************************************************************//**
  * \fn uint8_t OSDQueueCreate(uint16_t queue_lenght, OS_CPU_TYPE type_size, BRTOS_Queue **event)
  * \brief Allocates a queue control block and queue data size
  * \param queue_lenght Queue lenght
  * \param type_size Queue type size
  * \param **event Queue event pointer
  * \return INVALID_PARAMETERS There is at least one invalid parameter
  * \return NO_AVAILABLE_MEMORY There is no memory for allocate the queue
  * \return IRQ_PEND_ERR Can not use queue create function from interrupt handler code
  * \return NO_AVAILABLE_EVENT No queue control blocks available
  * \return ALLOC_EVENT_OK Queue control block successfully allocated
  *********************************************************************************************/
  uint8_t OSDQueueCreate(uint16_t queue_lenght, OS_CPU_TYPE type_size, BRTOS_Queue **event);
  
  /*****************************************************************************************//**
  * \fn uint8_t OSDQueueDelete (BRTOS_Queue **event)
  * \brief Releases a queue control block
  * \param **event Address of the queue control block pointer
  * \return IRQ_PEND_ERR Can not use queue delete function from interrupt handler code
  * \return DELETE_EVENT_OK Queue control block released with success
  *********************************************************************************************/  
  uint8_t OSDQueueDelete (BRTOS_Queue **event);
  
  /*****************************************************************************************//**
  * \fn uint8_t OSDQueueClean(BRTOS_Queue *pont_event)
  * \brief Clean data in the specified queue
  * \param **event Queue event pointer
  * \return CLEAN_BUFFER_OK Queue successfully cleaned
  *********************************************************************************************/  
  uint8_t OSDQueueClean(BRTOS_Queue *pont_event);
  
  /*****************************************************************************************//**
  * \fn uint8_t OSDQueuePend (BRTOS_Queue *pont_event, void *pdata, ostick_t time_wait)
  * \brief Wait for a queue post 
  *  A task exits a pending state with a queue post or by timeout.
  * \param *pont_event Queue event pointer
  * \param timeout Timeout to the queue pend exits
  * \param *pdata First data in the output buffer of the specified queue
  * \return ERR_EVENT_NO_CREATED The pont_event is not valid
  * \return TIMEOUT The queue pend exit by timeout
  * \return READ_BUFFER_OK The queue was successfully read
  *********************************************************************************************/
  uint8_t OSDQueuePend (BRTOS_Queue *pont_event, void *pdata, ostick_t time_wait);
  
  /*****************************************************************************************//**
  * \fn uint8_t OSDQueuePost(BRTOS_Queue *pont_event, void *pdata)
  * \brief Queue post
  *  A task exits a pending state with a queue post or by timeout.
  * \param *pont_event Queue event pointer
  * \param *pdata Pointer of the data to be written in the queue
  * \param timeout Timeout to the queue pend exits
  * \return
  *********************************************************************************************/
  uint8_t OSDQueuePost(BRTOS_Queue *pont_event, void *pdata);
#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////





////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////      OS Variables Extern Declarations            /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

extern       PriorityType OSReadyList;
extern       PriorityType OSBlockedList;
extern const PriorityType PriorityMask[configMAX_TASK_PRIORITY+1];

extern ContextType *Tail;
extern ContextType *Head;

extern uint8_t                iNesting;
extern volatile uint8_t       currentTask;
extern volatile uint8_t       SelectedTask;
extern ContextType          ContextTask[NUMBER_OF_TASKS + 1];
extern uint16_t               iStackAddress;
extern uint8_t                NumberOfInstalledTasks;
extern volatile uint32_t      OSDuty;
extern uint8_t                PriorityVector[configMAX_TASK_INSTALL];
extern volatile uint32_t      OSDutyTmp;

#ifdef TICK_TIMER_32BITS
  extern volatile uint32_t LastOSDuty;
#else
  extern volatile uint16_t LastOSDuty;
#endif

#ifdef OS_CPU_TYPE
#if (!BRTOS_DYNAMIC_TASKS_ENABLED)
  extern OS_CPU_TYPE STACK[(HEAP_SIZE / sizeof(OS_CPU_TYPE))];
#endif
  extern OS_CPU_TYPE QUEUE_STACK[(QUEUE_HEAP_SIZE / sizeof(OS_CPU_TYPE))];
#else
	#error("You must define the OS_CPU_TYPE !!!")
#endif

extern uint32_t TaskAlloc;
extern uint16_t iQueueAddress;

#if (PROCESSOR == ATMEGA)
#if (!defined __GNUC__)
#define CONST
#else
#define CONST const
#endif
extern PGM_P CONST BRTOSStringTable[] PROGMEM;
#else
#if (PROCESSOR == PIC18)
extern const rom CHAR8 *version;
#else
extern const CHAR8 *version;
#endif
#endif

#if ((PROCESSOR == ATMEGA) || (PROCESSOR == PIC18))
extern CHAR8 BufferText[32];
#endif


extern stack_pointer_t StackAddress;



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#if (defined ISR_DEDICATED_STACK && ISR_DEDICATED_STACK == 1)

////////////////////////////////////////////////////////////
#define OS_INT_ENTER() if (!iNesting){OS_SAVE_SP(); OS_RESTORE_ISR_SP(); }; iNesting++;

#define OS_INT_EXIT()                                                   \
  CriticalDecNesting();                                                 \
  if (!iNesting)                                                        \
  {                                                                     \
	OS_RESTORE_SP();                                                    \
    SelectedTask = OSSchedule();                                        \
    if (currentTask != SelectedTask){                                   \
        OS_SAVE_CONTEXT();                                              \
        OS_SAVE_SP();                                                   \
        ContextTask[currentTask].StackPoint = SPvalue;                  \
	      currentTask = SelectedTask;                                   \
        SPvalue = ContextTask[currentTask].StackPoint;                  \
        OS_RESTORE_SP();                                                \
        OS_RESTORE_CONTEXT();                                           \
    }                                                                   \
  }                                                                     \
  

#else

#define OS_INT_ENTER()  iNesting++;

      
#if (COMPUTES_TASK_LOAD == 1)
extern void COMPUTE_TASK_LOAD(void);

#define OS_INT_EXIT()                                                   					\
	CriticalDecNesting();                                                 					\
	if (!iNesting)                                                        					\
	{                                                                     					\
		SelectedTask = OSSchedule();                                        				\
		if (currentTask != SelectedTask){                                   				\
			COMPUTE_TASK_LOAD();															\
			OS_SAVE_CONTEXT();                                              				\
			OS_SAVE_SP();                                                   				\
			ContextTask[currentTask].StackPoint = SPvalue;                  				\
			currentTask = SelectedTask;                                   					\
			SPvalue = ContextTask[currentTask].StackPoint;                  				\
			OS_RESTORE_SP();                                                				\
			OS_RESTORE_CONTEXT();                                           				\
		}																					\
    }																						\

#else
#define OS_INT_EXIT()                                                   \
  CriticalDecNesting();                                                 \
  if (!iNesting)                                                        \
  {                                                                     \
    SelectedTask = OSSchedule();                                        \
    if (currentTask != SelectedTask){                                   \
        OS_SAVE_CONTEXT();                                              \
        OS_SAVE_SP();                                                   \
        ContextTask[currentTask].StackPoint = SPvalue;                  \
	    currentTask = SelectedTask;                                     \
        SPvalue = ContextTask[currentTask].StackPoint;                  \
        OS_RESTORE_SP();                                                \
        OS_RESTORE_CONTEXT();                                           \
    }                                                                   \
  }                                                                     \
  
#endif
#endif

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////


#define RemoveFromDelayList()                       \
        if(Task == Head)                            \
        {                                           \
          if(Task == Tail)                          \
          {                                         \
            Tail = NULL;                            \
            Head = NULL;                            \
          }                                         \
          else                                      \
          {                                         \
            Head = Task->Next;                      \
            Head->Previous = NULL;                  \
          }                                         \
        }                                           \
        else                                        \
        {                                           \
          if(Task == Tail)                          \
          {                                         \
            Tail = Task->Previous;                  \
            Tail->Next = NULL;                      \
          }                                         \
          else                                      \
          {                                         \
            Task->Next->Previous = Task->Previous;  \
            Task->Previous->Next = Task->Next;      \
          }                                         \
        }


#define IncludeTaskIntoDelayList()                  \
        if(Tail != NULL)                            \
        {                                           \
          /* Insert task into list */               \
          Tail->Next = Task;                        \
          Task->Previous = Tail;                    \
          Tail = Task;                              \
          Tail->Next = NULL;                        \
        }                                           \
        else{                                       \
           /* Init delay list */                    \
           Tail = Task;                             \
           Head = Task;                             \
           Task->Next = NULL;                       \
           Task->Previous = NULL;                   \
        }


#endif
