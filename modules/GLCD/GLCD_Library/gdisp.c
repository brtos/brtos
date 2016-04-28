/*
    ChibiOS/RT - Copyright (C) 2012
                 Joel Bodenmann aka Tectu <joel@unormal.org>

    This file is part of ChibiOS-LCD-Driver.

    ChibiOS-LCD-Driver is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    ChibiOS-LCD-Driver is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/**
 * @file    gdisp.c
 * @brief   GDISP Driver code.
 *
 * @addtogroup GDISP
 * @{
 */
#include "gdisp.h"

#ifndef _GDISP_C
#define _GDISP_C

//#if HAL_USE_GDISP || defined(__DOXYGEN__)

#ifdef GDISP_NEED_TEXT
	#include "gdisp_fonts.h"
#endif

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

#if GDISP_NEED_MULTITHREAD
	#if !CH_USE_MUTEXES
		#error "GDISP: CH_USE_MUTEXES must be defined in chconf.h because GDISP_NEED_MULTITHREAD is defined"
	#endif
#endif

#if GDISP_NEED_ASYNC
	#if !CH_USE_MAILBOXES || !CH_USE_MUTEXES || !CH_USE_SEMAPHORES
		#error "GDISP: CH_USE_MAILBOXES, CH_USE_SEMAPHORES and CH_USE_MUTEXES must be defined in chconf.h because GDISP_NEED_ASYNC is defined"
	#endif
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

#if GDISP_NEED_MULTITHREAD || GDISP_NEED_ASYNC
	static Mutex			gdispMutex;
#endif

#if GDISP_NEED_ASYNC
	#define GDISP_THREAD_STACK_SIZE	512		/* Just a number - not yet a reflection of actual use */
	#define GDISP_QUEUE_SIZE		8		/* We only allow a short queue */

	static Thread *			lldThread;
	static Mailbox			gdispMailbox;
	static msg_t 			gdispMailboxQueue[GDISP_QUEUE_SIZE];
	static Semaphore		gdispMsgsSem;
	static Mutex			gdispMsgsMutex;
	static gdisp_lld_msg_t	gdispMsgs[GDISP_QUEUE_SIZE];
	static WORKING_AREA(waGDISPThread, GDISP_THREAD_STACK_SIZE);
#endif

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

#if GDISP_NEED_ASYNC
	static msg_t GDISPThreadHandler(void *UNUSED(arg)) {
		gdisp_lld_msg_t	*pmsg;

		#if CH_USE_REGISTRY
			chRegSetThreadName("GDISPAsyncAPI");
		#endif

		while(1) {
			/* Wait for msg with work to do. */
			chMBFetch(&gdispMailbox, (msg_t *)&pmsg, TIME_INFINITE);

			/* OK - we need to obtain the mutex in case a synchronous operation is occurring */
			chMtxLock(&gdispMutex);
			GDISP_LLD(msgdispatch)(pmsg);
			chMtxUnlock();

			/* Mark the message as free */
			pmsg->action = GDISP_LLD_MSG_NOP;
			chSemSignal(&gdispMsgsSem);
		}
		return 0;
	}

	static gdisp_lld_msg_t *gdispAllocMsg(gdisp_msgaction_t action) {
		gdisp_lld_msg_t	*p;

		while(1) {		/* To be sure, to be sure */

			/* Wait for a slot */
			chSemWait(&gdispMsgsSem);

			/* Find the slot */
			chMtxLock(&gdispMsgsMutex);
			for(p=gdispMsgs; p < &gdispMsgs[GDISP_QUEUE_SIZE]; p++) {
				if (p->action == GDISP_LLD_MSG_NOP) {
					/* Allocate it */
					p->action = action;
					chMtxUnlock();
					return p;
				}
			}
			chMtxUnlock();

			/* Oops - none found, try again */
			chSemSignal(&gdispMsgsSem);
		}
	}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

#if GDISP_NEED_MULTITHREAD || defined(__DOXYGEN__)
	/**
	 * @brief   GDISP Driver initialization.
	 * @note    This function is NOT currently implicitly invoked by @p halInit().
	 *			It must be called manually.
	 *
	 * @init
	 */
	bool_t gdispInit(void) {
		bool_t	res;

		/* Initialise Mutex */
		chMtxInit(&gdispMutex);

		/* Initialise driver */
		chMtxLock(&gdispMutex);
		res = GDISP_LLD(init)();
		chMtxUnlock();

		return res;
	}
