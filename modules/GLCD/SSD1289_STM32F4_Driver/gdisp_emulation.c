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

/*
	Emulation routines included into gdisp_lld.c
*/

/*
	Even though this is a software emulation of a low level driver
	most validation doesn't need to happen here as eventually
	we call a real low level driver routine and if validation is
	required - it will do it.
*/
#ifndef GDISP_EMULATION_C
#define GDISP_EMULATION_C

#include "gdisp.h"
#include "gdisp_lld.h"

//#if HAL_USE_GDISP || defined(__DOXYGEN__)

#ifdef UNUSED
#elif defined(__GNUC__)
# define UNUSED(x) UNUSED_ ## x __attribute__((unused))
#elif defined(__LCLINT__)
# define UNUSED(x) /*@unused@*/ x
#else
# define UNUSED(x) x
#endif

#ifndef GDISP_LLD_NO_STRUCT
	static struct GDISPDriver {
		coord_t				Width;
		coord_t				Height;
		gdisp_orientation_t	Orientation;
		gdisp_powermode_t	Powermode;
		coord_t				Backlight;
		coord_t				Contrast;
		#if GDISP_NEED_CLIP || GDISP_NEED_VALIDATION
			coord_t				clipx0, clipy0;
			coord_t				clipx1, clipy1;		/* not inclusive */
		#endif
		} GDISP;
#endif

#if !GDISP_HARDWARE_CLEARS 
	void GDISP_LLD(clear)(color_t color) {
		GDISP_LLD(fillarea)(0, 0, GDISP.Width, GDISP.Height, color);
	}
#endif

#if !GDISP_HARDWARE_LINES 
	void GDISP_LLD(drawline)(coord_t x0, coord_t y0, coord_t x1, coord_t y1, color_t color) {
		int16_t dy, dx;
		int16_t addx, addy;
		int16_t P, diff, i;

		#if GDISP_HARDWARE_FILLS || GDISP_HARDWARE_SCROLL
		// speed improvement if vertical or horizontal
		if (x0 == x1) {
			if (y1 > y0)
				GDISP_LLD(fillarea)(x0, y0, 1, y1-y0+1, color);
			else
				GDISP_LLD(fillarea)(x0, y1, 1, y0-y1+1, color);
			return;
		}
		if (y0 == y1) {
			if (x1 > x0)
				GDISP_LLD(fillarea)(x0, y0, x1-x0+1, 1, color);
			else
				GDISP_LLD(fillarea)(x0, y1, x0-x1+1, 1, color);
			return;
		}
		#endif

		if (x1 >= x0) {
			dx = x1 - x0;
			addx = 1;
		} else {
			dx = x0 - x1;
			addx = -1;
		}
		if (y1 >= y0) {
			dy = y1 - y0;
			addy = 1;
		} else {
			dy = y0 - y1;
			addy = -1;
		}

		if (dx >= dy) {
			dy *= 2;
			P = dy - dx;
			diff = P - dx;

			for(i=0; i<=dx; ++i) {
				GDISP_LLD(drawpixel)(x0, y0, color);
				if (P < 0) {
					P  += dy;
					x0 += addx;
				} else {
					P  += diff;
					x0 += addx;
					y0 += addy;
				}
			}
		} else {
			dx *= 2;
			P = dx - dy;
			diff = P - dy;

			for(i=0; i<=dy; ++i) {
				GDISP_LLD(drawpixel)(x0, y0, color);
				if (P < 0) {
					P  += dx;
					y0 += addy;
				} else {
					P  += diff;
					x0 += addx;
					y0 += addy;
				}
			}
		}
	}
#endif

