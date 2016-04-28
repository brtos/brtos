/*
 * terminal_cfg.h
 *
 *  Created on: 28/04/2016
 *      Author: Universidade Federal
 */

#ifndef APP_TERMINAL_CFG_H_
#define APP_TERMINAL_CFG_H_

/************* TERMINAL CONFIG *************************/

/* Supported commands */
/* Must be listed in alphabetical order !!!! */

/*  ------ NAME ------- HELP --- */
#define COMMAND_TABLE(ENTRY) \
    ENTRY(help, "Help Command")

#define HELP_DESCRIPTION         1

#include "stdio.h"
#define TERM_PRINT(...)		printf(__VA_ARGS__); fflush(stdout);

/*******************************************************/

#endif /* APP_TERMINAL_CFG_H_ */
