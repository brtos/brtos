/* Buffer Descriptors -- must be aligned on a 4-byte boundary but a 
 * 16-byte boundary is recommended. To avoid playing games with the 
 * various compilers and their different extension to ANSI C, these 
 * buffers are aligned by allocating an extra line of data and 
 * adjusting the pointers in nbuf_init().
 */

#include "common.h"
#include "nbuf.h"

/*FSL: to avoid overlapping, buffers must be declared at the beggining of file*/
/*then pointers can access them correctly*/

/* Data Buffers -- must be aligned on a 16-byte boundary. To avoid 
 * playing games with the various compilers and their different 
 * extension to ANSI C, these buffers are aligned by allocating an 
 * extra line of data and adjusting the pointers in nbuf_init().
 */
static uint8_t unaligned_txbd[(sizeof(NBUF) * NUM_TXBDS) + 16];
static uint8_t unaligned_rxbd[(sizeof(NBUF) * NUM_RXBDS) + 16];
#ifdef USE_DEDICATED_TX_BUFFERS
static uint8_t unaligned_txbuffer[(TX_BUFFER_SIZE * NUM_TXBDS) + 16];
#endif
static uint8_t unaligned_rxbuffer[(RX_BUFFER_SIZE * NUM_RXBDS) + 16];

/* Pointers to alligned buffer descriptors */
static NBUF *TxNBUF;
static NBUF *RxNBUF;

/* Pointers to alligned Tx/Rx data buffers */
#ifdef USE_DEDICATED_TX_BUFFERS
static uint8_t *TxBuffer;
#endif
static uint8_t *RxBuffer;

/* Next BD indicators for static BD queue */ 
static int next_txbd;
static int next_rxbd;

