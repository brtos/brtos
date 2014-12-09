/* Standard includes. */
#include <stdio.h>
#include <stdint.h>

/* Scheduler includes. */
#include "BRTOS.h"
#include "tasks.h"

BRTOS_TH th1, th2, th3;

int main(void)
{
  // Init your system clock here

  // Initialize BRTOS
  BRTOS_Init();

  if(InstallTask(&exec,"Teste 1",384,3,&th1) != OK)
  {
    // Oh Oh
    // Não deveria entrar aqui !!!
    while(1){};
  };


  if(InstallTask(&exec2,"Teste 2",384,5,&th2) != OK)
  {
    // Oh Oh
    // Não deveria entrar aqui !!!
    while(1){};
  };

  if(InstallTask(&exec3,"Teste 3",384,10,&th3) != OK)
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
    for(;;){};
  };

  return 0;
}

