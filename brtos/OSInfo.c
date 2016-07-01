/*
 * debug.c
 *
 *  Created on: 12/12/2014
 *      Author: Gustavo
 */
#include "BRTOS.h"
#include "OSInfo.h"

static int mem_cpy(char *dst, const char *src)
{
	int i = 0;
	while(*src)
	{
		*dst++ = *src++;
		i++;
	}
	return i;
}

char *PrintDecimal(int16_t val, CHAR8 *buff)
{
   uint16_t backup;
   uint32_t i = 0;
   CHAR8 s = ' ';

   // Fill buffer with spaces
   for (i = 0; i < 6; i++)
   {
      *(buff + i) = ' ';
   }

   // Null termination for data
   *(buff + i) = 0;

   if (val < 0)
   {
      val = -val;
      s = '-';
   }

   // Convert binary value to decimal ASCII
   for (i = 5; i > 0;)
   {
      backup = val;
      val /= 10;
      *(buff + i) = (backup - (val*10)) + '0';
      i--;

      if (val == 0)  break;
   }

   // Completes the string for sign information
   *(buff + i) = s;  // Sign character

   if (*(buff + i) == '-')
   {
	   return (buff+i);
   }else
   {
	   return (buff+i+1);
   }
}


// Imprimir ID, nome, estado, prioridade, stack
/* Tasks are reported as blocked ('B'), ready ('R') or suspended ('S'). */
void OSTaskList(char *string)
{
    uint16_t VirtualStack = 0;
    uint8_t  j = 0;
#ifndef WATERMARK
    uint8_t  i = 0;
#endif
    uint8_t  prio = 0;
    CHAR8  str[9];
    uint32_t *sp_end = 0;
    uint32_t *sp_address = 0;
    int z,count;
    
    string += mem_cpy(string,"\n\r***********************************************************\n\r");
    string += mem_cpy(string,"ID   NAME                    STATE   PRIORITY   STACK SIZE\n\r");
    string += mem_cpy(string,"***********************************************************\n\r");

	#if (!BRTOS_DYNAMIC_TASKS_ENABLED)
    for (j=1;j<=NumberOfInstalledTasks;j++)
	#else
    for (j=1;j<=NUMBER_OF_TASKS;j++)
	#endif
    {
		  if (ContextTask[j].Priority != EMPTY_PRIO){
			  *string++ = '[';
			  if (j<10)
			  {
				  *string++ = j+'0';
				  string += mem_cpy(string, "]  ");
			  }else
			  {
				  (void)PrintDecimal(j, str);
				  string += mem_cpy(string, (str+4));
				  string += mem_cpy(string, "] ");
			  }
			  z = mem_cpy(string,(char*)ContextTask[j].TaskName);
			  string +=z;

			  // Task name align
			  for(count=0;count<(24-z);count++)
			  {
				  *string++ = ' ';
			  }

			  // Print the task state
			  string += mem_cpy(string,"  ");
			  UserEnterCritical();
			  if ((OSBlockedList & (PriorityMask[ContextTask[j].Priority])) == 0){
				  *string++ = 'B';
			  }else{
				  if ((OSReadyList & (PriorityMask[ContextTask[j].Priority])) == PriorityMask[ContextTask[j].Priority]){
					  *string++ = 'R';
				  }else{
					  *string++ = 'S';
				  }
			  }
			  UserExitCritical();
			  string += mem_cpy(string,"      ");

			  // Print the task priority
			  UserEnterCritical();
			  prio = ContextTask[j].Priority;
			  UserExitCritical();

			  (void)PrintDecimal(prio, str);
			  string += mem_cpy(string, (str+2));
			  string += mem_cpy(string,"       ");

			  // Print the task stack size
			  UserEnterCritical();
			  sp_address = (uint32_t*)ContextTask[j].StackPoint;
			  if (j == 1)
			  {
				  #if (!BRTOS_DYNAMIC_TASKS_ENABLED)
				  sp_end = (uint32_t*)&STACK[0];
				  #else
				  sp_end = (uint32_t*)ContextTask[1].StackInit;
				  #endif
			  }else
			  {
				  #if (!BRTOS_DYNAMIC_TASKS_ENABLED)
				  sp_end = (uint32_t*)ContextTask[j-1].StackInit;
				  #else
				  sp_end = (uint32_t*)ContextTask[j].StackInit;
				  #endif
			  }
			  UserExitCritical();

			  // Find for at least 16 available sp data into task stack
			  #ifdef WATERMARK
			  sp_address = sp_end;
			  sp_address++;
			  do
			  {
				  if (*sp_address != 0x24242424)
				  {
					  break;
				  }
				  sp_address++;
			  }while((uint32_t)sp_address <= ContextTask[j].StackInit);			  
			  #else
			  i = 0;
			  while(i<16)
			  {
				if (sp_address <= sp_end)
				{
				  break;
				}
				if (*sp_address == 0)
				{
				  i++;
				}else
				{
				  i = 0;
				}
				sp_address--;
			  }
			  #endif

			  UserEnterCritical();
			  #if (!BRTOS_DYNAMIC_TASKS_ENABLED)
			  #ifdef WATERMARK
			  VirtualStack = ContextTask[j].StackInit - ((uint32_t)sp_address + 4);
			  #else
			  VirtualStack = ContextTask[j].StackInit - ((uint32_t)sp_address + (i*4));
			  #endif
			  #else
			  #ifdef WATERMARK
			  VirtualStack = (ContextTask[j].StackInit + (uint32_t)ContextTask[j].StackSize) - ((uint32_t)sp_address + 4);
			  #else
			  VirtualStack = (ContextTask[j].StackInit + (uint32_t)ContextTask[j].StackSize) - ((uint32_t)sp_address + (i*4));
			  #endif
			  #endif
			  UserExitCritical();

			  (void)PrintDecimal(VirtualStack, str);
			  string += mem_cpy(string, str);

			  string += mem_cpy(string, "\n\r");
		}
    }

    string += mem_cpy(string, "\n\r");

    // End of string
    *string = '\0';
}