/********************************************************************/
void 
nbuf_alloc(int ch)
{
	int i;
	//uint32_t *temp;

        next_txbd = 0;
        next_rxbd = 0;

	TxNBUF = (NBUF *)(((uint32_t)(unaligned_txbd)) & 0xFFFFFFF0);
	RxNBUF = (NBUF *)(((uint32_t)(unaligned_rxbd)) & 0xFFFFFFF0);

	RxBuffer = (uint8_t *)(((uint32_t)(unaligned_rxbuffer)) & 0xFFFFFFF0);
#ifdef USE_DEDICATED_TX_BUFFERS
	TxBuffer = (uint8_t *)(((uint32_t)(unaligned_txbuffer)) & 0xFFFFFFF0);
#endif
	// Initialize transmit descriptor ring
	for (i = 0; i < NUM_TXBDS; i++)
	{
		TxNBUF[i].status = 0x0000;
		TxNBUF[i].length = 0;		
	        #ifdef USE_DEDICATED_TX_BUFFERS
	           #ifdef NBUF_LITTLE_ENDIAN 
	           TxNBUF[i].data = (uint8_t *)__REV((uint32_t)&TxBuffer[i * TX_BUFFER_SIZE]);
                   #else
                   TxNBUF[i].data = (uint8_t *)(uint32_t)&TxBuffer[i * TX_BUFFER_SIZE];
                   #endif
                #endif
        
                #ifdef ENHANCED_BD
                   TxNBUF[i].ebd_status = TX_BD_IINS | TX_BD_PINS;
                #endif
	}

	// Initialize receive descriptor ring
	for (i = 0; i < NUM_RXBDS; i++)
	{
		RxNBUF[i].status = RX_BD_E;
		RxNBUF[i].length = 0;
                #ifdef NBUF_LITTLE_ENDIAN
		RxNBUF[i].data = (uint8_t *)__REV((uint32_t)&RxBuffer[i * RX_BUFFER_SIZE]);
                #else
                RxNBUF[i].data = (uint8_t *)(uint32_t)&RxBuffer[i * RX_BUFFER_SIZE];
                #endif

                #ifdef ENHANCED_BD
	        RxNBUF[i].bdu = 0x00000000;
	        RxNBUF[i].ebd_status = RX_BD_INT;
                #endif               
	}
        
	// Set the Wrap bit on the last one in the ring
	RxNBUF[NUM_RXBDS - 1].status |= RX_BD_W;
	TxNBUF[NUM_TXBDS - 1].status |= TX_BD_W;
}
/********************************************************************/
void 
nbuf_flush(int ch)
{
	int i;

	next_txbd = 0;
	next_rxbd = 0;
	
	// Reset enet hardware bd pointers also ??

	// Reset receive descriptor ring
	for (i = 0; i < NUM_RXBDS; i++)
	{
		RxNBUF[i].status = RX_BD_E;
		RxNBUF[i].length = 0;
	        #ifdef NBUF_LITTLE_ENDIAN	
		RxNBUF[i].data = (uint8_t *)__REV((uint32_t)&RxBuffer[i * RX_BUFFER_SIZE]);
	        #else
	        RxNBUF[i].data = (uint8_t *)(uint32_t)&RxBuffer[i * RX_BUFFER_SIZE];
	        #endif	
	}

	// Reset transmit descriptor ring
	for (i = 0; i < NUM_TXBDS; i++)
	{
		TxNBUF[i].status = 0x0000;
		TxNBUF[i].length = 0;
	}

	// Set the Wrap bit on the last one in the ring
	RxNBUF[NUM_RXBDS - 1].status |= RX_BD_W;
	TxNBUF[NUM_TXBDS - 1].status |= TX_BD_W;
}
/********************************************************************/
void 
nbuf_init(int ch)
{
	// Set Receive Buffer Size
	ENET_MRBR/*(ch)*/ = (uint16_t)RX_BUFFER_SIZE;  
  
 	// Point to the start of the Tx buffer descriptor queue
	ENET_TDSR/*(ch)*/ = (uint32_t)TxNBUF;
	// Point to the start of the circular Rx buffer descriptor queue
	ENET_RDSR/*(ch)*/ = (uint32_t)RxNBUF;
}
/********************************************************************/
void 
nbuf_start_rx(int ch)
{
        // Indicate Empty buffers have been produced
	ENET_RDAR/*(ch)*/ = ENET_RDAR_RDAR_MASK;

        while( !ENET_RDAR )
        {
          //If RDAR cannot be test,  
          //printf("Error with internal ENET DMA engine\n");
        }
}
/********************************************************************/
void 
enet_get_received_packet(int ch, NBUF * rx_packet)
{
	int last_buffer;
	uint16_t status;
	int index_rxbd;

	last_buffer = 0;
	rx_packet->length = 0;
	
	index_rxbd = next_rxbd;
    
	if(RxNBUF[index_rxbd].status & RX_BD_E)
	{
		printf("Under processing. SHouldnt be here\n");
		return;	
	}
        #ifdef NBUF_LITTLE_ENDIAN
        rx_packet->data = (uint8_t *)__REV((uint32_t)RxNBUF[index_rxbd].data);
        #else
        rx_packet->data = (uint8_t *)(uint32_t)RxNBUF[index_rxbd].data;
        #endif
	// Update next_rxbd pointer and mark buffers as empty again
	while(!last_buffer)
	{
		status = RxNBUF[index_rxbd].status;
	        #ifdef NBUF_LITTLE_ENDIAN	
		rx_packet->length = __REVSH(RxNBUF[index_rxbd].length);
                #else
                rx_packet->length = RxNBUF[index_rxbd].length;
                #endif
                #ifdef ENHANCED_BD
		rx_packet->ebd_status = RxNBUF[index_rxbd].ebd_status;
	            #ifdef NBUF_LITTLE_ENDIAN	
		    rx_packet->timestamp = __REV(RxNBUF[index_rxbd].timestamp);
		    rx_packet->length_proto_type = __REVSH(RxNBUF[index_rxbd].length_proto_type);
		    rx_packet->payload_checksum = __REVSH(RxNBUF[index_rxbd].payload_checksum);
                    #else
		    rx_packet->timestamp = RxNBUF[index_rxbd].timestamp;
		    rx_packet->length_proto_type = RxNBUF[index_rxbd].length_proto_type;
		    rx_packet->payload_checksum = RxNBUF[index_rxbd].payload_checksum;
                    #endif
                #endif

		last_buffer = (status & RX_BD_L);
		if(status & RX_BD_W)
		{
			RxNBUF[index_rxbd].status = (RX_BD_W | RX_BD_E);
			index_rxbd = 0;
		}
		else
		{
			RxNBUF[index_rxbd].status = RX_BD_E;
			index_rxbd++;
		}
	}
	
	// Update the global rxbd index
	next_rxbd = index_rxbd;
	
	// Put the last BD status in rx_packet->status as MISS flags and more 
	// are updated in last BD
	rx_packet->status = status;
}
/********************************************************************/
void enet_fill_txbds(int ch, NBUF * tx_packet)
{
	int num_txbds, i;
	int index_txbd;

	num_txbds = (tx_packet->length/TX_BUFFER_SIZE);
	
	index_txbd = next_txbd;
	
	if((num_txbds * TX_BUFFER_SIZE) < tx_packet->length)
	{
		num_txbds = num_txbds + 1;
	}
    
	// Fill Descriptors
	for (i = 0; i < num_txbds; i++)
	{
		
		TxNBUF[index_txbd].status = TX_BD_TC | TX_BD_R;
                #ifdef ENHANCED_BD
		TxNBUF[index_txbd].bdu = 0x00000000;
		TxNBUF[index_txbd].ebd_status = TX_BD_INT | TX_BD_TS;// | TX_BD_IINS | TX_BD_PINS;
                #endif

		if(i == num_txbds - 1)
		{
		    #ifdef NBUF_LITTLE_ENDIAN 
		    TxNBUF[index_txbd].length = __REVSH((tx_packet->length - (i*TX_BUFFER_SIZE)));
                    #else
                    TxNBUF[index_txbd].length = (tx_packet->length - (i*TX_BUFFER_SIZE));
                    #endif
		    // Set the Last bit on the last BD
		    TxNBUF[index_txbd].status |= TX_BD_L;		 
		}
		else
		{
		    #ifdef NBUF_LITTLE_ENDIAN 
		    TxNBUF[index_txbd].length = __REVSH(TX_BUFFER_SIZE);
		    #else
		    TxNBUF[index_txbd].length = TX_BUFFER_SIZE;
		    #endif
		}
		
		#ifdef USE_DEDICATED_TX_BUFFERS
		  #ifdef NBUF_LITTLE_ENDIAN
		  //Copy data to Tx buffers
                   memcpy((void *)__REV((uint32_t)TxNBUF[index_txbd].data), (void *)(((uint32_t)(tx_packet->data)) + (i*TX_BUFFER_SIZE)),
                          __REVSH(TxNBUF[index_txbd].length));  
                  #else
		  // Copy data to Tx buffers
                  memcpy((void *)(uint32_t)TxNBUF[index_txbd].data, (void *)(((uint32_t)(tx_packet->data)) + (i*TX_BUFFER_SIZE)),
                          TxNBUF[index_txbd].length);         
                  #endif
                #else
                  // Just update data pointer as data is aready there
                  #ifdef NBUF_LITTLE_ENDIAN 
                  TxNBUF[index_txbd].data = (uint8_t *)__REV((((uint32_t)(tx_packet->data)) + (i*TX_BUFFER_SIZE)));
                  #else
                  TxNBUF[index_txbd].data = (uint8_t *)(((uint32_t)(tx_packet->data)) + (i*TX_BUFFER_SIZE));
                  #endif 
                #endif

		// Wrap if this was last TxBD
		if(++index_txbd == NUM_TXBDS)
		{
			TxNBUF[NUM_TXBDS - 1].status |= TX_BD_W;
			index_txbd = 0;
		}
	}
	
	// Update the global txbd index
	next_txbd = index_txbd;
}

void 
enet_transmit_packet(int ch, NBUF * tx_packet)
{
	enet_fill_txbds(ch,tx_packet);
	
	// Indicate that Descriptors are ready to transmit 
	ENET_TDAR/*(ch)*/ = ENET_TDAR_TDAR_MASK;
}
/********************************************************************/