#elif GDISP_NEED_ASYNC
	bool_t gdispInit(void) {
		bool_t		res;
		unsigned	i;

		/* Mark all the Messages as free */
		for(i=0; i < GDISP_QUEUE_SIZE; i++)
			gdispMsgs[i].action = GDISP_LLD_MSG_NOP;

		/* Initialise our Mailbox, Mutex's and Counting Semaphore.
		 * 	A Mutex is required as well as the Mailbox and Thread because some calls have to be synchronous.
		 *	Synchronous calls get handled by the calling thread, asynchronous by our worker thread.
		 */
		chMBInit(&gdispMailbox, gdispMailboxQueue, sizeof(gdispMailboxQueue)/sizeof(gdispMailboxQueue[0]));
		chMtxInit(&gdispMutex);
		chMtxInit(&gdispMsgsMutex);
		chSemInit(&gdispMsgsSem, GDISP_QUEUE_SIZE);

		lldThread = chThdCreateStatic(waGDISPThread, sizeof(waGDISPThread), NORMALPRIO, GDISPThreadHandler, NULL);

		/* Initialise driver - synchronous */
		chMtxLock(&gdispMutex);
		res = GDISP_LLD(init)();
		chMtxUnlock();

		return res;
	}
#endif

#if GDISP_NEED_MULTITHREAD || defined(__DOXYGEN__)
	/**
	 * @brief   Test if the GDISP engine is currently drawing.
	 * @note    This function will always return FALSE if
	 * 			 GDISP_NEED_ASYNC is not defined.
	 *
	 * @init
	 */
	bool_t gdispIsBusy(void) {
		return FALSE;
	}
#elif GDISP_NEED_ASYNC
	bool_t gdispIsBusy(void) {
		return chMBGetUsedCountI(&gdispMailbox) != FALSE;
	}
#endif

#if GDISP_NEED_MULTITHREAD || defined(__DOXYGEN__)
	/**
	 * @brief   Clear the display to the specified color.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] color The color to use when clearing the screen
	 *
	 * @api
	 */
	void gdispClear(color_t color) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(clear)(color);
		chMtxUnlock();
	}
#elif GDISP_NEED_ASYNC
	void gdispClear(color_t color) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_CLEAR);
		p->clear.color = color;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif

#if GDISP_NEED_MULTITHREAD || defined(__DOXYGEN__)
	/**
	 * @brief   Set a pixel in the specified color.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x,y   The position to set the pixel.
	 * @param[in] color The color to use
	 *
	 * @api
	 */
	void gdispDrawPixel(coord_t x, coord_t y, color_t color) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(drawpixel)(x, y, color);
		chMtxUnlock();
	}
#elif GDISP_NEED_ASYNC
	void gdispDrawPixel(coord_t x, coord_t y, color_t color) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_DRAWPIXEL);
		p->drawpixel.x = x;
		p->drawpixel.y = y;
		p->drawpixel.color = color;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif
	
#if GDISP_NEED_MULTITHREAD || defined(__DOXYGEN__)
	/**
	 * @brief   Draw a line.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x0,y0   The start position
	 * @param[in] x1,y1   The end position
	 * @param[in] color The color to use
	 *
	 * @api
	 */
	void gdispDrawLine(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(drawline)(x0, y0, x1, y1, color);
		chMtxUnlock();
	}
#elif GDISP_NEED_ASYNC
	void gdispDrawLine(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_DRAWLINE);
		p->drawline.x0 = x0;
		p->drawline.y0 = y0;
		p->drawline.x1 = x1;
		p->drawline.y1 = y1;
		p->drawline.color = color;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif
	
#if GDISP_NEED_MULTITHREAD || defined(__DOXYGEN__)
	/**
	 * @brief   Fill an area with a color.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x0,y0   The start position
	 * @param[in] cx,cy   The size of the box (outside dimensions)
	 * @param[in] color   The color to use
	 *
	 * @api
	 */
	void gdispFillArea(coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(fillarea)(x, y, cx, cy, color);
		chMtxUnlock();
	}
