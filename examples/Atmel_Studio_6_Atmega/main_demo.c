#include "hardware.h"
#include "BRTOSConfig.h"
#include "BRTOS.h"
#include "tasks.h"
#include "serial.h"

// Declara uma estrutura de fila
OS_QUEUE SerialPortBuffer;
OS_QUEUE ADBuffer;

// Declara um ponteiro para o bloco de controle da Porta Serial
BRTOS_Queue  *Serial;
BRTOS_Queue  *AD;

BRTOS_Sem *SemTeste;

#define FOSC 16000000 // Clock Speed
#define BAUD 9600
#define MYBAUD (FOSC/16/BAUD)-1

const CHAR8 Task1Name[] PROGMEM = "System Time";
const CHAR8 Task2Name[] PROGMEM = "Tarefa Teste 1";
const CHAR8 Task3Name[] PROGMEM = "Tarefa Teste 2";
const CHAR8 Task4Name[] PROGMEM = "Tarefa Serial";

#if (!defined __GNUC__)
#define CONST
#else
#define CONST const
#endif

PGM_P CONST MainStringTable[] PROGMEM = 
{
    Task1Name,
	Task2Name,
	Task3Name,
	Task4Name
};

#define F_CPU 16000000UL 

int main_demo(void)
{
  // Clock Init
  // Make clock go 16MHz 
  CLKPR = 0x80;        // Initiate write cycle for clock setting
  CLKPR = 0x00;        // 16 MHz clock. Clock division factor = 1
  
  // Disable the watchdog timer
  wdt_disable();
	  
  // Initialize BRTOS
  BRTOS_Init();
  Serial_Init(MYBAUD);

  if (OSQueueCreate(&SerialPortBuffer,32,&Serial) != ALLOC_EVENT_OK)
  {
    // Oh Oh
    // Não deveria entrar aqui !!!
    while(1){};
  };  
  
  if (OSSemCreate(0,&SemTeste) != ALLOC_EVENT_OK)
  {
    // Oh Oh
    // Não deveria entrar aqui !!!
    while(1){};
  };      

  if(InstallTask(&System_Time,Task1Name,80,15,NULL) != OK)
  {
    // Oh Oh
    // Não deveria entrar aqui !!!
    while(1){};
  };
  
  
  if(InstallTask(&Task_2,Task2Name,80,2,NULL) != OK)
  {
    // Oh Oh
    // Não deveria entrar aqui !!!
    while(1){};
  }; 
  

  if(InstallTask(&Task_3,Task3Name,80,3,NULL) != OK)
  {
    // Oh Oh
    // Não deveria entrar aqui !!!
    while(1){};
  };
  
  
  if(InstallTask(&Task_Serial,Task4Name,100,1,NULL) != OK)
  {
    // Oh Oh
    // Não deveria entrar aqui !!!
    while(1){};
  };


  // Start Task Scheduler
  if(BRTOSStart() != OK)
  {
    // Oh Oh
    // Não deveria entrar aqui !!!
    while(1){};
  };

  for(;;){};


}
