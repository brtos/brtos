void Serial_Init(unsigned int baudrate);
void Serial_Envia_Caracter(unsigned char data);
void Serial_Envia_Frase(char *string);
void Serial_Envia_Frase_P(char const *string);

#define CR             13         //  ASCII code for carry return
#define LF             10         //  ASCII code for line feed
