/*
 * debug.h
 *
 *  Created on: 12/12/2014
 *      Author: Gustavo
 */

#ifndef DEBUG_H_
#define DEBUG_H_

#define NO_ALIGN    (INT8U)0
#define SPACE_ALIGN (INT8U)1
#define ZEROS_ALIGN (INT8U)2

void OSTaskList(char *string);
void OSAvailableMemory(char *string);
void OSUptimeInfo(char *string);
void OSCPULoad(char *string);


#endif /* DEBUG_H_ */
