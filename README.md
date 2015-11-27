# BRTOS - Brazilian Real-Time Operating System 
## Acrônimo: Basic Real-Time Operating System
=====

[![Build Status](https://travis-ci.org/brtos/brtos.svg)](https://travis-ci.org/brtos/brtos)

*Please see English version below.*

Sistema operacional de tempo real para microcontroladores de pequeno porte.

**Escalonamento:** Preemptivo por prioridades. Cada tarefa deverá ser associada a uma prioridade. Número máximo de tarefas instaladas = 32. 

**Recursos de gerenciamento:** Semáforos, mutex, caixas de mensagens, filas e temporizadores por software (*timers*). O mutex utiliza o protocolo priority ceiling com o intuito de evitar deadlocks e inversões de prioridade.

O sistema é escrito em linguagem C, possuindo algumas chamadas em assembly no HAL (Hardware Abstraction Layer).

**Ports oficiais:** Freescale Kinetis (ARM Cortex-M4), Freescale Coldfire V1, Freescale HCS08, ST STM32F4xx (ARM Cortex-M4F), NXP LPC11xx (ARM Cortex-M0), NXP LPC176x (ARM Cortex-M3), Renesas RX600 (RX62N), Texas Instruments MSP430, Texas Instruments Stellaris LM3S8968 (ARM Cortex-M3), Texas Instruments Stellaris LM4F120H5QR (ARM Cortex-M4F), Atmel ATMEGA328/128 e Microchip PIC18. 

**Requisitos mínimos:** O Sistema Operacional ocupa menos de 100 bytes de RAM e 2KB de memória de programa com seus recursos mínimos. Pode chegar a 1KB de RAM e 8KB de memória de programa caso sejam utilizados todos os serviços do sistema e o número máximo de tarefas (32).

**Maiores detalhes sobre o BRTOS estão disponíveis no Wiki do projeto e no [Blog do BRTOS](http://brtosblog.wordpress.com/).**

**English version**

BRTOS is a lightweight preemptive real time operating system designed for low end microcontrollers.

**Scheduler:** priority-based preemptive scheduler. A priority must be assigned for each task (aka thread). Max. number of installed tasks = 32.

**Resources:** Semaphores, mutexes, message queues, mailboxes and software timers. Mutex makes use of a priority ceiling protocol in order to avoid deadlocks and unbounded priority inversion.

BRTOS kernel is written mostly in C language, with little assembly code in the HAL file (Hardware Abstraction Layer).

**Official ports:** Freescale Kinetis (ARM Cortex-M4), Freescale Coldfire V1, Freescale HCS08, ST STM32F4xx (ARM Cortex-M4F), NXP LPC11xx (ARM Cortex-M0), NXP LPC176x (ARM Cortex-M3), Renesas RX600 (RX62N), Texas Instruments MSP430, Texas Instruments Stellaris LM3S8968 (ARM Cortex-M3), Texas Instruments Stellaris LM4F120H5QR (ARM Cortex-M4F), Atmel ATMEGA328/128 and Microchip PIC18. 

**Minimum requirements:** BRTOS kernel uses less than 100 bytes of RAM and 2KB of program memory with minimal resources. It can use up to 1KB of RAM and 8KB program memory if all system services are used and all 32 tasks are installed.

License
----

MIT