#if (COMPUTES_TASK_LOAD == 1)
#include <string.h>
#include <stdio.h>
void OSRuntimeStats(char *string)
{
    uint8_t  j = 0;
    CHAR8  str[16];
    int z,count;
    uint32_t runtime, total_time, percentage;

    string += mem_cpy(string,"\n\r**********************************************\n\r");
    string += mem_cpy(string,"ID   NAME                    Abs Time   % Time \n\r");
    string += mem_cpy(string,"**********************************************\n\r");

    total_time = OSGetTimerForRuntimeStats();
    total_time /= 100UL;

	#if (!BRTOS_DYNAMIC_TASKS_ENABLED)
    for (j=1;j<=NumberOfInstalledTasks;j++)
	#else
    for (j=1;j<=NUMBER_OF_TASKS;j++)
	#endif
    {
		  if (ContextTask[j].Priority != EMPTY_PRIO){
			  *string++ = '[';
			  if (j<10){
				  *string++ = j+'0';
				  string += mem_cpy(string, "]  ");
			  }else{
				  (void)PrintDecimal(j, str);
				  string += mem_cpy(string, (str+4));
				  string += mem_cpy(string, "] ");
			  }

			  // Print task name
			  z = mem_cpy(string,(char*)ContextTask[j].TaskName);
			  string +=z;

			  // Task name align
			  for(count=0;count<(24-z);count++){
				  *string++ = ' ';
			  }

			  // Get task runtime
			  UserEnterCritical();
			  runtime = ContextTask[j].Runtime;
			  UserExitCritical();

			  // Percentage calculation
			  percentage = runtime / total_time;


			  // Print task runtime
			  sprintf( str, "%u", (unsigned int) runtime);
			  z = mem_cpy(string,str);
			  string +=z;

			  // Align
			  for(count=0;count<(12-z);count++){
				  *string++ = ' ';
			  }

			  if( percentage > 0UL ){
				  // Print percentage
				  sprintf( str, "%u%%", (unsigned int)percentage);
			  }
			  else{
				  // If percentace is zero, print <1%
				  sprintf( str, "<1%%");
			  }

			  string += mem_cpy(string,str);
			  string += mem_cpy(string,"\n\r");
		  }
    }

    string += mem_cpy(string, "\n\r");

    // End of string
    *string = '\0';
}
#endif



