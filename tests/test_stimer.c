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

tick_t timer0_start;
tick_t timer1_start;
tick_t timer2_start;

tick_t get_diff_time(tick_t timer_now, tick_t timer_start)
{
	tick_t diff_time;

	if(timer_now < timer_start)
	{
		diff_time = timer_now + (TICK_COUNT_OVERFLOW-timer_start);
	}else
	{
		diff_time = timer_now-timer_start;
	}

	return diff_time;
}

static TIMER_CNT timer0_cb(void)
{
	tick_t timer0_now = OSGetTickCount();
	tick_t diff_time;

	if(timer0_now < 64000)
	{
		diff_time = get_diff_time(timer0_now, timer0_start) + 64000;
	}else
	{
		diff_time = 64000;
	}

	TEST_ASSERT(diff_time >= 64000);

	PRINTF("ALL TESTS PASSED\n");
	PRINTF("Tests run: %d\n", tests_run);
	return 0;
}

void test_stimer_stop_system(void)
{
	BRTOS_TIMER timer0;
	OSTimerSet (&timer0, timer0_cb, 64000);
	timer0_start = OSGetTickCount();
}

static TIMER_CNT timer1_cb(void)
{
	tick_t timer1_now = OSGetTickCount();
	tick_t diff_time = get_diff_time(timer1_now, timer1_start);

	/* tol = 10*/
	TEST_ASSERT((diff_time > 90) && (diff_time < 110));

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

static TIMER_CNT timer2_cb(void)
{
	OS_SR_SAVE_VAR;

	tick_t timer2_now = OSGetTickCount();

	tick_t diff_time = get_diff_time(timer2_now,timer2_start);

	/* increment tick counter by 5 */
	OSEnterCritical();
		OSIncCounter();
		OSIncCounter();
		OSIncCounter();
		OSIncCounter();
		OSIncCounter();
	OSExitCritical();

	TEST_ASSERT((diff_time > 90) && (diff_time < 110));

	timer2_start = OSGetTickCount();

	return 100;
}

void test_stimer_period_100_taking_5ticks(void)
{
	BRTOS_TIMER timer2;

	OSTimerSet (&timer2, timer2_cb, 100);
	OSTimerStop (timer2, FALSE);
	timer2_start = OSGetTickCount();
	OSTimerStart (timer2, 100);
}

void task_stimer_test(void)
{
  while(1)
  {
	  printf("Tick Count: %u\r\n", (uint32_t)OSGetTickCount());
	  fflush(stdout);
	  DelayTask(10000);
  }
}

void stimer_test(void)
{
	OSTimerInit(16,30);

	/* test to stop system */
	run_test(test_stimer_stop_system);
	/* test stimer period */
	run_test(test_stimer_period_100);
	/* test stimer period */
	run_test(test_stimer_period_100_taking_5ticks);

	if(OSInstallTask(&task_stimer_test,"stimer test task",16,3,NULL) == OK)
	{
		if(BRTOSStart() != OK) while(1){}
	}
}



