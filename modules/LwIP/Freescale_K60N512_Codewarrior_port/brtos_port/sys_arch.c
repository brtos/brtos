/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#include "BRTOS.h"

/* lwIP includes. */
#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/mem.h"

#include <stdio.h>

/* Message queue constants. */
#define archMESG_QUEUE_LENGTH	( 6 )
#define archPOST_BLOCK_TIME_MS	( ( unsigned long ) 10000 )

#define THREAD_INIT( tcb ) \
    do { \
        tcb->next = NULL; \
        tcb->pid = ( sys_thread_t )0; \
        tcb->timeouts.next = NULL; \
    } while( 0 )

/* ------------------------ Type definitions ------------------------------ */
typedef struct sys_tcb
{
    struct sys_tcb *next;
    struct sys_timeouts timeouts;
    sys_thread_t     pid;
} sys_tcb_t;

struct timeoutlist
{
	struct sys_timeouts timeouts;
	sys_thread_t pid;
};

/* This is the number of threads that can be started with sys_thread_new() */
#define SYS_THREAD_MAX 4

/* ------------------------ Static variables ------------------------------ */
static sys_tcb_t *tasks = NULL;

/*-----------------------------------------------------------------------------------*/
//  Creates an empty mailbox.
sys_mbox_t
sys_mbox_new(int size)
{
	sys_mbox_t mbox;

	(void)OSDQueueCreate(size, sizeof( void * ), &mbox);
        
	return mbox;
}

/*-----------------------------------------------------------------------------------*/
/*
  Deallocates a mailbox. If there are messages still present in the
  mailbox when the mailbox is deallocated, it is an indication of a
  programming error in lwIP and the developer should be notified.
*/
void
sys_mbox_free(sys_mbox_t mbox)
{
	/*
	if( uxQueueMessagesWaiting( mbox ) )
	{
		// Line for breakpoint.  Should never break here!
		//__asm volatile ( "NOP" );
	}
	*/

	(void)OSDQueueDelete (&mbox);
}

/*-----------------------------------------------------------------------------------*/
//   Posts the "msg" to the mailbox.
void
sys_mbox_post(sys_mbox_t mbox, void *data)
{
	(void)OSDQueuePost(mbox, &data);
}

/*-----------------------------------------------------------------------------------*/

/*FSL*/
/*  
 *Try to post the "msg" to the mailbox. Returns ERR_MEM if this one
 *is full, else, ERR_OK if the "msg" is posted.
 */
