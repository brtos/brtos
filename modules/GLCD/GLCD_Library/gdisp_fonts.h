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
 * @file    gdisp_fonts.h
 * @brief   GDISP internal font definitions.
 * @details	This is not generally needed by an application. It is used
 * 			by the low level drivers that need to understand a font.
 *
 * @addtogroup GDISP
 * @{
 */

#ifndef _GDISP_FONTS_H
#define _GDISP_FONTS_H

/* Don't test against HAL_USE_GDISP as we may want to use this in other non-GDISP utilities. */

/**
 * @brief   The maximum height of a font.
 * @details	Either 16 or 32. Defaults to 16
 * @note	Setting this to 32 causes the font tables to take
 *			twice the internal program memory.
 */
#ifndef GDISP_MAX_FONT_HEIGHT
	#define GDISP_MAX_FONT_HEIGHT	16
#endif

/**
 * @brief   The type of a font column.
 * @note	Set by defining @p GDISP_MAX_FNT_HEIGHT appropriately.
 */
#if GDISP_MAX_FONT_HEIGHT == 16
	typedef uint16_t	fontcolumn_t;
#elif GDISP_MAX_FONT_HEIGHT == 32
	typedef uint32_t	fontcolumn_t;
#else
	#error "GDISP: GDISP_MAX_FONT_HEIGHT must be either 16 or 32"
#endif

/**
 * @brief   Internal font structure.
 * @note	This structure is followed by:
 *				1. An array of character widths (uint8_t)
 *				2. An array of column data offsets (relative to the font structure)
 *				3. Each characters array of column data (fontcolumn_t)
 *			Each sub-structure must be padded to a multiple of 8 bytes
 *			to allow the tables to work across many different compilers.
 */
struct font {
	uint8_t				height;
	uint8_t				charPadding;
	uint8_t				lineSpacing;
	uint8_t				descenderHeight;
	uint8_t				minWidth;
	uint8_t				maxWidth;
	char				minChar;
	char				maxChar;
	uint8_t				xscale;
	uint8_t				yscale;
	const uint8_t		*widthTable;
	const uint16_t      *offsetTable;
	const fontcolumn_t  *dataTable;
	};

/**
 * @brief   Macros to get to the complex parts of the font structure.
 */
#define _getCharWidth(f,c)		(((c) < (f)->minChar || (c) > (f)->maxChar) ? 0 : (f)->widthTable[(c) - (f)->minChar])
#define _getCharOffset(f,c)		((f)->offsetTable[(c) - (f)->minChar])
#define _getCharData(f,c)		(&(f)->dataTable[_getCharOffset(f, c)])

#endif /* _GDISP_FONTS_H */
/** @} */