#elif GDISP_NEED_ASYNC
	void gdispFillArea(coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_FILLAREA);
		p->fillarea.x = x;
		p->fillarea.y = y;
		p->fillarea.cx = cx;
		p->fillarea.cy = cy;
		p->fillarea.color = color;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif
	
#if GDISP_NEED_MULTITHREAD || defined(__DOXYGEN__)
	/**
	 * @brief   Fill an area using the supplied bitmap.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 * @details The bitmap is in the pixel format specified by the low level driver
	 * @note	If a packed pixel format is used and the width doesn't
	 *			match a whole number of bytes, the next line will start on a
	 *			non-byte boundary (no end-of-line padding).
	 * @note	If GDISP_NEED_ASYNC is defined then the buffer must be static
	 * 			or at least retained until this call has finished the blit. You can
	 * 			tell when all graphics drawing is finished by @p gdispIsBusy() going FALSE.
	 *
	 * @param[in] x,y     The start position
	 * @param[in] cx,cy   The size of the filled area
	 * @param[in] buffer  The bitmap in the driver's pixel format.
	 *
	 * @api
	 */
	void gdispBlitAreaEx(coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t srcx, coord_t srcy, coord_t srccx, const pixel_t *buffer) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(blitareaex)(x, y, cx, cy, srcx, srcy, srccx, buffer);
		chMtxUnlock();
	}
#elif GDISP_NEED_ASYNC
	void gdispBlitAreaEx(coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t srcx, coord_t srcy, coord_t srccx, const pixel_t *buffer) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_BLITAREA);
		p->blitarea.x = x;
		p->blitarea.y = y;
		p->blitarea.cx = cx;
		p->blitarea.cy = cy;
		p->blitarea.srcx = srcx;
		p->blitarea.srcy = srcy;
		p->blitarea.srccx = srccx;
		p->blitarea.buffer = buffer;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif
	
#if (GDISP_NEED_CLIP && GDISP_NEED_MULTITHREAD) || defined(__DOXYGEN__)
	/**
	 * @brief   Clip all drawing to the defined area.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x,y     The start position
	 * @param[in] cx,cy   The size of the clip area
	 *
	 * @api
	 */
	void gdispSetClip(coord_t x, coord_t y, coord_t cx, coord_t cy) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(setclip)(x, y, cx, cy);
		chMtxUnlock();
	}
#elif GDISP_NEED_CLIP && GDISP_NEED_ASYNC
	void gdispSetClip(coord_t x, coord_t y, coord_t cx, coord_t cy) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_SETCLIP);
		p->setclip.x = x;
		p->setclip.y = y;
		p->setclip.cx = cx;
		p->setclip.cy = cy;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif

#if (GDISP_NEED_CIRCLE && GDISP_NEED_MULTITHREAD) || defined(__DOXYGEN__)
	/**
	 * @brief   Draw a circle.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x0,y0   The center of the circle
	 * @param[in] radius  The radius of the circle
	 * @param[in] color   The color to use
	 *
	 * @api
	 */
	void gdispDrawCircle(coord_t x, coord_t y, coord_t radius, color_t color) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(drawcircle)(x, y, radius, color);
		chMtxUnlock();
	}
#elif GDISP_NEED_CIRCLE && GDISP_NEED_ASYNC
	void gdispDrawCircle(coord_t x, coord_t y, coord_t radius, color_t color) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_DRAWCIRCLE);
		p->drawcircle.x = x;
		p->drawcircle.y = y;
		p->drawcircle.radius = radius;
		p->drawcircle.color = color;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif
	
#if (GDISP_NEED_CIRCLE && GDISP_NEED_MULTITHREAD) || defined(__DOXYGEN__)
	/**
	 * @brief   Draw a filled circle.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x0,y0   The center of the circle
	 * @param[in] radius  The radius of the circle
	 * @param[in] color   The color to use
	 *
	 * @api
	 */
	void gdispFillCircle(coord_t x, coord_t y, coord_t radius, color_t color) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(fillcircle)(x, y, radius, color);
		chMtxUnlock();
	}
#elif GDISP_NEED_CIRCLE && GDISP_NEED_ASYNC
	void gdispFillCircle(coord_t x, coord_t y, coord_t radius, color_t color) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_FILLCIRCLE);
		p->fillcircle.x = x;
		p->fillcircle.y = y;
		p->fillcircle.radius = radius;
		p->fillcircle.color = color;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif

