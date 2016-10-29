/*
 * terminal.c
 *
 */

#include "terminal.h"
#include "stddef.h"
#include "stdint.h"
#include "string.h"

#ifndef TERM_PRINT
#define PRINTF_BUFSIZE 		64
static int printf_idx = 0;
static char printf_buf[PRINTF_BUFSIZE];

static char __putchar_buf(char c)
{
	printf_buf[printf_idx++%PRINTF_BUFSIZE] = c;
	return c;
}
#endif
static char (*putchar_func)(char);

void term_putchar_install(char (*_putchar_func)(char))
{
	if(_putchar_func == NULL) return;
	putchar_func = _putchar_func;
}


#define STRCMP(a,b)		strcmp(a,b)
#define TERM_FLUSH(a)	memset(a,0x00, sizeof(a))


static term_input input = NULL;
dcmd_t *cmd_head = NULL;

void terminal_set_input (term_input _input)
{
	input = _input;
}

void terminal_init(char (*_putchar_func)(char))
{
	#if defined(TERM_PRINT) && defined(CUSTOM_PRINTF)
	printf_install_putchar(_putchar_func);
	#endif
	putchar_func('\n');
	putchar_func('\r');
	putchar_func('>');
	putchar_func('>');
}

static int term_in_idx = 0;
static int term_tmp_idx = 0;
static char discard_char = 0;
static volatile int cpi=0;
static char term_in[TERM_INPUT_BUFSIZE];
static char last_term_in[TERM_INPUT_BUFSIZE];

char terminal_input (char c)
{
	if(input != NULL)
	{
		input(c);
	}

	if (c != 0x7F){
		if (c != UP_KEY_CHAR){
			if (!discard_char){
				putchar_func(c);
			}else{
				discard_char--;
				if (!discard_char){
					c = UP_KEY_CHAR;
				}else{
					return 0;
				}
			}
		}else{
			discard_char = CHARS_TO_DISCARD;
			return 0;
		}
		if (c != 13){
			if (term_tmp_idx >= 0){
				term_tmp_idx++;
			}else{
				term_tmp_idx = 1;
			}
		}else{
			term_tmp_idx = 0;
		}
	}else{
		term_tmp_idx--;
		if (term_tmp_idx >= 0){
			putchar_func(c);
		}
	}


	if (term_tmp_idx >= 0){
		if ((c=='\b' || c==0x7F) && term_in_idx > 0)
		{
			term_in[term_in_idx--] = '\0';
			return 0;
		}
		else
		{
			// if up key pressed
			if (c == UP_KEY_CHAR){
  			  // erase last chars in case of up key pressed
  			  while(term_in_idx)
  			  {
  				  putchar_func(0x7F);
  				  term_in_idx--;
  			  }
  			  term_in_idx = 0;
  			  if (last_term_in[0]){
				  while(term_in_idx <= cpi){
					  putchar_func(last_term_in[term_in_idx++]);
				  }

				  term_in_idx = 0;
				  term_tmp_idx = 0;
				  while(term_in_idx <= cpi)
				  {
					  term_in[term_in_idx] = last_term_in[term_in_idx];
					  term_in_idx++;
					  term_tmp_idx++;
				  }
				  term_tmp_idx--;
				  term_in[term_in_idx] = '\0';
				  term_in_idx--;
  			  }
			  return 0;
			}else{
				term_in[term_in_idx++] = c;
				if(c == '\n' || c == '\r' || (term_in_idx==(TERM_INPUT_BUFSIZE-1)))
				{
					term_in[term_in_idx-1]='\0';
					return 1;
				}else
				{
					return 0;
				}
			}
		}
	}else{
		return 0;
	}
}


/* search command in command table by command name */
// static uint8_t search_cmd (char * c);

/* table of commands */
const cmd_t cmds[NUMBER_OF_COMMANDS] =
{
    COMMAND_TABLE(EXPAND_AS_JUMPTABLE)
};

