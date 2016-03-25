/**
* \file OS_types.h
* \brief BRTOS types declaration
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
*                                            OS Types Header
*
*
*   Author: Gustavo Weber Denardin
*   Revision: 1.0
*   Date:     20/03/2009
*
*********************************************************************************************************/

#ifndef OS_TYPES_H
#define OS_TYPES_H

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
/////    Portable OS Types                             /////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#if (defined __CWCC__)
#define HAVE_STDINT 0
#else
#define HAVE_STDINT 1
#endif

#if HAVE_STDINT == 1
#include "stdint.h"
#include "stdbool.h"
#else
typedef unsigned char      bool;
typedef unsigned char      uint8_t;
typedef signed char        int8_t;
typedef unsigned short int uint16_t;
typedef signed short int   int16_t;
typedef unsigned long      uint32_t;
typedef signed long        int32_t;
#endif

/* for compatibility purpose */
typedef char               CHAR8;
#if !WINNT
typedef bool     		   BOOLEAN;
#endif
typedef uint8_t      	   INT8U;
typedef int8_t             INT8S;
typedef uint16_t           INT16U;
typedef int16_t            INT16S;
typedef uint32_t      	   INT32U;
typedef int32_t            INT32S;

/* for portability purpose */
typedef unsigned int       stack_pointer_t;	
typedef unsigned int       uint_t;

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

#endif