#if (GDISP_NEED_ELLIPSE && GDISP_NEED_MULTITHREAD) || defined(__DOXYGEN__)
	/**
	 * @brief   Draw an ellipse.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x0,y0   The center of the ellipse
	 * @param[in] a,b     The dimensions of the ellipse
	 * @param[in] color   The color to use
	 *
	 * @api
	 */
	void gdispDrawEllipse(coord_t x, coord_t y, coord_t a, coord_t b, color_t color) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(drawellipse)(x, y, a, b, color);
		chMtxUnlock();
	}
#elif GDISP_NEED_ELLIPSE && GDISP_NEED_ASYNC
	void gdispDrawEllipse(coord_t x, coord_t y, coord_t a, coord_t b, color_t color) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_DRAWELLIPSE);
		p->drawellipse.x = x;
		p->drawellipse.y = y;
		p->drawellipse.a = a;
		p->drawellipse.b = b;
		p->drawellipse.color = color;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif
	
#if (GDISP_NEED_ELLIPSE && GDISP_NEED_MULTITHREAD) || defined(__DOXYGEN__)
	/**
	 * @brief   Draw a filled ellipse.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x0,y0   The center of the ellipse
	 * @param[in] a,b     The dimensions of the ellipse
	 * @param[in] color   The color to use
	 *
	 * @api
	 */
	void gdispFillEllipse(coord_t x, coord_t y, coord_t a, coord_t b, color_t color) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(fillellipse)(x, y, a, b, color);
		chMtxUnlock();
	}
#elif GDISP_NEED_ELLIPSE && GDISP_NEED_ASYNC
	void gdispFillEllipse(coord_t x, coord_t y, coord_t a, coord_t b, color_t color) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_FILLELLIPSE);
		p->fillellipse.x = x;
		p->fillellipse.y = y;
		p->fillellipse.a = a;
		p->fillellipse.b = b;
		p->fillellipse.color = color;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif

#if (GDISP_NEED_ARC && GDISP_NEED_MULTITHREAD) || defined(__DOXYGEN__)
	/**
	 * @brief   Draw an arc.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x,y     The center of the arc circle
	 * @param[in] radius  The radius of the arc circle
	 * @param[in] startangle, endangle  The start and end angle in degrees (0 to 359)
	 * @param[in] color   The color to use
	 *
	 * @api
	 */
	void gdispDrawArc(coord_t x, coord_t y, coord_t radius, coord_t startangle, coord_t endangle, color_t color) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(drawarc)(x, y, radius, startangle, endangle, color);
		chMtxUnlock();
	}
#elif GDISP_NEED_ARC && GDISP_NEED_ASYNC
	void gdispDrawArc(coord_t x, coord_t y, coord_t radius, coord_t startangle, coord_t endangle, color_t color) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_DRAWARC);
		p->drawarc.x = x;
		p->drawarc.y = y;
		p->drawarc.radius = radius;
		p->drawarc.startangle = startangle;
		p->drawarc.endangle = endangle;
		p->drawarc.color = color;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif

#if (GDISP_NEED_ARC && GDISP_NEED_MULTITHREAD) || defined(__DOXYGEN__)
	/**
	 * @brief   Draw a filled arc.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x,y     The center of the arc circle
	 * @param[in] radius  The radius of the arc circle
	 * @param[in] startangle, endangle  The start and end angle in degrees (0 to 359)
	 * @param[in] color   The color to use
	 *
	 * @api
	 */
	void gdispFillArc(coord_t x, coord_t y, coord_t radius, coord_t startangle, coord_t endangle, color_t color) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(fillarc)(x, y, radius, startangle, endangle, color);
		chMtxUnlock();
	}
#elif GDISP_NEED_ARC && GDISP_NEED_ASYNC
	void gdispFillArc(coord_t x, coord_t y, coord_t radius, coord_t startangle, coord_t endangle, color_t color) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_FILLARC);
		p->fillarc.x = x;
		p->fillarc.y = y;
		p->fillarc.radius = radius;
		p->fillarc.startangle = startangle;
		p->fillarc.endangle = endangle;
		p->fillarc.color = color;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif

