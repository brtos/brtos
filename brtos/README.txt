== BRTOS 1.1 Changelog ==

- Improved scheduler 	       - New sucessive aproximation scheduler
- Priority information changed - Now the highest priority is 31 or 15, depending on the NUMBER_OF_PRIORITIES define
- Improved Post functions      - Now, post function executes the ChangeContext() if it is called from a task
- Improved Pend functions      - Erase the time to wait value from the task context when no timeout occurs
- Improved code for porting to another architecture
- Minor bugs corrections


== BRTOS 1.2 Changelog ==

- Improved code for porting to another architecture
- Improved semaphore / mutex / mailbox / queue services
- Support for selecting nesting/no nesting interrupt through define in the HAL for Coldfire V1 port
- New OS Trace feature (permit to collect data on how your application is behaving)
- Minor bugs corrections


== BRTOS 1.3 Changelog ==

- Improved block / unblock task functions
- Improved mutex - Now one mutex has its own priority
- Improved queue functions - Know there is no calls to malloc / calloc functions. Queue heap size must be defined in the BRTOS.h


== BRTOS 1.4 Changelog ==

- Improved semaphore / mutex / mailbox / queue services
- Improved code for speed - the VERBOSE define can be used to avoid saving unnecessary task data
- Now it is possible to disable the compute of the CPU load
- Improved queue function calls - No more need to pass the queue buffer in the Pend/Post functions


== BRTOS 1.45 Changelog ==

- Mutex bug correction - Now it is possible to call multiple mutex in a task to acquire more than one resource
- Improved serial driver - The TX polling was removed. Now TX uses interrupt.
- Improved include and remove of tasks from the ReadyList (better for processors of 8 and 16 bits)


== BRTOS 1.50 Changelog ==

- Stack size of the InstallTask function is te real stack size (in previous versions of the BRTOS the minimum stack size is added to the user stack)
- Better stack management for interrupts preemption
- New ERROR_CHECK define. The system becomes more fault tolerant With this definition
- TickCounterOverflow changed to 64000
- New BRTOSConfig.h file. This file contains all the user settings.
- Files event.c, event.h and queue.h removed from BRTOS. The event.c is in the BRTOS.c and the .h files into the BRTOS.h file.
- InstallTask was moved to the BRTOS.c core functions. Now there are a new function to be ported (CreateVirtualStack).
- New Idle stack size definition in the BRTOSConfig.h file


== BRTOS 1.60 Changelog ==

- add support for PIC18 microcontroller
- add support for ATMEGA microcontroller
- Add support for low power mode in ATMEGA microcontrollers
- New OS start function - Now to start the BRTOS we call BRTOSStart();
- Add support for stack growth in both directions
- New HAL define (OS_SR_SAVE_VAR) for save status register info


== BRTOS 1.61 Changelog ==
- minor bug correction - timeout greater than 1000 ticks could lead to a erroneous time to wait value
- better const support for PIC18 and ATMEGA microcontrollers (strings into the FLASH)
- New HAL define (OS_SR_SAVE_VAR) for save status register info
- New HAL define for ATMEGA and PIC18 microcontrollers (TEXT_BUFFER_SIZE) for RAM buffer to copy string from FLASH to RAM


== BRTOS 1.62 Changelog ==
- minor bug correction - lack of OSExitCritical() call in some XXCreate functions of the OS services
- now with SVN support
- svn checkout http://brtos.googlecode.com/svn/trunk/brtos (for kernel code checkout)
- svn checkout http://brtos.googlecode.com/svn/trunk/hal/Compiler_MCU (for HAL code checkout), Where Compiler_MCU must be changed with the required compiler and MCU (e.g. CodeWarrior_CFV1)

== BRTOS 1.63 Changelog ==
- new PriorityType - Now the user can select 8, 16 or 32 priority levels (better usage of memory and processor)

== BRTOS 1.64 Changelog ==
- minor bug correction - in some CISC architectures the timer linked list could cause an undesirable write to RAM memory and/or CPU registers

== BRTOS 1.65 Changelog ==
- Optimized scheduler support for Coldfire and ARM MCUs 
- Added support for ARM Cortex-M MCUs

== BRTOS 1.66 Changelog ==
- Added support for 32 bits tick timer register in cpu load computing (commonly used in ARM Cortex-M MCUs)

== BRTOS 1.67 Changelog ==
- now virtual stack and queue stack are declared in OS_CPU_TYPE. This helps the BRTOS memory allocation process to avoid memory misalignment.
- some kernel variables are now declared as volatile. This enables BRTOS to successfully compile with gcc compiler optimizations.

== BRTOS 1.68 Changelog ==
- The method to calculate the CPU load has been changed. Now we use a very simple method based on how many times the system goes into wait task in 1 milisecond.

== BRTOS 1.69 Changelog ==
- Minor fix for mutex acquire and release functions

== BRTOS 1.70 Changelog ==
- Minor fix in order to correct compiler issues in some IDEs

== BRTOS 1.75 Changelog ==
- Added a function to get the current tick count
- Added support for dinamic queue (now it is possible to allocate and deallocate queues with different sizes of data)
- Added support for parameters in the install task function

== BRTOS 1.76 Changelog ==
- Tick counter variable protection. Now it is impossible to change the tick counter value inside a user task

== BRTOS 1.77 Changelog ==
- Added soft timers service. Beta version.

== BRTOS 1.78 Changelog ==
- Fix in TimerStop function for soft timers service.

== BRTOS 1.79 Changelog ==
- Added support for binary semaphores.