#if !GDISP_HARDWARE_FILLS
	void GDISP_LLD(fillarea)(coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color) {
		#if GDISP_HARDWARE_SCROLL
			GDISP_LLD(verticalscroll)(x, y, cx, cy, cy, color);
		#elif GDISP_HARDWARE_LINES
			coord_t x1, y1;
			
			x1 = x + cx - 1;
			y1 = y + cy;
			for(; y < y1; y++)
				GDISP_LLD(drawline)(x, y, x1, y, color);
		#else
			coord_t x0, x1, y1;
			
			x0 = x;
			x1 = x + cx;
			y1 = y + cy;
			for(; y < y1; y++)
				for(x = x0; x < x1; x++)
					GDISP_LLD(drawpixel)(x, y, color);
		#endif
	}
#endif

#if !GDISP_HARDWARE_BITFILLS
	void GDISP_LLD(blitareaex)(coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t srcx, coord_t srcy, coord_t srccx, const pixel_t *buffer) {
			coord_t x0, x1, y1;
			
			x0 = x;
			x1 = x + cx;
			y1 = y + cy;
			buffer += srcy*srccx+srcx;
			srccx -= cx;
			for(; y < y1; y++, buffer += srccx)
				for(x=x0; x < x1; x++)
					GDISP_LLD(drawpixel)(x, y, *buffer++);
	}
#endif

#if GDISP_NEED_CLIP && !GDISP_HARDWARE_CLIP
	void GDISP_LLD(setclip)(coord_t x, coord_t y, coord_t cx, coord_t cy) {
		#if GDISP_NEED_VALIDATION
			if (x >= GDISP.Width || y >= GDISP.Height || cx < 0 || cy < 0)
				return;
			if (x < 0) x = 0;
			if (y < 0) y = 0;
			if (x+cx > GDISP.Width) cx = GDISP.Width - x;
			if (y+cy > GDISP.Height) cy = GDISP.Height - y;
		#endif
		GDISP.clipx0 = x;
		GDISP.clipy0 = y;
		GDISP.clipx1 = x+cx;
		GDISP.clipy1 = y+cy;
	}
#endif

#if GDISP_NEED_CIRCLE && !GDISP_HARDWARE_CIRCLES
	void GDISP_LLD(drawcircle)(coord_t x, coord_t y, coord_t radius, color_t color) {
		coord_t a, b, P;

		a = 0;
		b = radius;
		P = 1 - radius;

		do {
			GDISP_LLD(drawpixel)(x+a, y+b, color);
			GDISP_LLD(drawpixel)(x+b, y+a, color);
			GDISP_LLD(drawpixel)(x-a, y+b, color);
			GDISP_LLD(drawpixel)(x-b, y+a, color);
			GDISP_LLD(drawpixel)(x+b, y-a, color);
			GDISP_LLD(drawpixel)(x+a, y-b, color);
			GDISP_LLD(drawpixel)(x-a, y-b, color);
			GDISP_LLD(drawpixel)(x-b, y-a, color);
			if (P < 0)
				P += 3 + 2*a++;
			else
				P += 5 + 2*(a++ - b--);
		} while(a <= b);
	}
#endif

#if GDISP_NEED_CIRCLE && !GDISP_HARDWARE_CIRCLEFILLS
	void GDISP_LLD(fillcircle)(coord_t x, coord_t y, coord_t radius, color_t color) {
		coord_t a, b, P;
		
		a = 0;
		b = radius;
		P = 1 - radius;

		do {
			GDISP_LLD(drawline)(x-a, y+b, x+a, y+b, color);
			GDISP_LLD(drawline)(x-a, y-b, x+a, y-b, color);
			GDISP_LLD(drawline)(x-b, y+a, x+b, y+a, color);
			GDISP_LLD(drawline)(x-b, y-a, x+b, y-a, color);
			if (P < 0)
				P += 3 + 2*a++;
			else
				P += 5 + 2*(a++ - b--);
		} while(a <= b);
	}
#endif