#if (GDISP_NEED_TEXT && GDISP_NEED_MULTITHREAD) || defined(__DOXYGEN__)
	/**
	 * @brief   Draw a text character.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x,y     The position for the text
	 * @param[in] c       The character to draw
	 * @param[in] color   The color to use
	 *
	 * @api
	 */
	void gdispDrawChar(coord_t x, coord_t y, char c, font_t font, color_t color) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(drawchar)(x, y, c, font, color);
		chMtxUnlock();
	}
#elif GDISP_NEED_TEXT && GDISP_NEED_ASYNC
	void gdispDrawChar(coord_t x, coord_t y, char c, font_t font, color_t color) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_DRAWCHAR);
		p->drawchar.x = x;
		p->drawchar.y = y;
		p->drawchar.c = c;
		p->drawchar.font = font;
		p->drawchar.color = color;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif

#if (GDISP_NEED_TEXT && GDISP_NEED_MULTITHREAD) || defined(__DOXYGEN__)
	/**
	 * @brief   Draw a text character with a filled background.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x,y     The position for the text
	 * @param[in] c       The character to draw
	 * @param[in] color   The color to use
	 * @param[in] bgcolor The background color to use
	 *
	 * @api
	 */
	void gdispFillChar(coord_t x, coord_t y, char c, font_t font, color_t color, color_t bgcolor) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(fillchar)(x, y, c, font, color, bgcolor);
		chMtxUnlock();
	}
#elif GDISP_NEED_TEXT && GDISP_NEED_ASYNC
	void gdispFillChar(coord_t x, coord_t y, char c, font_t font, color_t color, color_t bgcolor) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_FILLCHAR);
		p->fillchar.x = x;
		p->fillchar.y = y;
		p->fillchar.c = c;
		p->fillchar.font = font;
		p->fillchar.color = color;
		p->fillchar.bgcolor = bgcolor;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif
	
#if (GDISP_NEED_PIXELREAD && (GDISP_NEED_MULTITHREAD || GDISP_NEED_ASYNC)) || defined(__DOXYGEN__)
	/**
	 * @brief   Get the color of a pixel.
	 * @return  The color of the pixel.
	 *
	 * @param[in] x,y     The position of the pixel
	 *
	 * @api
	 */
	color_t gdispGetPixelColor(coord_t x, coord_t y) {
		color_t		c;

		/* Always synchronous as it must return a value */
		chMtxLock(&gdispMutex);
		c = GDISP_LLD(getpixelcolor)(x, y);
		chMtxUnlock();

		return c;
	}
#endif

#if (GDISP_NEED_SCROLL && GDISP_NEED_MULTITHREAD) || defined(__DOXYGEN__)
	/**
	 * @brief   Scroll vertically a section of the screen.
	 * @note    Optional.
	 * @note    If lines is >= cy, it is equivelent to a area fill with bgcolor.
	 *
	 * @param[in] x, y     The start of the area to be scrolled
	 * @param[in] cx, cy   The size of the area to be scrolled
	 * @param[in] lines    The number of lines to scroll (Can be positive or negative)
	 * @param[in] bgcolor  The color to fill the newly exposed area.
	 *
	 * @api
	 */
	void gdispVerticalScroll(coord_t x, coord_t y, coord_t cx, coord_t cy, int lines, color_t bgcolor) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(verticalscroll)(x, y, cx, cy, lines, bgcolor);
		chMtxUnlock();
	}
#elif GDISP_NEED_SCROLL && GDISP_NEED_ASYNC
	void gdispVerticalScroll(coord_t x, coord_t y, coord_t cx, coord_t cy, int lines, color_t bgcolor) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_VERTICALSCROLL);
		p->verticalscroll.x = x;
		p->verticalscroll.y = y;
		p->verticalscroll.cx = cx;
		p->verticalscroll.cy = cy;
		p->verticalscroll.lines = lines;
		p->verticalscroll.bgcolor = bgcolor;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif

#if (GDISP_NEED_CONTROL && GDISP_NEED_MULTITHREAD) || defined(__DOXYGEN__)
	/**
	 * @brief   Set the power mode for the display.
	 * @pre     The GDISP unit must have been initialised using @p gdispInit().
	 * @note    Depending on the hardware implementation this function may not
	 *          support some codes. They will be ignored.
	 *
	 * @param[in] powerMode The power mode to use
	 *
	 * @api
	 */
	void gdispControl(unsigned what, void *value) {
		chMtxLock(&gdispMutex);
		GDISP_LLD(control)(what, value);
		chMtxUnlock();
	}
