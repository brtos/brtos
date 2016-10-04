/*
 * terminal.h
 *
 */

#ifndef APP_TERMINAL_H_
#define APP_TERMINAL_H_

typedef char *(*pf_cmd)(int, char**);

typedef struct
{
   const pf_cmd cmd_func;
   const char* cmd_name;
}cmd_t;

typedef struct _dcmd_t
{
	pf_cmd cmd_func;
	char* cmd_name;
	char* cmd_description;
	struct _dcmd_t *next;
}dcmd_t;

enum
{
	CMD_OK,
	CMD_UNKNOWN
};

#define STX  0x02
#define ETX  0x03
#define EOT  0x04
void term_putchar_install(char (*_putchar_func)(char));
void terminal_init(char (*_putchar_func)(char));
void terminal_add_cmd(dcmd_t *handler, pf_cmd cmd, char *name, char *description);

typedef char (*term_input)(char);
void terminal_set_input (term_input _input);
char terminal_input (char);

void *terminal_process(void);
void terminal_test(void);

#define CMD_FUNC(x)              char *cmd_##x(int argc, char ** argv)
#define CMD_UNUSED_ARG()        (void)argc;(void)argv;

#include "terminal_cfg.h"

#define EXPAND_AS_COMMAND_CODE_ENUM(a,c)    enum_##a,
#define EXPAND_AS_STRUCT(a,b)               char a;
#define EXPAND_AS_JUMPTABLE(a,b)            {&cmd_##a,""#a""},
#define EXPAND_AS_PROTOTYPES(a,b)           CMD_FUNC(a);
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
