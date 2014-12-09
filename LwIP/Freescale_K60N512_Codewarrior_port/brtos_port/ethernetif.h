/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

/*-----------------------------------------------------------
 * Ethernet configuration.
 *-----------------------------------------------------------*/

/* MAC address configuration. */
#define configMAC_ADDR0	0x00
#define configMAC_ADDR1	0xCF
#define configMAC_ADDR2	0x52
#define configMAC_ADDR3	0x35
#define configMAC_ADDR4	0x00
#define configMAC_ADDR5	0x01

/* IP address configuration. */
#define configIP_ADDR0		172
#define configIP_ADDR1		29
#define configIP_ADDR2		7
#define configIP_ADDR3		151

/* Netmask configuration. */
#define configNET_MASK0		255
#define configNET_MASK1		255
#define configNET_MASK2		0
#define configNET_MASK3		0

/* Gateway IP address configuration. */
#define configGW_ADDR0	        172
#define configGW_ADDR1	        29
#define configGW_ADDR2	        1
#define configGW_ADDR3	        10

#define configNUM_ENET_RX_BUFFERS		8
#define configNUM_ENET_TX_BUFFERS   	8
#define configENET_RX_BUFFER_SIZE		1520
#define configENET_TX_BUFFER_SIZE		1520
#define configUSE_PROMISCUOUS_MODE		0
#define configUSE_MII_MODE              0/*FSL: using RMII mode*/
#define ETHERNET_INPUT_TASK_STACK_SIZE	768

#ifndef TSIEVB
#define configPHY_ADDRESS	0
#else
#define configPHY_ADDRESS	1
#endif

#if 1 //CW10
#define configENET_BUFFER_SIZE 1520
#endif //CW10

#if ( configENET_BUFFER_SIZE & 0x0F ) != 0
	#error configENET_BUFFER_SIZE must be a multiple of 16.
#endif

#endif /* __ETHERNETIF_H__ */
