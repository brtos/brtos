#include "BRTOS.h"
#include "touch_7846.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

Pen_Holder Pen_Point;

//extern BRTOS_Mbox	*MboxTouch;

/*
 * Declaration of TouchPanel function handler. Users must define their own functions handlers.
 * */
extern void TP_Handler(void);

unsigned char flag=0;

														
unsigned char SPI_WriteByte(u8 num)    
{  
  unsigned char Data = 0;
  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_TXE)==RESET);
  SPI_I2S_SendData(SPI1,num);
  while(SPI_I2S_GetFlagStatus(SPI1,SPI_I2S_FLAG_RXNE)==RESET);
  Data = SPI_I2S_ReceiveData(SPI1);

  return Data; 	 				   
} 	


static u16 tp_read_value(u8 cmd)
{
	u8  read = 0;
	u16 ret  = 0;

	T_CS();
	SPI_WriteByte(cmd);
	read = SPI_WriteByte(0);
    ret = (u16)(read << 5);
    read = SPI_WriteByte(0);
    ret = ret | (read >> 3);
    T_DCS();

	return ret;
}

static u16 tp_filter(u16 *sampleBuf)
{
	u16 temp;
	int i,j;

	for(i = 0; i < 4; i++)
	{
		for(j=i; j < 7; j++)
		{
			if(sampleBuf[i] > sampleBuf[j])
			{
				/* Swap the values */
				temp = sampleBuf[i];
				sampleBuf[i] = sampleBuf[j];
				sampleBuf[j] = temp;
			}
		}
	}
	return sampleBuf[3];
}

static u16 tp_read_axis(u8 axis)
{
	int i = 0;
	u16 buffer[7];

	// Valid values are 0 or 1
	if (axis > 1)
		return 0;

	if (axis == AXIS_X)
		axis = 0xD1;
	else
		axis = 0x91;

	/* Discard the first conversion - very noisy and keep the ADC on hereafter
	 * till we are done with the sampling. Note that PENIRQ is disabled.
 	 */
	(void)tp_read_value(axis);

	for(i = 0; i < 7; i++)
	{
		buffer[i]=tp_read_value(axis);
	}

	/* Switch on PENIRQ once again - perform a dummy read */
	(void)tp_read_value((u8)(axis - 1));

	return tp_filter(buffer);
}



void TP_Read(void)
{

   u16 pen_tmp;

   Pen_Point.X=tp_read_axis(AXIS_X);
   Pen_Point.Y=tp_read_axis(AXIS_Y);

   Pen_Point.X0=(int)((Pen_Point.X-Pen_Point.xoff)/Pen_Point.xfac);
   Pen_Point.Y0=(int)((Pen_Point.Y-Pen_Point.yoff)/Pen_Point.yfac);


   if(Pen_Point.X0>=SCREEN_HEIGHT)
   {
	 Pen_Point.X0=(SCREEN_HEIGHT-1);
   }
   if(Pen_Point.Y0>=SCREEN_WIDTH)
   {
	 Pen_Point.Y0=(SCREEN_WIDTH-1);
   }

   if (Pen_Point.CalibOrientation == landscape)
   {
	   pen_tmp = Pen_Point.Y0;
	   Pen_Point.Y0 = Pen_Point.X0;
	   Pen_Point.X0 = pen_tmp;
   }

   if (Pen_Point.CalibOrientation != gdispGetOrientation())
   {
	   if ((Pen_Point.CalibOrientation == portrait) && (gdispGetOrientation() == landscape))
		{
		   pen_tmp = Pen_Point.Y0;
		   Pen_Point.Y0 = Pen_Point.X0;
		   Pen_Point.X0 = (SCREEN_WIDTH - 1) - pen_tmp;
		}

	   if ((Pen_Point.CalibOrientation == landscape) && (gdispGetOrientation() == portrait))
		{
		   pen_tmp = Pen_Point.X0;
		   Pen_Point.X0 = Pen_Point.Y0;
		   Pen_Point.Y0 = (SCREEN_WIDTH - 1) - pen_tmp;
		}
   }

}