#elif GDISP_NEED_CONTROL && GDISP_NEED_ASYNC
	void gdispControl(unsigned what, void *value) {
		gdisp_lld_msg_t *p = gdispAllocMsg(GDISP_LLD_MSG_CONTROL);
		p->control.what = what;
		p->control.value = value;
		chMBPost(&gdispMailbox, (msg_t)p, TIME_INFINITE);
	}
#endif

#if (GDISP_NEED_QUERY && (GDISP_NEED_MULTITHREAD || GDISP_NEED_ASYNC)) || defined(__DOXYGEN__)
	/**
	 * @brief   Query a property of the display.
	 * @pre     The GDISP unit must have been initialised using @p gdispInit().
	 * @note    The result must be typecast to the correct type.
	 * @note    An uunsupported query will return (void *)-1.
	 *
	 * @param[in] what		What to query
	 *
	 * @api
	 */
	void *gdispQuery(unsigned what) {
		void *res;

		chMtxLock(&gdispMutex);
		res = GDISP_LLD(query)(what);
		chMtxUnlock();
		return res;
	}
#endif

/*===========================================================================*/
/* High Level Driver Routines.                                               */
/*===========================================================================*/

/**
 * @brief   Draw a rectangular box.
 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
 *
 * @param[in] x0,y0   The start position
 * @param[in] cx,cy   The size of the box (outside dimensions)
 * @param[in] color   The color to use
 * @param[in] filled  Should the box should be filled
 *
 * @api
 */
void gdispDrawBox(coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color) {
	/* No mutex required as we only call high level functions which have their own mutex */
	coord_t	x1, y1;

	x1 = x+cx-1;
	y1 = y+cy-1;

	if (cx > 2) {
		if (cy >= 1) {
			gdispDrawLine(x, y, x1, y, color);
			if (cy >= 2) {
				gdispDrawLine(x, y1, x1, y1, color);
				if (cy > 2) {
					gdispDrawLine(x, y+1, x, y1-1, color);
					gdispDrawLine(x1, y+1, x1, y1-1, color);
				}
			}
		}
	} else if (cx == 2) {
		gdispDrawLine(x, y, x, y1, color);
		gdispDrawLine(x1, y, x1, y1, color);
	} else if (cx == 1) {
		gdispDrawLine(x, y, x, y1, color);
	}
}


#if GDISP_NEED_TEXT || defined(__DOXYGEN__)
	/**
	 * @brief   Draw a text string.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x,y     The position for the text
	 * @param[in] str     The string to draw
	 * @param[in] color   The color to use
	 *
	 * @api
	 */
	void gdispDrawString(coord_t x, coord_t y, const char *str, font_t font, color_t color) {
		/* No mutex required as we only call high level functions which have their own mutex */
		coord_t		w, p;
		char		c;
		int			first;
		
		first = 1;
		p = font->charPadding * font->xscale;
		while(*str) {
			/* Get the next printable character */
			c = *str++;
			w = _getCharWidth(font, c) * font->xscale;
			if (!w) continue;
			
			/* Handle inter-character padding */
			if (p) {
				if (!first)
					x += p;
				else
					first = 0;
			}
			
			/* Print the character */
			gdispDrawChar(x, y, c, font, color);
			x += w;
		}
	}

	void gdispDrawStringInv(coord_t x, coord_t y, const char *str, font_t font, color_t color) {
		/* No mutex required as we only call high level functions which have their own mutex */
		coord_t		w, p;
		char		c;
		int			first;

		first = 1;
		p = font->charPadding * font->xscale;
		while(*str) {
			/* Get the next printable character */
			c = *str++;
			w = _getCharWidth(font, c) * font->xscale;
			if (!w) continue;

			/* Handle inter-character padding */
			if (p) {
				if (!first)
					y -= p;
				else
					first = 0;
			}

			/* Print the character */
			gdispDrawCharInv(x, y, c, font, color);
			y -= w;
		}
	}
#endif
	