#if GDISP_NEED_ELLIPSE && !GDISP_HARDWARE_ELLIPSES
	void GDISP_LLD(drawellipse)(coord_t x, coord_t y, coord_t a, coord_t b, color_t color) {
		int  dx = 0, dy = b; /* im I. Quadranten von links oben nach rechts unten */
		long a2 = a*a, b2 = b*b;
		long err = b2-(2*b-1)*a2, e2; /* Fehler im 1. Schritt */

		do {
			GDISP_LLD(drawpixel)(x+dx, y+dy, color); /* I. Quadrant */
			GDISP_LLD(drawpixel)(x-dx, y+dy, color); /* II. Quadrant */
			GDISP_LLD(drawpixel)(x-dx, y-dy, color); /* III. Quadrant */
			GDISP_LLD(drawpixel)(x+dx, y-dy, color); /* IV. Quadrant */

			e2 = 2*err;
			if(e2 <  (2*dx+1)*b2) {
				dx++;
				err += (2*dx+1)*b2;
			}
			if(e2 > -(2*dy-1)*a2) {
				dy--;
				err -= (2*dy-1)*a2;
			}
		} while(dy >= 0); 

		while(dx++ < a) { /* fehlerhafter Abbruch bei flachen Ellipsen (b=1) */
			GDISP_LLD(drawpixel)(x+dx, y, color); /* -> Spitze der Ellipse vollenden */
			GDISP_LLD(drawpixel)(x-dx, y, color);
	   }   
	}
#endif

#if GDISP_NEED_ELLIPSE && !GDISP_HARDWARE_ELLIPSEFILLS
	void GDISP_LLD(fillellipse)(coord_t x, coord_t y, coord_t a, coord_t b, color_t color) {
		int  dx = 0, dy = b; /* im I. Quadranten von links oben nach rechts unten */
		long a2 = a*a, b2 = b*b;
		long err = b2-(2*b-1)*a2, e2; /* Fehler im 1. Schritt */

		do {
			GDISP_LLD(drawline)(x-dx,y+dy,x+dx,y+dy, color);
			GDISP_LLD(drawline)(x-dx,y-dy,x+dx,y-dy, color);

			e2 = 2*err;
			if(e2 <  (2*dx+1)*b2) {
				dx++;
				err += (2*dx+1)*b2;
			}
			if(e2 > -(2*dy-1)*a2) {
				dy--;
				err -= (2*dy-1)*a2;
			}
		} while(dy >= 0); 

		while(dx++ < a) { /* fehlerhafter Abbruch bei flachen Ellipsen (b=1) */
			GDISP_LLD(drawpixel)(x+dx, y, color); /* -> Spitze der Ellipse vollenden */
			GDISP_LLD(drawpixel)(x-dx, y, color);
	   }   
	}
#endif