void TP_Calibration(void)
{
	u16 x1,x2,y1,y2;

	Pen_Point.CalibOrientation = gdispGetOrientation();
	gdispDrawString(40,35, "Toque na marca para calibrar !", &fontUI1, White);

	gdispDrawLine(5, 5, 15, 5, White);
	gdispDrawLine(10, 0, 10, 10, White);

	while(1)
	{
	  // If the touch panel is pressed
	  if(GPIO_ReadInputDataBit(TOUCH_INT_PORT,TOUCH_INT_PIN)==0)
	  {
		  // Read position
		  x1=tp_read_axis(AXIS_X);
		  y1=tp_read_axis(AXIS_Y);
		  break;
	  }
	  DelayTask(2);
	}

	while(GPIO_ReadInputDataBit(TOUCH_INT_PORT,TOUCH_INT_PIN)==0);
	DelayTask(100);

	gdispClear(Black);
	if (gdispGetOrientation() == portrait)
	{
		gdispDrawString(40,((SCREEN_WIDTH / 2) + 25), "Toque na marca para calibrar !", &fontUI2, White);
		gdispDrawLine(((SCREEN_HEIGHT / 2) - 5) , (SCREEN_WIDTH / 2), ((SCREEN_HEIGHT / 2) + 5), (SCREEN_WIDTH / 2), White);
		gdispDrawLine((SCREEN_HEIGHT / 2) , ((SCREEN_WIDTH / 2) - 5), (SCREEN_HEIGHT / 2), ((SCREEN_WIDTH / 2) + 5), White);
	}
	if (gdispGetOrientation() == landscape)
	{
		gdispDrawString(40,((SCREEN_HEIGHT / 2) + 25), "Toque na marca para calibrar !", &fontUI2, White);
		gdispDrawLine(((SCREEN_WIDTH / 2) - 5) , (SCREEN_HEIGHT / 2), ((SCREEN_WIDTH / 2) + 5), (SCREEN_HEIGHT / 2), White);
		gdispDrawLine((SCREEN_WIDTH / 2) , ((SCREEN_HEIGHT / 2) - 5), (SCREEN_WIDTH / 2), ((SCREEN_HEIGHT / 2) + 5), White);
	}

	while(1)
	{
	  // If the touch panel is pressed
	  if(GPIO_ReadInputDataBit(TOUCH_INT_PORT,TOUCH_INT_PIN)==0)
	  {
		  // Read position
		  x2=tp_read_axis(AXIS_X);
		  y2=tp_read_axis(AXIS_Y);
		  if ((x2 > 1000) || (y2 > 1000))
			  break;
	  }
	  DelayTask(2);
	}

	while(GPIO_ReadInputDataBit(TOUCH_INT_PORT,TOUCH_INT_PIN)==0);
	DelayTask(300);

	Pen_Point.xfac = (float)((float)(x2 - x1) / (float)((SCREEN_HEIGHT / 2) - 10));
	Pen_Point.yfac = (float)((float)(y2 - y1) / (float)((SCREEN_WIDTH / 2) - 5));

	Pen_Point.xoff = x1 - (short)(10 * Pen_Point.xfac);
	Pen_Point.yoff = y1 - (short)(5 * Pen_Point.yfac);

	gdispClear(Black);
}


void TP_Init(void)
{

	SPI_InitTypeDef  SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStruct;
	NVIC_InitTypeDef NVIC_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;

	// Enable the clock of the touch ports
	TOUCH_PORT_CLOCK();

	/* Enable SYSCFG clock */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* SPI1 pin config */
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_AF;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;

	GPIO_InitStruct.GPIO_Pin=GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5;
	GPIO_Init(GPIOB,&GPIO_InitStruct);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource3, GPIO_AF_SPI1);      //sclk	PB3
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource4, GPIO_AF_SPI1);		//miso	PB4
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource5, GPIO_AF_SPI1);		//mosi	PB5

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);
												   
	SPI_I2S_DeInit(SPI1);
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;//SPI_CPOL_Low 	 SPI_CPOL_High
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;   //SPI_NSS_Hard	 //SPI_NSS_Soft
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32; 	//32
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1,&SPI_InitStructure);
	SPI_Cmd(SPI1,ENABLE);

	/* ChipSelect pin setup */
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_OUT;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_OType=GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_UP;
	GPIO_InitStruct.GPIO_Pin=TOUCH_CS_PIN;
	GPIO_Init(TOUCH_CS_PORT,&GPIO_InitStruct);
	T_DCS();

	/* Touch interrupt pin setup */
	GPIO_InitStruct.GPIO_Mode=GPIO_Mode_IN;
	GPIO_InitStruct.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_PuPd=GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Pin=TOUCH_INT_PIN;
	GPIO_Init(TOUCH_INT_PORT,&GPIO_InitStruct);

	/* Connect EXTI Line to pin */
	SYSCFG_EXTILineConfig(TOUCH_EXTI_PortSource, TOUCH_EXTI_Source);

	/* Configure EXTI Line */
	EXTI_InitStructure.EXTI_Line = TOUCH_EXTI_Line;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	/* Enable and set EXTI Line Interrupt to the lowest priority */
	NVIC_InitStructure.NVIC_IRQChannel = TOUCH_IRQ_Channel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

}

/*
 * Enable or disable touchpad interrupt
 * */
void TP_InterruptEnable(FunctionalState state)
{
	if (state == ENABLE)
	{
	    NVIC->ISER[TOUCH_IRQ_Channel >> 0x05] =
	      (uint32_t)0x01 << (TOUCH_IRQ_Channel & (uint8_t)0x1F);	/* Enable NVIC Channel associated with EXTI */
	}
	else
	{
	    NVIC->ICER[TOUCH_IRQ_Channel >> 0x05] =
	      (uint32_t)0x01 << (TOUCH_IRQ_Channel & (uint8_t)0x1F);	/* Disable NVIC Channel associated with EXTI */
	}
}

/* Touch external interrupt pin handler */
void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(TOUCH_EXTI_Line) != RESET)
	{
		TP_Handler(); // Call the TouchPanel function handler

		EXTI_ClearITPendingBit(TOUCH_EXTI_Line);
	}
	OS_INT_EXIT_EXT();
}
