#include "BRTOS.h"
#include "drivers.h"
#include "tasks.h"

extern BRTOS_Sem    *SemTeste;
extern BRTOS_Queue  *Serial;

void System_Time(void)
{
   // task setup
   INT8U i = 0;
   OSResetTime();   
  
   /* task main loop */
   for (;;)
   {
      #if (WATCHDOG == 1)
        wdt_reset();
      #endif
      (void)DelayTask(10);
      i++;
      if (i >= 100)
      {
        OSUpdateUptime();
        i = 0;
      }
   }
}



void Task_2(void)
{
   /* task setup */
   INT8U cont = 0;   
  
   /* task main loop */
   for (;;)
   {
	  cont++;
      (void)DelayTask(20);
	  PORTB = PORTB ^ 2;
      
      //Acorda a tarefa 3
      (void)OSSemPost(SemTeste);
   }
}



void Task_3(void)
{
   /* task setup */
   PORTB = 0x00;
   DDRB = 0xFF;
  
   /* task main loop */
   for (;;)
   {
      PORTB = PORTB ^ 1;
	  (void)OSSemPend(SemTeste,0);
   }
}

#include "OSInfo.h"

char BufferTextDebug[128];

void Task_Serial(void)
{
    /* task setup */
    INT8U pedido = 0;  
  
	strcpy_P(BufferText, (PGM_P)pgm_read_word(&(BRTOSStringTable[0])));
	Serial_Envia_Frase((CHAR8*)BufferText);
			
	Serial_Envia_Caracter(10);
	Serial_Envia_Caracter(13);
			
   // task main loop
   for (;;) 
   {

      if(!OSQueuePend(Serial, &pedido, 0))
      {
		switch(pedido)
        {
          
		  #if (COMPUTES_CPU_LOAD == 1)
          case '1':
            Transmite_CPU_Load();
			//OSCPULoad(BufferTextDebug);
			//Serial_Envia_Frase((CHAR8*)BufferTextDebug);
			Serial_Envia_Caracter(10);
			Serial_Envia_Caracter(13);			
            break;
          #endif
          case '2':
            Transmite_Uptime();
			//OSUptimeInfo(BufferTextDebug);
			//Serial_Envia_Frase((CHAR8*)BufferTextDebug);
			Serial_Envia_Caracter(10);
			Serial_Envia_Caracter(13);			
            break;
          case '3':			
            strcpy_P(BufferText, (PGM_P)pgm_read_word(&(BRTOSStringTable[0])));
            Serial_Envia_Frase((CHAR8*)BufferText);
			
            Serial_Envia_Caracter(10);
            Serial_Envia_Caracter(13);
            break;
          case '4':
            //Transmite_RAM_Ocupada();
			OSAvailableMemory(BufferTextDebug);
			Serial_Envia_Frase((CHAR8*)BufferTextDebug);
			Serial_Envia_Caracter(10);
			Serial_Envia_Caracter(13);			
            break;
		  case '5':
            Transmite_Task_Stacks();
			//OSTaskList(BufferTextDebug);
			//Serial_Envia_Frase((CHAR8*)BufferTextDebug);
			Serial_Envia_Caracter(10);
			Serial_Envia_Caracter(13);			
            break;
          #if (OSTRACE == 1) 
          case '6':
            Send_OSTrace();
            Serial_Envia_Caracter(LF);
            Serial_Envia_Caracter(CR);            
            break;            
          #endif                                    
          default:
            Serial_Envia_Caracter(pedido);
            break;
        }
      }
   }
}