#if GDISP_NEED_TEXT || defined(__DOXYGEN__)
	/**
	 * @brief   Draw a text string.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 *
	 * @param[in] x,y     The position for the text
	 * @param[in] str     The string to draw
	 * @param[in] color   The color to use
	 * @param[in] bgcolor The background color to use
	 *
	 * @api
	 */
	void gdispFillString(coord_t x, coord_t y, const char *str, font_t font, color_t color, color_t bgcolor) {
		/* No mutex required as we only call high level functions which have their own mutex */
		coord_t		w, h, p;
		char		c;
		int			first;
		
		first = 1;
		h = font->height * font->yscale;
		p = font->charPadding * font->xscale;
		while(*str) {
			/* Get the next printable character */
			c = *str++;
			w = _getCharWidth(font, c) * font->xscale;
			if (!w) continue;
			
			/* Handle inter-character padding */
			if (p) {
				if (!first) {
					gdispFillArea(x, y, p, h, bgcolor);
					x += p;
				} else
					first = 0;
			}

			/* Print the character */
			gdispFillChar(x, y, c, font, color, bgcolor);
			x += w;
		}
	}
#endif
	
#if GDISP_NEED_TEXT || defined(__DOXYGEN__)
	/**
	 * @brief   Draw a text string verticly centered within the specified box.
	 * @pre     The GDISP unit must be in powerOn or powerSleep mode.
	 * @note    The entire box is filled
	 *
	 * @param[in] x,y     The position for the text (need to define top-right or base-line - check code)
	 * @param[in] str     The string to draw
	 * @param[in] color   The color to use
	 * @param[in] bgcolor The background color to use
	 * @param[in] justify Justify the text left, center or right within the box
	 *
	 * @api
	 */
	void gdispFillStringBox(coord_t x, coord_t y, coord_t cx, coord_t cy, const char* str, font_t font, color_t color, color_t bgcolor, justify_t justify) {
		/* No mutex required as we only call high level functions which have their own mutex */
		coord_t		w, h, p, ypos, xpos;
		char		c;
		int			first;
		const char *rstr;
		
		h = font->height * font->yscale;
		p = font->charPadding * font->xscale;

		/* Oops - font too large for the area */
		if (h > cy) return;

		/* See if we need to fill above the font */
		ypos = (cy - h + 1)/2;
		if (ypos > 0) {
			gdispFillArea(x, y, cx, ypos, bgcolor);
			y += ypos;
			cy -= ypos;
		}
		
		/* See if we need to fill below the font */
		ypos = cy - h;
		if (ypos > 0) {
			gdispFillArea(x, y+cy-ypos, cx, ypos, bgcolor);
			cy -= ypos;
		}
		
		/* get the start of the printable string and the xpos */
		switch(justify) {
		case justifyCenter:
			/* Get the length of the entire string */
			w = gdispGetStringWidth(str, font);
			if (w <= cx)
				xpos = x + (cx - w)/2;
			else {
				/* Calculate how much of the string we need to get rid of */
				ypos = (w - cx)/2;
				xpos = 0;
				first = 1;
				while(*str) {
					/* Get the next printable character */
					c = *str++;
					w = _getCharWidth(font, c) * font->xscale;
					if (!w) continue;
					
					/* Handle inter-character padding */
					if (p) {
						if (!first) {
							xpos += p;
							if (xpos > ypos) break;
						} else
							first = 0;
					}

					/* Print the character */
					xpos += w;
					if (xpos > ypos) break;
				}
				xpos = ypos - xpos + x;
			}
			break;
		case justifyRight:
			/* Find the end of the string */
			for(rstr = str; *str; str++);
			xpos = x+cx - 2;
			first = 1;
			for(str--; str >= rstr; str--) {
				/* Get the next printable character */
				c = *str;
				w = _getCharWidth(font, c) * font->xscale;
				if (!w) continue;
				
				/* Handle inter-character padding */
				if (p) {
					if (!first) {
						if (xpos - p < x) break;
						xpos -= p;
					} else
						first = 0;
				}

				/* Print the character */
				if (xpos - w < x) break;
				xpos -= w;
			}
			str++;
			break;
		case justifyLeft:
			/* Fall through */
		default:
			xpos = x+1;
			break;
		}
		
		/* Fill any space to the left */
		if (x < xpos)
			gdispFillArea(x, y, xpos-x, cy, bgcolor);
		
		/* Print characters until we run out of room */
		first = 1;
		while(*str) {
			/* Get the next printable character */
			c = *str++;
			w = _getCharWidth(font, c) * font->xscale;
			if (!w) continue;
			
			/* Handle inter-character padding */
			if (p) {
				if (!first) {
					if (xpos + p > x+cx) break;
					gdispFillArea(xpos, y, p, cy, bgcolor);
					xpos += p;
				} else
					first = 0;
			}

			/* Print the character */
			if (xpos + w > x+cx) break;
			gdispFillChar(xpos, y, c, font, color, bgcolor);
			xpos += w;
		}
		
		/* Fill any space to the right */
		if (xpos < x+cx)
			gdispFillArea(xpos, y, x+cx-xpos, cy, bgcolor);
	}
