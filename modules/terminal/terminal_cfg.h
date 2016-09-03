/*
 * terminal_cfg.h
 *
 *  Created on: 28/04/2016
 *      Author: Universidade Federal
 */

#ifndef APP_TERMINAL_CFG_H_
#define APP_TERMINAL_CFG_H_

/************* TERMINAL CONFIG *************************/
#define TERM_INPUT_BUFSIZE 		32

/* Supported commands */
/* Must be listed in alphabetical order !!!! */

/*  ------ NAME ------- HELP --- */
#define COMMAND_TABLE(ENTRY) \
	ENTRY(help,"Help Command")     \
	ENTRY(runst,"Running stats")	\
	ENTRY(top,"System info")    \
	ENTRY(ver,"System version")

#define HELP_DESCRIPTION         1

#if WINNT
#include <stdio.h>
#define TERM_PRINT(...) printf(__VA_ARGS__); fflush(stdout);
#else
#include "printf_lib.h"
#define TERM_PRINT(...) printf_lib(__VA_ARGS__);
#endif


/*******************************************************/

#endif /* APP_TERMINAL_CFG_H_ */
