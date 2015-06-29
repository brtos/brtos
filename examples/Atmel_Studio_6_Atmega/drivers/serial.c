#include "BRTOS.h"

extern BRTOS_Queue *Serial;


void Serial_Init(unsigned int baudrate)
{
	/*Set baud rate */
	UBRR0H = (INT8U)(baudrate>>8);
	UBRR0L = (INT8U)baudrate;

	/*Enable receiver and transmitter. Enable RX interrupt */
	UCSR0B = (1<<RXEN0) | (1<<TXEN0) | (1 << RXCIE0);
	/* Set frame format: 8 bit data, 1 stop bit */
	UCSR0C = (3<<UCSZ00); 								
}


void Serial_Envia_Caracter(CHAR8 data)
{
   /* Wait for empty transmit buffer */
	while (!(UCSR0A & 0x20));
    UCSR0A = 0x20;
	// Put data into buffer, sends the data */
    UDR0 = data;
}


void Serial_Envia_Frase(char *string)
{
  while(*string)
  { 
	Serial_Envia_Caracter(*string);
    string++;
  }
}

void Serial_Envia_Frase_P(char const *string)
{
	char c;
	while((c=pgm_read_byte(*string)) != 0)
	{
		Serial_Envia_Caracter(c);
		string++;
	}
}



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
#if defined(__AVR_ATmega2560__) || defined(__AVR_ATmega2561__)
//__attribute__ ((section (".lowtext")))
ISR(USART0_RX_vect, __attribute__ ( ( naked ) ))
#else
ISR(USART_RX_vect, __attribute__ ( ( naked ) ))
#endif
{
  // ************************
  // Entrada de interrupção
  // ************************
  OS_SAVE_ISR();
  OS_INT_ENTER();

  INT8U caracter = 0;  
  caracter = UDR0;

  #if (NESTING_INT == 1)
  OS_ENABLE_NESTING();
  #endif

  if (OSQueuePost(Serial,caracter) == BUFFER_UNDERRUN)
  {
    // Problema: Estouro de buffer
    (void)OSCleanQueue(Serial);
  }

  // ************************
  // Interrupt Exit
  // ************************
  OS_INT_EXIT();  
  OS_RESTORE_ISR();
  // ************************  
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