#endif
	
#if GDISP_NEED_TEXT || defined(__DOXYGEN__)
	/**
	 * @brief   Get a metric of a font.
	 * @return  The metric requested in pixels.
	 *
	 * @param[in] font    The font to test
	 * @param[in] metric  The metric to measure
	 *
	 * @api
	 */
	coord_t gdispGetFontMetric(font_t font, fontmetric_t metric) {
		/* No mutex required as we only read static data */
		switch(metric) {
		case fontHeight:			return font->height * font->yscale;
		case fontDescendersHeight:	return font->descenderHeight * font->yscale;
		case fontLineSpacing:		return font->lineSpacing * font->yscale;
		case fontCharPadding:		return font->charPadding * font->xscale;
		case fontMinWidth:			return font->minWidth * font->xscale;
		case fontMaxWidth:			return font->maxWidth * font->xscale;
		}
		return 0;
	}
#endif
	
#if GDISP_NEED_TEXT || defined(__DOXYGEN__)
	/**
	 * @brief   Get the pixel width of a character.
	 * @return  The width of the character in pixels. Does not include any between character padding.
	 *
	 * @param[in] c       The character to draw
	 * @param[in] font    The font to use
	 *
	 * @api
	 */
	coord_t gdispGetCharWidth(char c, font_t font) {
		/* No mutex required as we only read static data */
		return _getCharWidth(font, c) * font->xscale;
	}
#endif
	
#if GDISP_NEED_TEXT || defined(__DOXYGEN__)
	/**
	 * @brief   Get the pixel width of a string.
	 * @return  The width of the string in pixels.
	 *
	 * @param[in] str     The string to measure
	 * @param[in] font    The font to use
	 *
	 * @api
	 */
	coord_t gdispGetStringWidth(const char* str, font_t font) {
		/* No mutex required as we only read static data */
		coord_t		w, p, x;
		char		c;
		int			first;
		
		first = 1;
		x = 0;
		p = font->charPadding * font->xscale;
		while(*str) {
			/* Get the next printable character */
			c = *str++;
			w = _getCharWidth(font, c)  * font->xscale;
			if (!w) continue;
			
			/* Handle inter-character padding */
			if (p) {
				if (!first)
					x += p;
				else
					first = 0;
			}
			
			/* Add the character width */
			x += w;
		}
		return x;
	}

	coord_t gdispGetStringHeight(font_t font)
	{
		return (coord_t)font->height;
	}
#endif

#if (!defined(gdispPackPixels) && !defined(GDISP_PIXELFORMAT_CUSTOM)) || defined(__DOXYGEN__)
	/**
	 * @brief   Pack a pixel into a pixel buffer.
	 * @note    This function performs no buffer boundary checking
	 *			regardless of whether GDISP_NEED_CLIPPING has been specified.
	 *
	 * @param[in] buf		The buffer to put the pixel in
	 * @param[in] cx		The width of a pixel line
	 * @param[in] x, y		The location of the pixel to place
	 * @param[in] color		The color to put into the buffer
	 *
	 * @api
	 */
	void gdispPackPixels(pixel_t *buf, coord_t cx, coord_t x, coord_t y, color_t color) {
		/* No mutex required as we only read static data */
		#if defined(GDISP_PIXELFORMAT_RGB888)
			#error "GDISP: Packed pixels not supported yet"
		#elif defined(GDISP_PIXELFORMAT_RGB444)
			#error "GDISP: Packed pixels not supported yet"
		#elif defined(GDISP_PIXELFORMAT_RGB666)
			#error "GDISP: Packed pixels not supported yet"
		#elif
			#error "GDISP: Unsupported packed pixel format"
		#endif
	}
#endif

//#endif /* HAL_USE_GDISP */

#endif /* _GDISP_C */
/** @} */