#if GDISP_NEED_ARC && !GDISP_HARDWARE_ARCS

	#include <math.h>

	/*
	 * @brief				Internal helper function for gdispDrawArc()
	 *
	 * @note				DO NOT USE DIRECTLY!
	 *
	 * @param[in] x, y		The middle point of the arc
	 * @param[in] start		The start angle of the arc
	 * @param[in] end		The end angle of the arc
	 * @param[in] radius	The radius of the arc
	 * @param[in] color		The color in which the arc will be drawn
	 *
	 * @notapi
	 */
	static void _draw_arc(coord_t x, coord_t y, uint16_t start, uint16_t end, uint16_t radius, color_t color) {
	    if (start >= 0 && start <= 180) {
	        float x_maxI = x + radius*cos(start*M_PI/180);
	        float x_minI;

	        if (end > 180)
	            x_minI = x - radius;
	        else
	            x_minI = x + radius*cos(end*M_PI/180);

	        int a = 0;
	        int b = radius;
	        int P = 1 - radius;

	        do {
	            if(x-a <= x_maxI && x-a >= x_minI)
	            	GDISP_LLD(drawpixel)(x-a, y+b, color);
	            if(x+a <= x_maxI && x+a >= x_minI)
	            	GDISP_LLD(drawpixel)(x+a, y+b, color);
	            if(x-b <= x_maxI && x-b >= x_minI)
	            	GDISP_LLD(drawpixel)(x-b, y+a, color);
	            if(x+b <= x_maxI && x+b >= x_minI)
	            	GDISP_LLD(drawpixel)(x+b, y+a, color);

	            if (P < 0) {
	                P = P + 3 + 2*a;
	                a = a + 1;
	            } else {
	                P = P + 5 + 2*(a - b);
	                a = a + 1;
	                b = b - 1;
	            }
	        } while(a <= b);
	    }

	    if (end > 180 && end <= 360) {
	        float x_maxII = x+radius*cos(end*M_PI/180);
	        float x_minII;

	        if(start <= 180)
	            x_minII = x - radius;
	        else
	            x_minII = x+radius*cos(start*M_PI/180);

	        int a = 0;
	        int b = radius;
	        int P = 1 - radius;

	        do {
	            if(x-a <= x_maxII && x-a >= x_minII)
	            	GDISP_LLD(drawpixel)(x-a, y-b, color);
	            if(x+a <= x_maxII && x+a >= x_minII)
	            	GDISP_LLD(drawpixel)(x+a, y-b, color);
	            if(x-b <= x_maxII && x-b >= x_minII)
	            	GDISP_LLD(drawpixel)(x-b, y-a, color);
	            if(x+b <= x_maxII && x+b >= x_minII)
	            	GDISP_LLD(drawpixel)(x+b, y-a, color);

	            if (P < 0) {
	                P = P + 3 + 2*a;
	                a = a + 1;
	            } else {
	                P = P + 5 + 2*(a - b);
	                a = a + 1;
	                b = b - 1;
	            }
	        } while (a <= b);
	    }
	}

	void GDISP_LLD(drawarc)(coord_t x, coord_t y, coord_t radius, coord_t startangle, coord_t endangle, color_t color) {
		if(endangle < startangle) {
	        _draw_arc(x, y, startangle, 360, radius, color);
	        _draw_arc(x, y, 0, endangle, radius, color);
	    } else {
	        _draw_arc(x, y, startangle, endangle, radius, color);
		}
	}
#endif

#if GDISP_NEED_ARC && !GDISP_HARDWARE_ARCFILLS
	void GDISP_LLD(fillarc)(coord_t x, coord_t y, coord_t radius, coord_t startangle, coord_t endangle, color_t color) {
		(void)x;
		(void)y;
		(void)radius;
		(void)startangle;
		(void)endangle;
		(void)color;
#warning "GDISP: FillArc Emulation Not Implemented Yet"
	}
#endif

#if GDISP_NEED_TEXT && !GDISP_HARDWARE_TEXT
	#include "gdisp_fonts.h"
#endif

#if GDISP_NEED_TEXT && !GDISP_HARDWARE_TEXT
	void GDISP_LLD(drawchar)(coord_t x, coord_t y, char c, font_t font, color_t color) {
		const fontcolumn_t	*ptr;
		fontcolumn_t		column;
		coord_t				width, height, xscale, yscale;
		coord_t				i, j, xs, ys;

		/* Check we actually have something to print */
		width = _getCharWidth(font, c);
		if (!width) return;
		
		xscale = font->xscale;
		yscale = font->yscale;
		height = font->height * yscale;
		width *= xscale;

		ptr = _getCharData(font, c);

		/* Loop through the data and display. The font data is LSBit first, down the column */
		for(i=0; i < width; i+=xscale) {
			/* Get the font bitmap data for the column */
			column = *ptr++;
			
			/* Draw each pixel */
			for(j=0; j < height; j+=yscale, column >>= 1) {
				if (column & 0x01) {
					for(xs=0; xs < xscale; xs++)
						for(ys=0; ys < yscale; ys++)
							GDISP_LLD(drawpixel)(x+i+xs, y+j+ys, color);
				}
			}
		}
	}

	void GDISP_LLD(drawchar_inv)(coord_t x, coord_t y, char c, font_t font, color_t color) {
		const fontcolumn_t	*ptr;
		fontcolumn_t		column;
		coord_t				width, height, xscale, yscale;
		coord_t				i, j, xs, ys;

		/* Check we actually have something to print */
		width = _getCharWidth(font, c);
		if (!width) return;

		yscale = font->xscale;
		xscale = font->yscale;
		height = font->height * xscale;
		width *= yscale;

		ptr = _getCharData(font, c);

		/* Loop through the data and display. The font data is LSBit first, down the column */
		for(i=0; i < width; i+=yscale) {
			/* Get the font bitmap data for the column */
			column = *ptr++;

			/* Draw each pixel */
			for(j=0; j < height; j+=yscale, column >>= 1) {
				if (column & 0x01) {
					for(ys=0; ys < yscale; ys++)
						for(xs=0; xs < xscale; xs++)
							GDISP_LLD(drawpixel)(x+j+ys, y-i+xs, color);
				}
			}
		}
	}