err_t
sys_mbox_trypost( sys_mbox_t mbox, void *data )
{
    /* Queue must not be full - Otherwise it is an error. */
    if(!OSDQueuePost(mbox, &data))
    {
    	return ERR_OK;
    }
    else
    {
    	return ERR_MEM;
    }
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread until a message arrives in the mailbox, but does
  not block the thread longer than "timeout" milliseconds (similar to
  the sys_arch_sem_wait() function). The "msg" argument is a result
  parameter that is set by the function (i.e., by doing "*msg =
  ptr"). The "msg" parameter maybe NULL to indicate that the message
  should be dropped.

  The return values are the same as for the sys_arch_sem_wait() function:
  Number of milliseconds spent waiting or SYS_ARCH_TIMEOUT if there was a
  timeout.

  Note that a function with a similar name, sys_mbox_fetch(), is
  implemented by lwIP.
*/
u32_t sys_arch_mbox_fetch(sys_mbox_t mbox, void **msg, u32_t timeout)
{
void *dummyptr;
INT16U StartTime, EndTime, Elapsed;

	StartTime = OSGetTickCount();

	if( msg == NULL )
	{
		msg = &dummyptr;
	}
		
	if(	timeout != 0 )
	{
		if(!OSDQueuePend (mbox, &(*msg), timeout))
		{
			EndTime = OSGetTickCount();
			if (EndTime > StartTime)
			{
				Elapsed = EndTime - StartTime;
			}else
			{
				Elapsed = (TICK_COUNT_OVERFLOW - StartTime) + EndTime;
			}
			if( Elapsed == 0 )
			{
				Elapsed = 1;
			}
			return ( Elapsed );
		}
		else // timed out blocking for message
		{
			*msg = NULL;
			return SYS_ARCH_TIMEOUT;
		}
	}
	else // block forever for a message.
	{
		while( OSDQueuePend (mbox, &(*msg), 10000)) // time is arbitrary
		{
			;
		}
		EndTime = OSGetTickCount();
		if (EndTime > StartTime)
		{
			Elapsed = EndTime - StartTime;
		}else
		{
			Elapsed = (TICK_COUNT_OVERFLOW - StartTime) + EndTime;
		}
		if( Elapsed == 0 )
		{
			Elapsed = 1;
		}
		return ( Elapsed ); // return time blocked TBD test	
	}
}

/*-----------------------------------------------------------------------------------*/
//  Creates and returns a new semaphore. The "count" argument specifies
//  the initial state of the semaphore. TBD finish and test
sys_sem_t
sys_sem_new(u8_t count)
{
	sys_sem_t  Semaphore = NULL;

	OSSemCreate(count,&Semaphore);

	if(count == 0)	// Means it can't be taken
	{
		OSSemPend(Semaphore, 1);
	}


	return Semaphore; 	// if return NULL TBD need assert
}

/*-----------------------------------------------------------------------------------*/
/*
  Blocks the thread while waiting for the semaphore to be
  signaled. If the "timeout" argument is non-zero, the thread should
  only be blocked for the specified time (measured in
  milliseconds).

  If the timeout argument is non-zero, the return value is the number of
  milliseconds spent waiting for the semaphore to be signaled. If the
  semaphore wasn't signaled within the specified time, the return value is
  SYS_ARCH_TIMEOUT. If the thread didn't have to wait for the semaphore
  (i.e., it was already signaled), the function may return zero.

  Notice that lwIP implements a function with a similar name,
  sys_sem_wait(), that uses the sys_arch_sem_wait() function.
*/
u32_t
sys_arch_sem_wait(sys_sem_t sem, u32_t timeout)
{
	INT16U StartTime, EndTime, Elapsed;

	StartTime = OSGetTickCount();

	if(	timeout != 0)
	{
		if(!OSSemPend(sem, timeout))
		{
			EndTime = OSGetTickCount();
			if (EndTime > StartTime)
			{
				Elapsed = EndTime - StartTime;
			}else
			{
				Elapsed = (TICK_COUNT_OVERFLOW - StartTime) + EndTime;
			}
			if( Elapsed == 0 )
			{
				Elapsed = 1;
			}
			return (Elapsed); // return time blocked TBD test	
		}
		else
		{
			return SYS_ARCH_TIMEOUT;
		}
	}
	else // must block without a timeout
	{
		while( OSSemPend(sem, 10000) )
		{
			;
		}
		EndTime = OSGetTickCount();
		if (EndTime > StartTime)
		{
			Elapsed = EndTime - StartTime;
		}else
		{
			Elapsed = (TICK_COUNT_OVERFLOW - StartTime) + EndTime;
		}
		if( Elapsed == 0 )
		{
			Elapsed = 1;
		}

		return ( Elapsed ); // return time blocked	
		
	}
}

/*-----------------------------------------------------------------------------------*/
// Signals a semaphore
void
sys_sem_signal(sys_sem_t sem)
{
	(void)OSSemPost(sem);
}

/*-----------------------------------------------------------------------------------*/
// Deallocates a semaphore
void
sys_sem_free(sys_sem_t sem)
{
	OSSemDelete(&sem);
}

/*-----------------------------------------------------------------------------------*/
// Initialize sys arch
void
sys_init(void)
{
    tasks = NULL;
}

sys_thread_t xTaskGetCurrentTaskHandle( void )
{
	OS_SR_SAVE_VAR
	sys_thread_t xReturn;

	/* A critical section is not required as this is not called from
	an interrupt and the current TCB will always be the same for any
	individual execution thread. */
	
    // Enter Critical Section
    OSEnterCritical();
    
	xReturn = &ContextTask[currentTask];
    
	// Exit Critical Section
    OSExitCritical();	

	return xReturn;
}

/*
 * Returns the thread control block for the currently active task. In case
 * of an error the functions returns NULL.
 */
sys_tcb_t
*sys_arch_thread_current( void )
{
	OS_SR_SAVE_VAR
	sys_tcb_t      *p = tasks;
	sys_thread_t   pid = xTaskGetCurrentTaskHandle();

    // Enter Critical Section
    OSEnterCritical();
    while( ( p != NULL ) && ( p->pid != pid ) )
    {
        p = p->next;
    }
    // Exit Critical Section
    OSExitCritical();
    return p;
}

/*-----------------------------------------------------------------------------------*/
/*
  Returns a pointer to the per-thread sys_timeouts structure. In lwIP,
  each thread has a list of timeouts which is represented as a linked
  list of sys_timeout structures. The sys_timeouts structure holds a
  pointer to a linked list of timeouts. This function is called by
  the lwIP timeout scheduler and must not return a NULL value.

  In a single threaded sys_arch implementation, this function will
  simply return a pointer to a global sys_timeouts variable stored in
  the sys_arch module.
*/
struct sys_timeouts *
sys_arch_timeouts(void)
{
    sys_tcb_t      *ptask;

    ptask = sys_arch_thread_current(  );
    LWIP_ASSERT( "sys_arch_timeouts: ptask != NULL", ptask != NULL );
    return ptask != NULL ? &( ptask->timeouts ) : NULL;
}

/*-----------------------------------------------------------------------------------*/
/*-----------------------------------------------------------------------------------*/
// TBD
/*-----------------------------------------------------------------------------------*/
/*
 * Starts a new thread with priority "prio" that will begin its execution in the
 * function "thread()". The "arg" argument will be passed as an argument to the
 * thread() function. The argument "ssize" is the requested stack size for the
 * new thread. The id of the new thread is returned. Both the id and the
 * priority are system dependent.
 */
sys_thread_t
sys_thread_new(char *name, void ( *thread ) ( void *arg ), void *arg, int stacksize, int prio )
{
    sys_thread_t    thread_hdl = SYS_THREAD_NULL;
    int             i,j;
    sys_tcb_t      *p;

    /* We disable the FreeRTOS scheduler because it might be the case that the new
     * tasks gets scheduled inside the xTaskCreate function. To prevent this we
     * disable the scheduling. Note that this can happen although we have interrupts
     * disabled because xTaskCreate contains a call to taskYIELD( ).
     */
    UserEnterCritical( );

    p = tasks;
    i = 0;
    /* We are called the first time. Initialize it. */
    if( p == NULL )
    {
        p = (sys_tcb_t *)BRTOS_ALLOC( sizeof( sys_tcb_t ) );
        if( p != NULL )
        {
            tasks = p;
        }
    }
    else
    {
        /* First task already counter. */
        i++;
        /* Cycle to the end of the list. */
        while( p->next != NULL )
        {
            i++;
            p = p->next;
        }
        p->next = (sys_tcb_t *)BRTOS_ALLOC( sizeof( sys_tcb_t ) );
        p = p->next;
    }

    if( p != NULL )
    {
        /* Memory allocated. Initialize the data structure. */
        THREAD_INIT( p );

        /* Now q points to a free element in the list. */
        //if( xTaskCreate( thread, (const signed char *)name, stacksize, arg, prio, &p->pid ) == pdPASS )        
        if (InstallTask(thread, name, stacksize, prio, arg) == OK)
        {
        	for(j = 1;j<NUMBER_OF_TASKS;j++)
        	{
        		if (ContextTask[j].Priority == prio)
        		{
        			p->pid = &ContextTask[j];
            		thread_hdl = p->pid;
        		}
        	}
        }
        else
        {
        	BRTOS_DEALLOC( p );
        }
    }

    UserExitCritical( );
    return thread_hdl;
}
/*
  This optional function does a "fast" critical region protection and returns
  the previous protection level. This function is only called during very short
  critical regions. An embedded system which supports ISR-based drivers might
  want to implement this function by disabling interrupts. Task-based systems
  might want to implement this by using a mutex or disabling tasking. This
  function should support recursive calls from the same task or interrupt. In
  other words, sys_arch_protect() could be called while already protected. In
  that case the return value indicates that it is already protected.

  sys_arch_protect() is only required if your port is supporting an operating
  system.
*/
sys_prot_t sys_arch_protect(void)
{
	OS_SR_SAVE_VAR
    
	// Enter Critical Section
    OSEnterCritical();
	return CPU_SR;
}

/*
  This optional function does a "fast" set of critical region protection to the
  value specified by pval. See the documentation for sys_arch_protect() for
  more information. This function is only required if your port is supporting
  an operating system.
*/
void sys_arch_unprotect(sys_prot_t CPU_SR)
{
	// Enter Critical Section
    OSExitCritical();
}

/*
 * Prints an assertion messages and aborts execution.
 */
void
sys_assert( const char *msg )
{	

//FSL:only needed for debugging
#ifdef LWIP_DEBUG
	  printf(msg);
  	printf("\n\r");
#endif
	
  	(void)sys_arch_protect();
    for(;;)
    ;
}

void
sys_debug( const char *const fmt, ... )
{
	  /*FSL: same implementation as printf*/
#ifdef LWIP_DEBUG
    /*FSL: removed due to lack of space*/
    printf(fmt);
#endif
}