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
 * @file    gdispSsd1289/gdisp_lld.c
 * @brief   GDISP Graphics Driver subsystem low level driver source for the Ssd1289 display.
 *
 * @addtogroup GDISP
 * @{
 */

#include "gdisp_lld.h"
#include "stm32f4xx.h"
#include <stddef.h>

//#if HAL_USE_GDISP || defined(__DOXYGEN__)

/* Include the emulation code for things we don't support */
#include "gdisp_emulation.c"


static __inline void lld_lcdWriteIndex(uint16_t index)		{ LCD_REG = index; }
//static __inline void lld_lcdWriteData(uint16_t data)		{ LCD_RAM = data; }
#define lld_lcdWriteData(x)	LCD_RAM = x
static __inline void lld_lcdWriteReg(uint16_t lcdReg,uint16_t lcdRegValue) {
  LCD_REG = lcdReg;
  LCD_RAM = lcdRegValue;
}
static __inline uint16_t lld_lcdReadData(void)				{ return (LCD_RAM); }
static __inline uint16_t lld_lcdReadReg(uint16_t lcdReg) {
  //volatile uint16_t dummy;

  LCD_REG = lcdReg;
  (void)LCD_RAM;
  return (LCD_RAM);
}
static __inline void lld_lcdWriteStreamStart(void)			{ LCD_REG = 0x0022; }
static __inline void lld_lcdWriteStreamStop(void)			{}
static __inline void lld_lcdWriteStream(uint16_t *buffer, uint16_t size) {
  uint16_t i;

  for(i = 0; i < size; i++) LCD_RAM = buffer[i];
}
static __inline void lld_lcdReadStreamStart(void)			{ LCD_REG = 0x0022; }
static __inline void lld_lcdReadStreamStop(void)			{}
static __inline void lld_lcdReadStream(uint16_t *buffer, size_t size)
{
  uint16_t i;

  (void)LCD_RAM; /* throw away first value read */
  for(i = 0; i < size; i++) buffer[i] = LCD_RAM;
}

/* ATUALIZAR AQUI*/
static __inline void lld_lcdDelay(uint16_t us)
{
  INT16U time;
  if (us<1000)
  {
	  time = 1;
  }
  else
  {
	  time = us/1000;
  }
  DelayTask(time);
}


static void lld_lcdSetCursor(uint16_t x, uint16_t y)
{
  /* Reg 0x004E is an 8 bit value
   * Reg 0x004F is 9 bit
   * Use a bit mask to make sure they are not set too high
   */
  switch(GDISP.Orientation) {
    case portraitInv:
      lld_lcdWriteReg(0x004e, (SCREEN_WIDTH-1-x) & 0x00FF);
      lld_lcdWriteReg(0x004f, (SCREEN_HEIGHT-1-y) & 0x01FF);
      break;
    case portrait:
      lld_lcdWriteReg(0x004e, x & 0x00FF);
      lld_lcdWriteReg(0x004f, y & 0x01FF);
      break;
    case landscape:
      lld_lcdWriteReg(0x004e, y & 0x00FF);
      lld_lcdWriteReg(0x004f, x & 0x01FF);
      break;
    case landscapeInv:
      lld_lcdWriteReg(0x004e, (SCREEN_WIDTH - y - 1) & 0x00FF);
      lld_lcdWriteReg(0x004f, (SCREEN_HEIGHT - x - 1) & 0x01FF);
      break;
    }
}

static void lld_lcdSetViewPort(uint16_t x, uint16_t y, uint16_t cx, uint16_t cy)
{
  lld_lcdSetCursor(x, y);

  /* Reg 0x44 - Horizontal RAM address position
   * 		Upper Byte - HEA
   * 		Lower Byte - HSA
   * 		0 <= HSA <= HEA <= 0xEF
   * Reg 0x45,0x46 - Vertical RAM address position
   * 		Lower 9 bits gives 0-511 range in each value
   * 		0 <= Reg(0x45) <= Reg(0x46) <= 0x13F
   */

  switch(GDISP.Orientation) {
    case portrait:
      lld_lcdWriteReg(0x44, (((x+cx-1) << 8) & 0xFF00 ) | (x & 0x00FF));
      lld_lcdWriteReg(0x45, y & 0x01FF);
      lld_lcdWriteReg(0x46, (y+cy-1) & 0x01FF);
      break;
    case landscape:
      lld_lcdWriteReg(0x44, (((y+cy-1) << 8) & 0xFF00) | (y & 0x00FF));
      lld_lcdWriteReg(0x45, x & 0x01FF);
      lld_lcdWriteReg(0x46, (x+cx-1) & 0x01FF);
      break;
    case portraitInv:
      lld_lcdWriteReg(0x44, (((SCREEN_WIDTH-x-1) & 0x00FF) << 8) | ((SCREEN_WIDTH - (x+cx)) & 0x00FF));
      lld_lcdWriteReg(0x45, (SCREEN_HEIGHT-(y+cy)) & 0x01FF);
      lld_lcdWriteReg(0x46, (SCREEN_HEIGHT-y-1) & 0x01FF);
      break;
    case landscapeInv:
      lld_lcdWriteReg(0x44, (((SCREEN_WIDTH - y - 1) & 0x00FF) << 8) | ((SCREEN_WIDTH - (y+cy)) & 0x00FF));
      lld_lcdWriteReg(0x45, (SCREEN_HEIGHT - (x+cx)) & 0x01FF);
      lld_lcdWriteReg(0x46, (SCREEN_HEIGHT - x - 1) & 0x01FF);
      break;
  }
  lld_lcdSetCursor(x, y);
}

static void lld_lcdResetViewPort(void){
	switch(GDISP.Orientation) {
		case portrait:
			lld_lcdSetViewPort(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH);
			break;
		case portraitInv:
			lld_lcdSetViewPort(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH);
			break;
		case landscape:
			lld_lcdSetViewPort(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			break;
		case landscapeInv:
			lld_lcdSetViewPort(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
			break;
	}
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/* ---- Required Routines ---- */
/*
	The following 2 routines are required.
	All other routines are optional.
*/

/**
 * @brief   Low level GDISP driver initialization.
 *
 * @notapi
 */
bool_t GDISP_LLD(init)(void) {
	#ifdef LCD_USE_FSMC

	  /* Enable FSMC clock */
	  RCC->AHB3ENR |= RCC_AHB3ENR_FSMCEN;

	  /* Enable GPIOD and GPIOE clock */
	  RCC->AHB1ENR |= (RCC_AHB1ENR_GPIODEN | RCC_AHB1ENR_GPIOEEN);

	  /* FSMC GPIOD pin setting as Alternate Function, 100MHz */
	  /* Pin:     15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
	   * Mask32b: 00 00 11 11 00 00 00 00 00 11 00 00 11 11 00 00 = 00001111000000000011000011110000 = 0x0F0030F0
	   * Mask16b:  0  0  1  1  0  0  0  0  0  1  0  0  1  1  0  0 = 0011000001001100 = 0x304C
	   * MODER:   10 10 00 00 10 10 10 10 10 00 10 10 00 00 10 10 = 10100000101010101000101000001010 = 0xA0AA8A0A
	   * OSPEEDR: 11 11 00 00 11 11 11 11 11 00 11 11 00 00 11 11 = 11110000111111111100111100001111 = 0xF0FFCF0F
	   * OTYPER:  all zero (using mask16b)
	   * PUPDR:   all zero (using mask32b)
	   * */
	  GPIOD->MODER &= 0x0F0030F0; /* Reseting bits to be configured */
	  GPIOD->MODER |= 0xA0AA8A0A; /* Configuring bits for AF */
	  GPIOD->OSPEEDR &= 0x0F0030F0;
	  GPIOD->OSPEEDR |= 0xF0FFCF0F; /* Configuring bits for speed 100MHz */
	  GPIOD->OTYPER &= 0x304C;  /* Output type push-pull */
	  GPIOD->PUPDR &= 0x0F0030F0;  /* No pull-up or pull-down */

	  /* FSMC GPIOE pin setting as Alternate Function, 100MHz */
	  /* Pin:     15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
	   * Mask32b: 00 00 00 00 00 00 00 00 00 11 11 11 11 11 11 11 = 00000000000000000011111111111111 = 0x00003FFF
	   * Mask16b:  0  0  0  0  0  0  0  0  0  1  1  1  1  1  1  1 = 0000000001111111 = 0x007F
	   * MODER:   10 10 10 10 10 10 10 10 10 00 00 00 00 00 00 00 = 10101010101010101000000000000000 = 0xAAAA8000
	   * OSPEEDR: 11 11 11 11 11 11 11 11 11 00 00 00 00 00 00 00 = 11111111111111111100000000000000 = 0xFFFFC000
	   * OTYPER:  all zero (using mask16b)
	   * PUPDR:   all zero (using mask32b)
	   * */
	  GPIOE->MODER &= 0x00003FFF; /* Reseting bits to be configured */
	  GPIOE->MODER |= 0xAAAA8000; /* Configuring bits for AF */
	  GPIOE->OSPEEDR &= 0x00003FFF; /* Reseting bits to be configured */
	  GPIOE->OSPEEDR |= 0xFFFFC000; /* Configuring bits for speed 100MHz */
	  GPIOE->OTYPER &= 0x007F;  /* Output type push-pull */
	  GPIOE->PUPDR &= 0x00003FFF;  /* No pull-up or pull-down */

	  /* FSMC GPIOD pin alternate function setup (0xC) */
	  /* Pin AFRL:  07   06   05   04   03   02   01   00
	   * MaskAFRL: 0000 1111 0000 0000 1111 1111 0000 0000 = 0x0F00FF00
	   * AFRL:     1100 0000 1100 1100 0000 0000 1100 1100 = 0xC0CC00CC
	   * Pin AFRH:  15   14   13   12   11   10   09   08
	   * MaskAFRH: 0000 0000 1111 1111 0000 0000 0000 0000 = 0x00FF0000
	   * AFRH:     1100 1100 0000 0000 1100 1100 1100 1100 = 0xCC00CCCC
	   * */
	  GPIOD->AFR[0] &= 0x0F00FF00;
	  GPIOD->AFR[0] |= 0xC0CC00CC;
	  GPIOD->AFR[1] &= 0x00FF0000;
	  GPIOD->AFR[1] |= 0xCC00CCCC;

	  /* FSMC GPIOE pin alternate function setup (0xC) */
	  /* Pin AFRL:  07   06   05   04   03   02   01   00
	   * MaskAFRL: 0000 1111 1111 1111 1111 1111 1111 1111 = 0x0FFFFFFF
	   * AFRL:     1100 0000 0000 0000 0000 0000 0000 0000 = 0xC0000000
	   * Pin AFRH:  15   14   13   12   11   10   09   08
	   * MaskAFRH: 0000 0000 0000 0000 0000 0000 0000 0000 = 0x00000000
	   * AFRH:     1100 1100 1100 1100 1100 1100 1100 1100 = 0xCCCCCCCC
	   * */
	  GPIOE->AFR[0] &= 0x0FFFFFFF;
	  GPIOE->AFR[0] |= 0xC0000000;
	  GPIOE->AFR[1] = 0xCCCCCCCC;


		const unsigned char FSMC_Bank = 0;
		/* FSMC timing */

		FSMC_Bank1->BTCR[FSMC_Bank+1] = (FSMC_BTR1_ADDSET_1 | FSMC_BTR1_ADDSET_3) \
				| (FSMC_BTR1_DATAST_1 | FSMC_BTR1_DATAST_3) \
				| (FSMC_BTR1_BUSTURN_1 | FSMC_BTR1_BUSTURN_3) ;

		/* Bank1 NOR/SRAM control register configuration
		 * This is actually not needed as already set by default after reset */
		FSMC_Bank1->BTCR[FSMC_Bank] = FSMC_BCR1_MWID_0 | FSMC_BCR1_WREN | FSMC_BCR1_MBKEN;
	#elif defined(LCD_USE_GPIO)
		//IOBus busCMD = {LCD_CMD_PORT, (1 << LCD_CS) | (1 << LCD_RS) | (1 << LCD_WR) | (1 << LCD_RD), 0};
		//IOBus busDATA = {LCD_CMD_PORT, 0xFFFFF, 0};
		//palSetBusMode(&busCMD, PAL_MODE_OUTPUT_PUSHPULL);
		//palSetBusMode(&busDATA, PAL_MODE_OUTPUT_PUSHPULL);

	#else
		#error "Please define LCD_USE_FSMC or LCD_USE_GPIO"
	#endif

		lld_lcdWriteReg(0x0007,0x0021);    DelayTask(1);
		lld_lcdWriteReg(0x0000,0x0001);    DelayTask(1);
		lld_lcdWriteReg(0x0007,0x0023);    DelayTask(1);
		lld_lcdWriteReg(0x0010,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x0007,0x0033);    DelayTask(1);
		lld_lcdWriteReg(0x0011,0x6830);    DelayTask(1);
		lld_lcdWriteReg(0x0002,0x0600);    DelayTask(1);
		lld_lcdWriteReg(0x0012,0x6CEB);    DelayTask(1);
		lld_lcdWriteReg(0x0003,0xA8A4);    DelayTask(1);
		lld_lcdWriteReg(0x000C,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x000D,0x080C);    DelayTask(1);
		lld_lcdWriteReg(0x000E,0x2B00);    DelayTask(1);
		lld_lcdWriteReg(0x001E,0x00B0);    DelayTask(1);
		lld_lcdWriteReg(0x0001,0x2b3F);    DelayTask(1);  //RGB
		lld_lcdWriteReg(0x0005,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x0006,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x0016,0xEF1C);    DelayTask(1);
		lld_lcdWriteReg(0x0017,0x0103);    DelayTask(1);
		lld_lcdWriteReg(0x000B,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x000F,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x0041,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x0042,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x0048,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x0049,0x013F);    DelayTask(1);
		lld_lcdWriteReg(0x004A,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x004B,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x0044,0xEF00);    DelayTask(1);
		lld_lcdWriteReg(0x0045,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x0046,0x013F);    DelayTask(1);
		lld_lcdWriteReg(0x0030,0x0707);    DelayTask(1);
		lld_lcdWriteReg(0x0031,0x0204);    DelayTask(1);
		lld_lcdWriteReg(0x0032,0x0204);    DelayTask(1);
		lld_lcdWriteReg(0x0033,0x0502);    DelayTask(1);
		lld_lcdWriteReg(0x0034,0x0507);    DelayTask(1);
		lld_lcdWriteReg(0x0035,0x0204);    DelayTask(1);
		lld_lcdWriteReg(0x0036,0x0204);    DelayTask(1);
		lld_lcdWriteReg(0x0037,0x0502);    DelayTask(1);
		lld_lcdWriteReg(0x003A,0x0302);    DelayTask(1);
		lld_lcdWriteReg(0x002F,0x12BE);    DelayTask(1);
		lld_lcdWriteReg(0x003B,0x0302);    DelayTask(1);
		lld_lcdWriteReg(0x0023,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x0024,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x0025,0x8000);    DelayTask(1);
		lld_lcdWriteReg(0x004f,0x0000);    DelayTask(1);
		lld_lcdWriteReg(0x004e,0x0000);    DelayTask(1);


	#if defined(LCD_USE_FSMC)
		/* FSMC delay reduced as the controller now runs at full speed */
		FSMC_Bank1->BTCR[FSMC_Bank+1] = FSMC_BTR1_ADDSET_1 | FSMC_BTR1_DATAST_2 | FSMC_BTR1_BUSTURN_0 ;
		FSMC_Bank1->BTCR[FSMC_Bank] = FSMC_BCR1_MWID_0 | FSMC_BCR1_WREN | FSMC_BCR1_MBKEN;
	#endif


    /* Initialise the GDISP structure */
	GDISP.Width = SCREEN_WIDTH;
	GDISP.Height = SCREEN_HEIGHT;
	GDISP.Orientation = portrait;
	GDISP.Powermode = powerOn;
	GDISP.Backlight = 100;
	GDISP.Contrast = 50;
	#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
		GDISP.clipx0 = 0;
		GDISP.clipy0 = 0;
		GDISP.clipx1 = GDISP.Width;
		GDISP.clipy1 = GDISP.Height;
	#endif
	return TRUE;
}

/**
 * @brief   Draws a pixel on the display.
 *
 * @param[in] x        X location of the pixel
 * @param[in] y        Y location of the pixel
 * @param[in] color    The color of the pixel
 *
 * @notapi
 */
void GDISP_LLD(drawpixel)(coord_t x, coord_t y, color_t color) {
	#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
		if (x < GDISP.clipx0 || y < GDISP.clipy0 || x >= GDISP.clipx1 || y >= GDISP.clipy1) return;
	#endif
	lld_lcdSetCursor(x, y);
	lld_lcdWriteReg(0x0022, color);
}

/* ---- Optional Routines ---- */
/*
	All the below routines are optional.
	Defining them will increase speed but everything
	will work if they are not defined.
	If you are not using a routine - turn it off using
	the appropriate GDISP_HARDWARE_XXXX macro.
	Don't bother coding for obvious similar routines if
	there is no performance penalty as the emulation software
	makes a good job of using similar routines.
		eg. If gfillarea() is defined there is little
			point in defining clear() unless the
			performance bonus is significant.
	For good performance it is suggested to implement
		fillarea() and blitarea().
*/

#if GDISP_HARDWARE_CLEARS || defined(__DOXYGEN__)
	/**
	 * @brief   Clear the display.
	 * @note    Optional - The high level driver can emulate using software.
	 *
	 * @param[in] color    The color of the pixel
	 *
	 * @notapi
	 */
	void GDISP_LLD(clear)(color_t color) {
	    unsigned i;
	    lld_lcdResetViewPort();
	    lld_lcdSetCursor(0, 0);
	    lld_lcdWriteStreamStart();

	    for(i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
	    	lld_lcdWriteData(color);

	    lld_lcdWriteStreamStop();
	}
#endif

#if GDISP_HARDWARE_FILLS || defined(__DOXYGEN__)
	/**
	 * @brief   Fill an area with a color.
	 * @note    Optional - The high level driver can emulate using software.
	 *
	 * @param[in] x, y     The start filled area
	 * @param[in] cx, cy   The width and height to be filled
	 * @param[in] color    The color of the fill
	 *
	 * @notapi
	 */
	void GDISP_LLD(fillarea)(coord_t x, coord_t y, coord_t cx, coord_t cy, color_t color) {
		#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
			if (x < GDISP.clipx0) { cx -= GDISP.clipx0 - x; x = GDISP.clipx0; }
			if (y < GDISP.clipy0) { cy -= GDISP.clipy0 - y; y = GDISP.clipy0; }
			if (cx <= 0 || cy <= 0 || x >= GDISP.clipx1 || y >= GDISP.clipy1) return;
			if (x+cx > GDISP.clipx1)	cx = GDISP.clipx1 - x;
			if (y+cy > GDISP.clipy1)	cy = GDISP.clipy1 - y;
		#endif

		unsigned i, area;

		area = cx*cy;
		lld_lcdSetViewPort(x, y, cx, cy);
		lld_lcdWriteStreamStart();
		for(i = 0; i < area; i++)
			lld_lcdWriteData(color);
		lld_lcdWriteStreamStop();
		lld_lcdResetViewPort();
	}
#endif

#if GDISP_HARDWARE_BITFILLS || defined(__DOXYGEN__)
	/**
	 * @brief   Fill an area with a bitmap.
	 * @note    Optional - The high level driver can emulate using software.
	 *
	 * @param[in] x, y     The start filled area
	 * @param[in] cx, cy   The width and height to be filled
	 * @param[in] srcx, srcy   The bitmap position to start the fill from
	 * @param[in] srccx    The width of a line in the bitmap.
	 * @param[in] buffer   The pixels to use to fill the area.
	 *
	 * @notapi
	 */
	void GDISP_LLD(blitareaex)(coord_t x, coord_t y, coord_t cx, coord_t cy, coord_t srcx, coord_t srcy, coord_t srccx, const pixel_t *buffer) {
		coord_t endx, endy;
		unsigned lg;

		#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
			if (x < GDISP.clipx0) { cx -= GDISP.clipx0 - x; srcx += GDISP.clipx0 - x; x = GDISP.clipx0; }
			if (y < GDISP.clipy0) { cy -= GDISP.clipy0 - y; srcy += GDISP.clipy0 - y; y = GDISP.clipy0; }
			if (srcx+cx > srccx)		cx = srccx - srcx;
			if (cx <= 0 || cy <= 0 || x >= GDISP.clipx1 || y >= GDISP.clipy1) return;
			if (x+cx > GDISP.clipx1)	cx = GDISP.clipx1 - x;
			if (y+cy > GDISP.clipy1)	cy = GDISP.clipy1 - y;
		#endif

		lld_lcdSetViewPort(x, y, cx, cy);
		lld_lcdWriteStreamStart();

		endx = srcx + cx;
		endy = y + cy;
		lg = srccx - cx;
		buffer += srcx + srcy * srccx;
		for(; y < endy; y++, buffer += lg)
			for(x=srcx; x < endx; x++)
				lld_lcdWriteData(*buffer++);
		lld_lcdWriteStreamStop();
		lld_lcdResetViewPort();
	}
#endif

#if (GDISP_NEED_PIXELREAD && GDISP_HARDWARE_PIXELREAD) || defined(__DOXYGEN__)
	/**
	 * @brief   Get the color of a particular pixel.
	 * @note    Optional.
	 * @note    If x,y is off the screen, the result is undefined.
	 *
	 * @param[in] x, y     The start of the text
	 *
	 * @notapi
	 */
	color_t GDISP_LLD(getpixelcolor)(coord_t x, coord_t y) {
		color_t color;

		#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
			if (x < 0 || x >= GDISP.Width || y < 0 || y >= GDISP.Height) return 0;
		#endif

		lld_lcdSetCursor(x, y);
		lld_lcdWriteStreamStart();

		color = lld_lcdReadData();
		color = lld_lcdReadData();

		lld_lcdWriteStreamStop();

		return color;
	}
#endif

#if (GDISP_NEED_SCROLL && GDISP_HARDWARE_SCROLL) || defined(__DOXYGEN__)
	/**
	 * @brief   Scroll vertically a section of the screen.
	 * @note    Optional.
	 * @note    If x,y + cx,cy is off the screen, the result is undefined.
	 * @note    If lines is >= cy, it is equivelent to a area fill with bgcolor.
	 *
	 * @param[in] x, y     The start of the area to be scrolled
	 * @param[in] cx, cy   The size of the area to be scrolled
	 * @param[in] lines    The number of lines to scroll (Can be positive or negative)
	 * @param[in] bgcolor  The color to fill the newly exposed area.
	 *
	 * @notapi
	 */
	void GDISP_LLD(verticalscroll)(coord_t x, coord_t y, coord_t cx, coord_t cy, int lines, color_t bgcolor) {
		static color_t buf[((SCREEN_HEIGHT > SCREEN_WIDTH ) ? SCREEN_HEIGHT : SCREEN_WIDTH)];
		coord_t row0, row1;
		unsigned i, gap, abslines;

		#if GDISP_NEED_VALIDATION || GDISP_NEED_CLIP
			if (x < GDISP.clipx0) { cx -= GDISP.clipx0 - x; x = GDISP.clipx0; }
			if (y < GDISP.clipy0) { cy -= GDISP.clipy0 - y; y = GDISP.clipy0; }
			if (!lines || cx <= 0 || cy <= 0 || x >= GDISP.clipx1 || y >= GDISP.clipy1) return;
			if (x+cx > GDISP.clipx1)	cx = GDISP.clipx1 - x;
			if (y+cy > GDISP.clipy1)	cy = GDISP.clipy1 - y;
		#endif

		abslines = lines < 0 ? -lines : lines;

		if (abslines >= cy) {
			abslines = cy;
			gap = 0;
		} else {
			gap = cy - abslines;
			for(i = 0; i < gap; i++) {
				if(lines > 0) {
					row0 = y + i + lines;
					row1 = y + i;
				} else {
					row0 = (y - i - 1) + lines;
					row1 = (y - i - 1);
				}

				/* read row0 into the buffer and then write at row1*/
				lld_lcdSetViewPort(x, row0, cx, 1);
				lld_lcdReadStreamStart();
				lld_lcdReadStream(buf, cx);
				lld_lcdReadStreamStop();

				lld_lcdSetViewPort(x, row1, cx, 1);
				lld_lcdWriteStreamStart();
				lld_lcdWriteStream(buf, cx);
				lld_lcdWriteStreamStop();
			}
		}

		/* fill the remaining gap */
		lld_lcdSetViewPort(x, lines > 0 ? (y+gap) : y, cx, abslines);
		lld_lcdWriteStreamStart();
		gap = cx*abslines;
		for(i = 0; i < gap; i++) lld_lcdWriteData(bgcolor);
		lld_lcdWriteStreamStop();
		lld_lcdResetViewPort();
	}
#endif

#if (GDISP_NEED_CONTROL && GDISP_HARDWARE_CONTROL) || defined(__DOXYGEN__)
	/**
	 * @brief   Driver Control
	 * @detail	Unsupported control codes are ignored.
	 * @note	The value parameter should always be typecast to (void *).
	 * @note	There are some predefined and some specific to the low level driver.
	 * @note	GDISP_CONTROL_POWER			- Takes a gdisp_powermode_t
	 * 			GDISP_CONTROL_ORIENTATION	- Takes a gdisp_orientation_t
	 * 			GDISP_CONTROL_BACKLIGHT -	 Takes an int from 0 to 100. For a driver
	 * 											that only supports off/on anything other
	 * 											than zero is on.
	 * 			GDISP_CONTROL_CONTRAST		- Takes an int from 0 to 100.
	 * 			GDISP_CONTROL_LLD			- Low level driver control constants start at
	 * 											this value.
	 *
	 * @param[in] what		What to do.
	 * @param[in] value		The value to use (always cast to a void *).
	 *
	 * @notapi
	 */
	void GDISP_LLD(control)(unsigned what, void *value) {
		switch(what) {
		case GDISP_CONTROL_POWER:
			if (GDISP.Powermode == (gdisp_powermode_t)value)
				return;
			switch((gdisp_powermode_t)value) {
			case powerOff:
				lld_lcdWriteReg(0x0010, 0x0000);	// leave sleep mode
				lld_lcdWriteReg(0x0007, 0x0000);	// halt operation
				lld_lcdWriteReg(0x0000, 0x0000);	// turn off oszillator
				lld_lcdWriteReg(0x0010, 0x0001);	// enter sleepmode
				break;
			case powerOn:
				lld_lcdWriteReg(0x0010, 0x0000);	// leave sleep mode
				if (GDISP.Powermode != powerSleep)
					GDISP_LLD(init)();
				break;
			case powerSleep:
				lld_lcdWriteReg(0x0010, 0x0001);	// enter sleep mode
				break;
			default:
				return;
			}
			GDISP.Powermode = (gdisp_powermode_t)value;
			return;
		case GDISP_CONTROL_ORIENTATION:
			if (GDISP.Orientation == (gdisp_orientation_t)value)
				return;
			switch((gdisp_orientation_t)value) {
			case portrait:
				lld_lcdWriteReg(0x0001, 0x2B3F);
				/* ID = 11 AM = 0 */
				lld_lcdWriteReg(0x0011, 0x6070);
				GDISP.Height = SCREEN_HEIGHT;
				GDISP.Width = SCREEN_WIDTH;
				break;
			case landscape:
				lld_lcdWriteReg(0x0001, 0x293F);
				/* ID = 11 AM = 1 */
				lld_lcdWriteReg(0x0011, 0x6078);
				GDISP.Height = SCREEN_WIDTH;
				GDISP.Width = SCREEN_HEIGHT;
				break;
			case portraitInv:
				lld_lcdWriteReg(0x0001, 0x2B3F);
				/* ID = 01 AM = 0 */
				lld_lcdWriteReg(0x0011, 0x6040);
				GDISP.Height = SCREEN_HEIGHT;
				GDISP.Width = SCREEN_WIDTH;
				break;
			case landscapeInv:
				lld_lcdWriteReg(0x0001, 0x293F);
				/* ID = 01 AM = 1 */
				lld_lcdWriteReg(0x0011, 0x6048);
				GDISP.Height = SCREEN_WIDTH;
				GDISP.Width = SCREEN_HEIGHT;
				break;
			default:
				return;
			}
			#if GDISP_NEED_CLIP || GDISP_NEED_VALIDATION
				GDISP.clipx0 = 0;
				GDISP.clipy0 = 0;
				GDISP.clipx1 = GDISP.Width;
				GDISP.clipy1 = GDISP.Height;
			#endif
			GDISP.Orientation = (gdisp_orientation_t)value;
			return;
/*
		case GDISP_CONTROL_BACKLIGHT:
		case GDISP_CONTROL_CONTRAST:
*/
		}
	}
#endif

//#endif /* HAL_USE_GDISP */
/** @} */