static uint8_t search_cmd (char * c)
{

	int8_t inf = 0;  // inferior limit
	int8_t sup = NUMBER_OF_COMMANDS-1; // superior limit
	int8_t meio;

  while(inf <= sup){
	meio = (uint8_t)((inf + sup) >> 1);
	if (STRCMP(cmds[meio].cmd_name, c) < 0)
	{
	  inf = (uint8_t)(meio + 1);
	}
	else
	{
	  if (STRCMP(cmds[meio].cmd_name, c) > 0)
	  {
		sup = (uint8_t)(meio-1);
	  }else
	  {
		return meio; // command index found
	  }
	}
  }
  // Teste dos comandos dinâmicos
  if (cmd_head != NULL){
	dcmd_t *cmd_p = cmd_head;
	uint8_t idx = NUMBER_OF_COMMANDS;
	while(cmd_p != NULL){
		if (STRCMP(cmd_p->cmd_name,c) == 0){
			return idx;
		}
		cmd_p = cmd_p->next;
		idx++;
	}
  }

  return 0; // command index not found, return default index 0
}

#if (HELP_DESCRIPTION == 1)
static const char* ListOfCmdDesc[NUMBER_OF_COMMANDS] =
{
  COMMAND_TABLE(EXPAND_AS_DESCRIPTIONS)
};
#endif

CMD_FUNC(help)
{
	CMD_UNUSED_ARG();
	uint8_t c = 0;
#if (HELP_DESCRIPTION == 1)
	TERM_PRINT("If you have not typed \"help\", the command you typed was not found!\r\n");
	TERM_PRINT("Else, here is the list of supported commands: \r\n");
	TERM_PRINT("----------------\n\r");
	for (c = 0; c < NUMBER_OF_COMMANDS; c++)
	{
			TERM_PRINT("%s : ", cmds[c].cmd_name);
			TERM_PRINT("%s\n\r", ListOfCmdDesc[c]);
	}
	// Teste dos comandos dinâmicos
	if (cmd_head != NULL){
		dcmd_t *cmd_p = cmd_head;
		while(cmd_p != NULL){
			TERM_PRINT("%s : ", cmd_p->cmd_name);
			TERM_PRINT("%s\n\r", cmd_p->cmd_description);
			cmd_p = cmd_p->next;
		}
	}
	TERM_PRINT("----------------\n\r");
#endif
	return NULL;
}

void *terminal_process(void)
{
	uint8_t idx_start = 0, c;
	char *arg;
	char *ret = NULL;
	uint8_t argc = 0;

	#define TERM_ARGS_MAX_COUNT		6
	char* 	argv[TERM_ARGS_MAX_COUNT];

	memset(argv,0,sizeof(argv));

	while(term_in[idx_start]== ' ' || term_in[idx_start]== '\t') idx_start++;

	arg = &term_in[idx_start];
	arg = strtok(arg," \t\r\n");

	while (arg != NULL)
	{
		//TERM_PRINT("%s\n",arg);
		argv[argc++] = arg;
		arg = strtok (NULL, " \t\r\n");
	}

	if(argv[0] != NULL)
	{
		c = search_cmd(argv[0]);

		TERM_PRINT("\r\n");
		if (c < NUMBER_OF_COMMANDS){
			ret = cmds[c].cmd_func(argc, argv);
		}else{
			int idx = c - NUMBER_OF_COMMANDS;
			dcmd_t *cmd_p = cmd_head;
			while(idx){
				cmd_p = cmd_p->next;
				idx--;
			}
			ret = cmd_p->cmd_func(argc, argv);
		}
	    // copy last command
	    cpi=0;
	    while((term_in[cpi] != 0) || (term_in[cpi+1] != 0))
	    {
	      if (!term_in[cpi]){
	    	  last_term_in[cpi] = 0x20;
	      }else{
	    	  last_term_in[cpi] = term_in[cpi];
	      }
		  cpi++;
	    }
	    last_term_in[cpi] = '\0';
		TERM_PRINT("\r\n>>");
	}

	TERM_FLUSH(term_in);
	term_in_idx = 0;
	return (void *)ret;
}

void terminal_add_cmd(dcmd_t *handler, pf_cmd cmd, char *name, char *description){
	dcmd_t *cmd_p = cmd_head;
	if (cmd_head == NULL){
		cmd_head = handler;
	}else{
		while(cmd_p->next != NULL){
			cmd_p = cmd_p->next;
		}
		cmd_p->next = handler;
	}
	handler->cmd_func = cmd;
	handler->cmd_name = name;
	handler->cmd_description = description;
	handler->next = NULL;
}
