#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"

err_t ethernetif_init(struct netif *netif);

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



#endif 
