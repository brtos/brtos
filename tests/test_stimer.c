/*
 * test_stimer.c
 *
 */

#include "BRTOS.h"
#include "stimer.h"

void stimer_test(void);

typedef uint16_t tick_t;

#if (PROCESSOR == X86)
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

#define PRINTF(...) printf(__VA_ARGS__);
void __cdecl _assert (const char *_Message, const char *_File, unsigned _Line)
{
 	PRINTF("Test failed at line %d\r\n", _Line);
 	fflush(stdout);
 	fflush(stderr);

 	exit(0);
}
#define TEST_ASSERT(x)  assert(x);
#endif

/* config TEST_ASSERT macro */
#ifndef TEST_ASSERT
#define TEST_ASSERT(x)		if(!(x)) while(1){}
#endif

#define run_test(test) do { test(); PRINTF("Test %d OK\r\n", tests_run); fflush(stdout); tests_run++; } while (0)
int tests_run = 0;

tick_t timer1_start;

static TIMER_CNT timer0_cb(void)
{

	PRINTF("ALL TESTS PASSED\n");
	PRINTF("Tests run: %d\n", tests_run);

	exit(1);
	return 0;
}

void test_stimer_stop_system(void)
{
	BRTOS_TIMER timer0;
	OSTimerSet (&timer0, timer0_cb, 5000);
}

static TIMER_CNT timer1_cb(void)
{
	tick_t timer1_now = OSGetTickCount();

	TEST_ASSERT((timer1_now-timer1_start) == 100);

	timer1_start = OSGetTickCount();

	return 100;
}

void test_stimer_period_100(void)
{
	BRTOS_TIMER timer1;

	OSTimerSet (&timer1, timer1_cb, 100);
	OSTimerStop (timer1, FALSE);
	timer1_start = OSGetTickCount();
	OSTimerStart (timer1, 100);
}

void task_stimer_test(void)
{
  while(1)
  {
	  printf("Tick Count: %u\r\n", (uint32_t)OSGetTickCount());
	  fflush(stdout);
	  DelayTask(1000);
  }
}

void stimer_test(void)
{
	OSTimerInit(16,30);

	/* test to stop system */
	run_test(test_stimer_stop_system);
	/* test stimer period */
	run_test(test_stimer_period_100);

	if(OSInstallTask(&task_stimer_test,"stimer test task",16,3,NULL) == OK)
	{
		if(BRTOSStart() != OK) while(1){}
	}
}



