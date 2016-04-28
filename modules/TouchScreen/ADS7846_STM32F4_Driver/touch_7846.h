
#ifndef __TOUCH_7846_H
#define __TOUCH_7846_H

#ifdef __cplusplus
 extern "C" {
#endif
 
#include "gdisp.h"



#include "stm32f4xx.h"	
typedef struct 
{
	u16 X0;
	u16 Y0;
	u16 X; 
	u16 Y;						   	    
	u8  Key_Sta;
	u8  Key_LSta;

	float xfac;
	float yfac;
	short xoff;
	short yoff;
	gdisp_orientation_t CalibOrientation;
}Pen_Holder;

/* Interrupt codes used with mailbox to pass values from interrupt to main task */
//typedef enum {INT_EXTI_0, INT_TOUCH} InterruptCodeTypedef;


extern Pen_Holder Pen_Point;

#define TOUCH_CS_PORT	 GPIOD
#define TOUCH_CS_PIN	 GPIO_Pin_3

#define T_CS()   GPIO_ResetBits(TOUCH_CS_PORT, TOUCH_CS_PIN);
#define T_DCS()  GPIO_SetBits(TOUCH_CS_PORT, TOUCH_CS_PIN);

/* Touch external interrup pin and line definitions */
#define TOUCH_INT_PIN		GPIO_Pin_6
#define TOUCH_INT_PORT		GPIOD
#define TOUCH_EXTI_Line 	EXTI_Line6
#define TOUCH_EXTI_Source	EXTI_PinSource6
#define TOUCH_EXTI_PortSource EXTI_PortSourceGPIOD
#define TOUCH_IRQ_Channel	EXTI9_5_IRQn
#define TOUCH_PORT_CLOCK()	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOB , ENABLE);	\
							RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOD , ENABLE)

#define CMD_RDY 0X90  //0B10010000
#define CMD_RDX	0XD0  //0B11010000

#define AXIS_X	0
#define AXIS_Y	1
 
//#define PEN  GPIOD->IDR&(1<<6) //
//#define NPEN !(0x0080&PEN)      //!PEN

unsigned char SPI_WriteByte(u8 num);
//void SpiDelay(unsigned int DelayCnt);
void TP_Read(void);
void TP_Init(void);
void TP_Calibration(void);
void EXTI9_5_IRQHandler(void);
void TP_InterruptEnable(FunctionalState state);

#ifdef __cplusplus
}
#endif

#endif 


