// memoria total heap de tarefas
// memoria total heap de filas
void OSAvailableMemory(char *string)
{
    uint16_t address = 0;
    CHAR8  str[8];

    string += mem_cpy(string, "\n\r***** BRTOS Memory Info *****\n\r");
#if (!BRTOS_DYNAMIC_TASKS_ENABLED)
    string += mem_cpy(string, "STATIC TASK MEMORY HEAP:  ");
#else
    string += mem_cpy(string, "DYNAMIC MEMORY HEAP:      ");
#endif

    UserEnterCritical();
    address = iStackAddress * sizeof(OS_CPU_TYPE);
    UserExitCritical();

#if (!BRTOS_DYNAMIC_TASKS_ENABLED)
    (void)PrintDecimal(address, str);
#else
    (void)PrintDecimal((int16_t)OSGetUsedHeapSize(), str);
#endif
    string += mem_cpy(string, &str[2]);
    string += mem_cpy(string, " of ");

	#if (!BRTOS_DYNAMIC_TASKS_ENABLED)
    string += mem_cpy(string, PrintDecimal(HEAP_SIZE, str));
	#else
    string += mem_cpy(string, PrintDecimal(DYNAMIC_HEAP_SIZE, str));
	#endif

#if (!BRTOS_DYNAMIC_TASKS_ENABLED)
    string += mem_cpy(string, "\n\rQUEUE MEMORY HEAP: ");
#else
    string += mem_cpy(string, "\n\rSTATIC QUEUE MEMORY HEAP: ");
#endif

    UserEnterCritical();
    address = iQueueAddress * sizeof(OS_CPU_TYPE);
    UserExitCritical();

    (void)PrintDecimal(address, str);
    string += mem_cpy(string, &str[2]);

    string += mem_cpy(string, " of ");

    string += mem_cpy(string, PrintDecimal(QUEUE_HEAP_SIZE, str));
    string += mem_cpy(string, "\n\r");

    // End of string
    *string = '\0';
}


void OSUptimeInfo(char *string)
{
   OSTime Uptime;
   OSDate UpDate;
   CHAR8  str[6];

   UserEnterCritical();
   Uptime = OSUptime();
   UpDate = OSUpDate();
   UserExitCritical();

   string += mem_cpy(string, "\n\rBRTOS UPTIME: ");

   // Só funciona até 255 dias
   if(UpDate.RTC_Day > 0)
   {
      string += mem_cpy(string,PrintDecimal(UpDate.RTC_Day, str));
      string += mem_cpy(string, "d ");
   }

   string += mem_cpy(string,PrintDecimal(Uptime.RTC_Hour, str));
   string += mem_cpy(string, "h ");

   string += mem_cpy(string,PrintDecimal(Uptime.RTC_Minute, str));
   string += mem_cpy(string, "m ");

   string += mem_cpy(string,PrintDecimal(Uptime.RTC_Second, str));
   string += mem_cpy(string, "s\n\r");

   // End of string
   *string = '\0';
}


#if (COMPUTES_CPU_LOAD == 1)
void OSCPULoad(char *string)
{
    uint32_t percent = 0;
    uint8_t caracter = 0;
    uint8_t cent,dez;

    UserEnterCritical();
    percent = LastOSDuty;
    UserExitCritical();

    string += mem_cpy(string, "\n\rCPU LOAD: ");

    if (percent >= 1000)
    {
    	string += mem_cpy(string, "100 %");
    }
    else
    {
        cent = (percent/100);
        caracter = cent + 48;
        if(caracter != 48)
          *string++ = caracter;
        dez = ((percent - cent*100)/10);
        caracter = dez + 48;
        *string++ = caracter;
        *string++ = '.';
        caracter = (percent%10) + 48;
        *string++ = caracter;
        *string++ = '%';
    }
    string += mem_cpy(string, "\n\r");

    // End of string
    *string = '\0';
}
#endif