#endif

#if GDISP_NEED_TEXT && !GDISP_HARDWARE_TEXTFILLS
	void GDISP_LLD(fillchar)(coord_t x, coord_t y, char c, font_t font, color_t color, color_t bgcolor) {
		coord_t			width, height;
		coord_t			xscale, yscale;
		
		/* Check we actually have something to print */
		width = _getCharWidth(font, c);
		if (!width) return;

		xscale = font->xscale;
		yscale = font->yscale;
		height = font->height * yscale;
		width *= xscale;

		/* Method 1: Use background fill and then draw the text */
		#if GDISP_HARDWARE_TEXT || GDISP_SOFTWARE_TEXTFILLDRAW
			
			/* Fill the area */
			GDISP_LLD(fillarea)(x, y, width, height, bgcolor);
			
			/* Draw the text */
			GDISP_LLD(drawchar)(x, y, c, font, color);

		/* Method 2: Create a single column bitmap and then blit it */
		#elif GDISP_HARDWARE_BITFILLS && GDISP_SOFTWARE_TEXTBLITCOLUMN
		{
			const fontcolumn_t	*ptr;
			fontcolumn_t		column;
			coord_t				i, j, xs, ys;

			/* Working buffer for fast non-transparent text rendering [patch by Badger]
				This needs to be larger than the largest character we can print.
				Assume the max is double sized by one column.
			*/
			static pixel_t		buf[sizeof(fontcolumn_t)*8*2];

			#if GDISP_NEED_VALIDATION
				/* Check our buffer is big enough */
				if ((unsigned)height > sizeof(buf)/sizeof(buf[0]))	return;
			#endif

			ptr = _getCharData(font, c);

			/* Loop through the data and display. The font data is LSBit first, down the column */
			for(i = 0; i < width; i+=xscale) {
				/* Get the font bitmap data for the column */
				column = *ptr++;
				
				/* Draw each pixel */
				for(j = 0; j < height; j+=yscale, column >>= 1) {
					if (column & 0x01) {
						for(ys=0; ys < yscale; ys++)
							gdispPackPixels(buf, 1, j+ys, 0, color);
					} else {
						for(ys=0; ys < yscale; ys++)
							gdispPackPixels(buf, 1, j+ys, 0, bgcolor);
					}
				}

				for(xs=0; xs < xscale; xs++)
					GDISP_LLD(blitareaex)(x+i+xs, y, 1, height, 0, 0, 1, buf);
			}
		}

		/* Method 3: Create a character bitmap and then blit it */
		#elif GDISP_HARDWARE_BITFILLS
		{
			const fontcolumn_t	*ptr;
			fontcolumn_t		column;
			coord_t				i, j, xs, ys;
			
			/* Working buffer for fast non-transparent text rendering [patch by Badger]
				This needs to be larger than the largest character we can print.
				Assume the max is double sized.
			*/
			static pixel_t		buf[20*(sizeof(fontcolumn_t)*8)*2];

			#if GDISP_NEED_VALIDATION
				/* Check our buffer is big enough */
				if ((unsigned)(width * height) > sizeof(buf)/sizeof(buf[0]))	return;
			#endif

			ptr = _getCharData(font, c);

			/* Loop through the data and display. The font data is LSBit first, down the column */
			for(i = 0; i < width; i+=xscale) {
				/* Get the font bitmap data for the column */
				column = *ptr++;
				
				/* Draw each pixel */
				for(j = 0; j < height; j+=yscale, column >>= 1) {
					if (column & 0x01) {
						for(xs=0; xs < xscale; xs++)
							for(ys=0; ys < yscale; ys++)
								gdispPackPixels(buf, width, i+xs, j+ys, color);
					} else {
						for(xs=0; xs < xscale; xs++)
							for(ys=0; ys < yscale; ys++)
								gdispPackPixels(buf, width, i+xs, j+ys, bgcolor);
					}
				}
			}

			/* [Patch by Badger] Write all in one stroke */
			GDISP_LLD(blitareaex)(x, y, width, height, 0, 0, width, buf);
		}

		/* Method 4: Draw pixel by pixel */
		#else
		{
			const fontcolumn_t	*ptr;
			fontcolumn_t		column;
			coord_t				i, j, xs, ys;

			ptr = _getCharData(font, c);

			/* Loop through the data and display. The font data is LSBit first, down the column */
			for(i = 0; i < width; i+=xscale) {
				/* Get the font bitmap data for the column */
				column = *ptr++;
				
				/* Draw each pixel */
				for(j = 0; j < height; j+=yscale, column >>= 1) {
					if (column & 0x01) {
						for(xs=0; xs < xscale; xs++)
							for(ys=0; ys < yscale; ys++)
								GDISP_LLD(drawpixel)(x+i+xs, y+j+ys, color);
					} else {
						for(xs=0; xs < xscale; xs++)
							for(ys=0; ys < yscale; ys++)
								GDISP_LLD(drawpixel)(x+i+xs, y+j+ys, bgcolor);
					}
				}
			}
		}
		#endif
	}
