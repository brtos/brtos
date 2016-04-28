/*
 * terminal.h
 *
 *  Created on: 28/04/2016
 *      Author: Universidade Federal
 */

#ifndef APP_TERMINAL_H_
#define APP_TERMINAL_H_

typedef void (*pf_cmd)(char*);

typedef struct
{
   const pf_cmd cmd_func;
   const char* cmd_name;
}cmd_t;

enum
{
	CMD_OK,
	CMD_UNKNOWN
};

void term_putchar_install(char (*_putchar_func)(char));

typedef char (*term_input)(char);
void terminal_set_input (term_input _input);
char terminal_input (char);

void terminal_process(void);

#define CMD_FUNC(x)              void cmd_##x(char * arg)
#define CMD_UNUSED_ARG()         (void)arg;

#include "terminal_cfg.h"

#define EXPAND_AS_COMMAND_CODE_ENUM(a,c)    enum_##a,
#define EXPAND_AS_STRUCT(a,b)               char a;
#define EXPAND_AS_JUMPTABLE(a,b)            {&cmd_##a,""#a""},
#define EXPAND_AS_PROTOTYPES(a,b)           void cmd_##a(char*);
#define EXPAND_AS_DESCRIPTIONS(a,b)         b,

enum {
    COMMAND_TABLE(EXPAND_AS_COMMAND_CODE_ENUM)
};

typedef  struct{
    COMMAND_TABLE(EXPAND_AS_STRUCT)
} size_struct_t;

#define NUMBER_OF_COMMANDS sizeof(size_struct_t)

COMMAND_TABLE(EXPAND_AS_PROTOTYPES)

#endif /* APP_TERMINAL_H_ */
