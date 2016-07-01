/*
 * debug.h
 *
 *  Created on: 12/12/2014
 *      Author: Gustavo
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#define NO_ALIGN    (uint8_t)0
#define SPACE_ALIGN (uint8_t)1
#define ZEROS_ALIGN (uint8_t)2

void OSTaskList(char *string);
void OSRuntimeStats(char *string);
void OSAvailableMemory(char *string);
void OSUptimeInfo(char *string);
void OSCPULoad(char *string);
char *PrintDecimal(signed short val, char *buff);


#endif /* DEBUG_H_ */
