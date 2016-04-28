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
 * @file    gdisp.h
 * @brief   GDISP Graphic Driver subsystem header file.
 *
 * @addtogroup GDISP
 * @{
 */
#ifndef _GDISP_H
#define _GDISP_H

//#if HAL_USE_GDISP || defined(__DOXYGEN__)


/*===========================================================================*/
/* Driver constants.														 */
/*===========================================================================*/

/**
 * @brief   Some basic colors
 */
#define White			HTML2COLOR(0xFFFFFF)
#define Black			HTML2COLOR(0x000000)
#define Gray			HTML2COLOR(0x808080)
#define Grey			Gray
#define Blue			HTML2COLOR(0x0000FF)
#define Red				HTML2COLOR(0xFF0000)
#define Fuchsia			HTML2COLOR(0xFF00FF)
#define Magenta			Fuchsia
#define Green			HTML2COLOR(0x008000)
#define Yellow			HTML2COLOR(0xFFFF00)
#define Aqua			HTML2COLOR(0x00FFFF)
#define Cyan			Aqua
#define Lime			HTML2COLOR(0x00FF00)
#define Maroon			HTML2COLOR(0x800000)
#define Navy			HTML2COLOR(0x000080)
#define Olive			HTML2COLOR(0x808000)
#define Purple			HTML2COLOR(0x800080)
#define Silver			HTML2COLOR(0xC0C0C0)
#define Teal			HTML2COLOR(0x008080)
#define Orange			HTML2COLOR(0xFFA500)
#define Pink			HTML2COLOR(0xFFC0CB)
#define SkyBlue			HTML2COLOR(0x87CEEB)

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    GDISP more complex functionality to be compiled
 * @{
 */
	/**
	 * @brief   Do the drawing functions need to be thread-safe.
	 * @details	Defaults to FALSE
	 * @note	Both GDISP_NEED_MULTITHREAD and GDISP_NEED_ASYNC make
	 * 			the gdisp API thread-safe.
	 * @note	This is more efficient than GDISP_NEED_ASYNC as it only
	 * 			requires a context switch if something else is already
	 * 			drawing.
	 */
	#ifndef GDISP_NEED_MULTITHREAD
		#define GDISP_NEED_MULTITHREAD	FALSE
	#endif

	/**
	 * @brief   Use asynchronous calls (multi-thread safe).
	 * @details	Defaults to FALSE
	 * @note	Both GDISP_NEED_MULTITHREAD and GDISP_NEED_ASYNC make
	 * 			the gdisp API thread-safe.
	 * @note	Turning this on adds two context switches per transaction
	 *			so it can significantly slow graphics drawing.
	 */
	#ifndef GDISP_NEED_ASYNC
		#define GDISP_NEED_ASYNC	FALSE
	#endif
/** @} */

#if GDISP_NEED_MULTITHREAD && GDISP_NEED_ASYNC
	#error "GDISP: Only one of GDISP_NEED_MULTITHREAD and GDISP_NEED_ASYNC should be defined."
#endif

#if GDISP_NEED_ASYNC
	/* Messaging API is required for Async Multi-Thread */
	#undef GDISP_NEED_MSGAPI
	#define GDISP_NEED_MSGAPI	TRUE
#endif

/*===========================================================================*/
/* Low Level Driver details and error checks.                                */
/*===========================================================================*/

/* Include the low level driver information */
#include "gdisp_lld.h"

/*===========================================================================*/
/* Type definitions                                                          */
/*===========================================================================*/

/**
 * @brief   Type for the text justification.
 */
typedef enum justify {justifyLeft, justifyCenter, justifyRight} justify_t;
/**
 * @brief   Type for the font metric.
 */
typedef enum fontmetric {fontHeight, fontDescendersHeight, fontLineSpacing, fontCharPadding, fontMinWidth, fontMaxWidth} fontmetric_t;

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if GDISP_NEED_TEXT || defined(__DOXYGEN__)
/**
 * @brief   Predefined fonts.
 */
extern const struct font fontSmall;
extern const struct font fontSmallDouble;
extern const struct font fontSmallNarrow;
extern const struct font fontLarger;
extern const struct font fontLargerDouble;
extern const struct font fontLargerNarrow;
extern const struct font fontUI1;
extern const struct font fontUI1Double;
extern const struct font fontUI1Narrow;
extern const struct font fontUI2;
extern const struct font fontUI2Double;
extern const struct font fontUI2Narrow;
extern const struct font fontLargeNumbers;
extern const struct font fontLargeNumbersDouble;
extern const struct font fontLargeNumbersNarrow;
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if GDISP_NEED_MULTITHREAD || GDISP_NEED_ASYNC

	/* Base Functions */
	bool_t gdispInit(void);
	bool_t gdispIsBusy(void);

	/* Drawing Functions */
	void gdispClear(color_t color);
	void gdispDrawPixel(coord_t x, coord_t y, color_t color);
	void gdispDrawLine(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color);
	void gdispFillArea(coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color);
	void gdispBlitAreaEx(coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t srcx, coord_t srcy, coord_t srccx, const pixel_t *buffer);

	/* Clipping Functions */
	#if GDISP_NEED_CLIP
	void gdispSetClip(coord_t x, coord_t y, coord_t cx, coord_t cy);
	#endif

	/* Circle Functions */
	#if GDISP_NEED_CIRCLE
	void gdispDrawCircle(coord_t x, coord_t y, coord_t radius, color_t color);
	void gdispFillCircle(coord_t x, coord_t y, coord_t radius, color_t color);
	#endif
	
	/* Ellipse Functions */
	#if GDISP_NEED_ELLIPSE
	void gdispDrawEllipse(coord_t x, coord_t y, coord_t a, coord_t b, color_t color);
	void gdispFillEllipse(coord_t x, coord_t y, coord_t a, coord_t b, color_t color);
	#endif

	/* Arc Functions */
	#if GDISP_NEED_ARC
	void gdispDrawArc(coord_t x, coord_t y, coord_t radius, coord_t startangle, coord_t endangle, color_t color);
	void gdispFillArc(coord_t x, coord_t y, coord_t radius, coord_t startangle, coord_t endangle, color_t color);
	#endif

	/* Basic Text Rendering Functions */
	#if GDISP_NEED_TEXT
	void gdispDrawChar(coord_t x, coord_t y, char c, font_t font, color_t color);
	void gdispDrawCharInv(coord_t x, coord_t y, char c, font_t font, color_t color);
	void gdispFillChar(coord_t x, coord_t y, char c, font_t font, color_t color, color_t bgcolor);
	#endif
	
	/* Read a pixel Function */
	#if GDISP_NEED_PIXELREAD
	color_t gdispGetPixelColor(coord_t x, coord_t y);
	#endif

	/* Scrolling Function - clears the area scrolled out */
	#if GDISP_NEED_SCROLL
	void gdispVerticalScroll(coord_t x, coord_t y, coord_t cx, coord_t cy, int lines, color_t bgcolor);
	#endif

	/* Set driver specific control */
	#if GDISP_NEED_CONTROL
	void gdispControl(unsigned what, void *value);
	#endif

	/* Query driver specific data */
	#if GDISP_NEED_CONTROL
	void *gdispQuery(unsigned what);
	#endif

#else

	/* The same as above but use the low level driver directly if no multi-thread support is needed */
	#define gdispInit(gdisp)									GDISP_LLD(init)()
	#define gdispIsBusy()										FALSE
	#define gdispClear(color)									GDISP_LLD(clear)(color)
	#define gdispDrawPixel(x, y, color)							GDISP_LLD(drawpixel)(x, y, color)
	#define gdispDrawLine(x0, y0, x1, y1, color)				GDISP_LLD(drawline)(x0, y0, x1, y1, color)
	#define gdispFillArea(x, y, cx, cy, color)					GDISP_LLD(fillarea)(x, y, cx, cy, color)
	#define gdispBlitAreaEx(x, y, cx, cy, sx, sy, scx, buf)		GDISP_LLD(blitareaex)(x, y, cx, cy, sx, sy, scx, buf)
	#define gdispSetClip(x, y, cx, cy)							GDISP_LLD(setclip)(x, y, cx, cy)
	#define gdispDrawCircle(x, y, radius, color)				GDISP_LLD(drawcircle)(x, y, radius, color)
	#define gdispFillCircle(x, y, radius, color)				GDISP_LLD(fillcircle)(x, y, radius, color)
	#define gdispDrawArc(x, y, radius, sangle, eangle, color)	GDISP_LLD(drawarc)(x, y, radius, sangle, eangle, color)
	#define gdispFillArc(x, y, radius, sangle, eangle, color)	GDISP_LLD(fillarc)(x, y, radius, sangle, eangle, color)
	#define gdispDrawEllipse(x, y, a, b, color)					GDISP_LLD(drawellipse)(x, y, a, b, color)
	#define gdispFillEllipse(x, y, a, b, color)					GDISP_LLD(fillellipse)(x, y, a, b, color)
	#define gdispDrawChar(x, y, c, font, color)					GDISP_LLD(drawchar)(x, y, c, font, color)
	#define gdispDrawCharInv(x, y, c, font, color)				GDISP_LLD(drawchar_inv)(x, y, c, font, color)
	#define gdispFillChar(x, y, c, font, color, bgcolor)		GDISP_LLD(fillchar)(x, y, c, font, color, bgcolor)
	#define gdispGetPixelColor(x, y)							GDISP_LLD(getpixelcolor)(x, y)
	#define gdispVerticalScroll(x, y, cx, cy, lines, bgcolor)	GDISP_LLD(verticalscroll)(x, y, cx, cy, lines, bgcolor)
	#define gdispControl(what, value)							GDISP_LLD(control)(what, value)
	#define gdispQuery(what)									GDISP_LLD(query)(what)

#endif

/* Now obsolete functions */
#define gdispBlitArea(x, y, cx, cy, buffer)						gdispBlitAreaEx(x, y, cx, cy, 0, 0, cx, buffer)

/* Extra drawing functions */
void gdispDrawBox(coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color);

/* Extra Text Functions */
#if GDISP_NEED_TEXT
	void gdispDrawString(coord_t x, coord_t y, const char *str, font_t font, color_t color);
	void gdispDrawStringInv(coord_t x, coord_t y, const char *str, font_t font, color_t color);
	void gdispFillString(coord_t x, coord_t y, const char *str, font_t font, color_t color, color_t bgcolor);
	void gdispFillStringBox(coord_t x, coord_t y, coord_t cx, coord_t cy, const char* str, font_t font, color_t color, color_t bgColor, justify_t justify);
	coord_t gdispGetFontMetric(font_t font, fontmetric_t metric);
	coord_t gdispGetCharWidth(char c, font_t font);
	coord_t gdispGetStringWidth(const char* str, font_t font);
	coord_t gdispGetStringHeight(font_t font);
#endif

/* Support routine for packed pixel formats */
#ifndef gdispPackPixels
	void gdispPackPixels(const pixel_t *buf, coord_t cx, coord_t x, coord_t y, color_t color);
#endif

/* Macro definitions for common gets and sets */
#define gdispSetPowerMode(powerMode)			gdispControl(GDISP_CONTROL_POWER, (void *)(unsigned)(powerMode))
#define gdispSetOrientation(newOrientation)		gdispControl(GDISP_CONTROL_ORIENTATION, (void *)(unsigned)(newOrientation))
#define gdispSetBacklight(percent)				gdispControl(GDISP_CONTROL_BACKLIGHT, (void *)(unsigned)(percent))
#define gdispSetContrast(percent)				gdispControl(GDISP_CONTROL_CONTRAST, (void *)(unsigned)(percent))

#define gdispGetWidth()							((coord_t)(unsigned)gdispQuery(GDISP_QUERY_WIDTH))
#define gdispGetHeight()						((coord_t)(unsigned)gdispQuery(GDISP_QUERY_HEIGHT))
#define gdispGetPowerMode()						((gdisp_powermode_t)(unsigned)gdispQuery(GDISP_QUERY_POWER))
#define gdispGetOrientation()					((gdisp_orientation_t)(unsigned)gdispQuery(GDISP_QUERY_ORIENTATION))
#define gdispGetBacklight()						((coord_t)(unsigned)gdispQuery(GDISP_QUERY_BACKLIGHT))
#define gdispGetContrast()						((coord_t)(unsigned)gdispQuery(GDISP_QUERY_CONTRAST))

/* More interesting macro's */
#define gdispUnsetClip()						gdispSetClip(0,0,gdispGetWidth(),gdispGetHeight())


#ifdef __cplusplus
}
#endif

//#endif /* HAL_USE_GDISP */

#endif /* _GDISP_H */
/** @} */