#endif


#if GDISP_NEED_CONTROL && !GDISP_HARDWARE_CONTROL
	void GDISP_LLD(control)(unsigned UNUSED(what), void *UNUSED(value)) {
		/* Ignore everything */
	}
#endif

#if GDISP_NEED_QUERY && !GDISP_HARDWARE_QUERY
void *GDISP_LLD(query)(unsigned what) {
	switch(what) {
	case GDISP_QUERY_WIDTH:			return (void *)(unsigned)GDISP.Width;
	case GDISP_QUERY_HEIGHT:		return (void *)(unsigned)GDISP.Height;
	case GDISP_QUERY_POWER:			return (void *)(unsigned)GDISP.Powermode;
	case GDISP_QUERY_ORIENTATION:	return (void *)(unsigned)GDISP.Orientation;
	case GDISP_QUERY_BACKLIGHT:		return (void *)(unsigned)GDISP.Backlight;
	case GDISP_QUERY_CONTRAST:		return (void *)(unsigned)GDISP.Contrast;
	default:						return (void *)-1;
	}
}
#endif

#if GDISP_NEED_MSGAPI
	void GDISP_LLD(msgdispatch)(gdisp_lld_msg_t *msg) {
		switch(msg->action) {
		case GDISP_LLD_MSG_NOP:
			break;
		case GDISP_LLD_MSG_INIT:
			GDISP_LLD(init)();
			break;
		case GDISP_LLD_MSG_CLEAR:
			GDISP_LLD(clear)(msg->clear.color);
			break;
		case GDISP_LLD_MSG_DRAWPIXEL:
			GDISP_LLD(drawpixel)(msg->drawpixel.x, msg->drawpixel.y, msg->drawpixel.color);
			break;
		case GDISP_LLD_MSG_FILLAREA:
			GDISP_LLD(fillarea)(msg->fillarea.x, msg->fillarea.y, msg->fillarea.cx, msg->fillarea.cy, msg->fillarea.color);
			break;
		case GDISP_LLD_MSG_BLITAREA:
			GDISP_LLD(blitareaex)(msg->blitarea.x, msg->blitarea.y, msg->blitarea.cx, msg->blitarea.cy, msg->blitarea.srcx, msg->blitarea.srcy, msg->blitarea.srccx, msg->blitarea.buffer);
			break;
		case GDISP_LLD_MSG_DRAWLINE:
			GDISP_LLD(drawline)(msg->drawline.x0, msg->drawline.y0, msg->drawline.x1, msg->drawline.y1, msg->drawline.color);
			break;
		#if GDISP_NEED_CLIP
			case GDISP_LLD_MSG_SETCLIP:
				GDISP_LLD(setclip)(msg->setclip.x, msg->setclip.y, msg->setclip.cx, msg->setclip.cy);
				break;
		#endif
		#if GDISP_NEED_CIRCLE
			case GDISP_LLD_MSG_DRAWCIRCLE:
				GDISP_LLD(drawcircle)(msg->drawcircle.x, msg->drawcircle.y, msg->drawcircle.radius, msg->drawcircle.color);
				break;
			case GDISP_LLD_MSG_FILLCIRCLE:
				GDISP_LLD(fillcircle)(msg->fillcircle.x, msg->fillcircle.y, msg->fillcircle.radius, msg->fillcircle.color);
				break;
		#endif
		#if GDISP_NEED_ELLIPSE
			case GDISP_LLD_MSG_DRAWELLIPSE:
				GDISP_LLD(drawellipse)(msg->drawellipse.x, msg->drawellipse.y, msg->drawellipse.a, msg->drawellipse.b, msg->drawellipse.color);
				break;
			case GDISP_LLD_MSG_FILLELLIPSE:
				GDISP_LLD(fillellipse)(msg->fillellipse.x, msg->fillellipse.y, msg->fillellipse.a, msg->fillellipse.b, msg->fillellipse.color);
				break;
		#endif
		#if GDISP_NEED_ARC
			case GDISP_LLD_MSG_DRAWARC:
				GDISP_LLD(drawcircle)(msg->drawarc.x, msg->drawarc.y, msg->drawarc.radius, msg->drawarc.startangle, msg->drawarc.endangle, msg->drawarc.color);
				break;
			case GDISP_LLD_MSG_FILLARC:
				GDISP_LLD(fillcircle)(msg->fillarc.x, msg->fillarc.y, msg->fillarc.radius, msg->fillarc.startangle, msg->fillarc.endangle, msg->fillarc.color);
				break;
		#endif
		#if GDISP_NEED_TEXT
			case GDISP_LLD_MSG_DRAWCHAR:
				GDISP_LLD(drawchar)(msg->drawchar.x, msg->drawchar.y, msg->drawchar.c, msg->drawchar.font, msg->drawchar.color);
				break;
			case GDISP_LLD_MSG_FILLCHAR:
				GDISP_LLD(fillchar)(msg->fillchar.x, msg->fillchar.y, msg->fillchar.c, msg->fillchar.font, msg->fillchar.color, msg->fillchar.bgcolor);
				break;
		#endif
		#if GDISP_NEED_PIXELREAD
			case GDISP_LLD_MSG_GETPIXELCOLOR:
				msg->getpixelcolor.result = GDISP_LLD(getpixelcolor)(msg->getpixelcolor.x, msg->getpixelcolor.y);
				break;
		#endif
		#if GDISP_NEED_SCROLL
			case GDISP_LLD_MSG_VERTICALSCROLL:
				GDISP_LLD(verticalscroll)(msg->verticalscroll.x, msg->verticalscroll.y, msg->verticalscroll.cx, msg->verticalscroll.cy, msg->verticalscroll.lines, msg->verticalscroll.bgcolor);
				break;
		#endif
		#if GDISP_NEED_CONTROL
			case GDISP_LLD_MSG_CONTROL:
				GDISP_LLD(control)(msg->control.what, msg->control.value);
				break;
		#endif
		#if GDISP_NEED_QUERY
			case GDISP_LLD_MSG_QUERY:
				msg->query.result = GDISP_LLD(query)(msg->query.what);
				break;
		#endif
		}
	}
#endif

//#endif  /* HAL_USE_GDISP */
#endif	/* GDISP_EMULATION_C */
