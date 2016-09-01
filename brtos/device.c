/*
 * device.c
 *
 *  Created on: 28 de jul de 2016
 *      Author: gustavo
 */

#include "device.h"
#include "string.h"



static OS_Device_t BRTOSDevices[MAX_INSTALLED_DEVICES];
static OS_Device_Control_t BRTOSDevControl[MAX_INSTALLED_DEVICES];

static const driver_installer_t OSInstalledDevices[AVAILABLE_DEVICES_TYPES] = DRIVER_LIST;
static const char *dev_types[] = DRIVER_NAMES;

OS_Device_Control_t *OSDevOpen(char *name, void *option){
	OS_CPU_TYPE    index = 0;
	OS_CPU_TYPE    idx = 0;
	char *dname = name;
	Device_Types_t type = END_TYPE;
	int8_t		   device_number = 0;

	// Zerar todos os devices na primeira vez que chamar o BRTOS_Open

	if (name == NULL){
		return (OS_Device_Control_t *)NULL;
	}

	/* Search for the device in the list of devices already installed. */
	for( index = 0; index < MAX_INSTALLED_DEVICES; index++ )
	{
		if (BRTOSDevices[index].name != NULL){
			if( strcmp((const char *)name, (const char *)BRTOSDevices[index].name) == 0 )
			{
				/* Device is already installed */
				return (OS_Device_Control_t *)&BRTOSDevControl[index];
			}
		}
	}

	/* Search for a free device block. */
	for( index = 0; index < MAX_INSTALLED_DEVICES; index++ )
	{
		if(BRTOSDevices[index].name == NULL)
		{
			/* Free device block found! */
			break;
		}
	}

	if(index < MAX_INSTALLED_DEVICES)
	{
		/* Search for the device number in ASCII format. */
		while(*name)
		{
			if((*name >= '0' ) && (*name <= '9'))
			{
				break;
			}
			name++;
		}

		/* Convert the number from its ASCII representation. */
		device_number = (uint8_t)(*name - '0');

		// todo: Verificar, pois pode haver drivers sem final numerico
		if (device_number < 0){
			// Verificar se é um GPIOx
			if( strncmp((const char *)dname, (const char *)"GPIO", (strlen((const char *)dname) - 1)) != 0 ){
				// Se o final for uma letra, mas não for um GPIO, retorna NULL
				return (OS_Device_Control_t *)NULL;
			}else{
				device_number = (int8_t)dname[4];
			}
		}

		for( idx = 0; idx < END_TYPE; idx++ )
		{
			// todo: Verificar, pois pode haver drivers sem final numerico
			// E para drivers não previstos? Utilizar um tipo de driver genérico?
			// Ou fazer os tipos de drivers disponíveis algo a ser configurado tb?
			if( strncmp((const char *)dname, (const char *)dev_types[idx], (strlen((const char *)dname) - 1)) == 0 ){
				type = idx;
				break;
			}
		}

		//BRTOSDevices[index].device_type = type;
		BRTOSDevices[index].name = dname;
		BRTOSDevices[index].DriverData = option;
		BRTOSDevControl[index].device = &BRTOSDevices[index];
		BRTOSDevControl[index].device_number = (uint8_t)device_number;


		// Como preencher os ponteiros de função para cada tipo de driver?
		OSOpenFunc func;
		for(idx=0;idx<AVAILABLE_DEVICES_TYPES;idx++)
		{
			if(OSInstalledDevices[idx].type == type &&
				OSInstalledDevices[idx].func != NULL){
				BRTOSDevices[index].device_type = OSInstalledDevices[idx].type;
				func = (OSOpenFunc)OSInstalledDevices[idx].func;
				func(&BRTOSDevControl[index], option);
			}
		}
#if 0
		switch(type){
			case UART_TYPE:
				OSOpenUART(&BRTOSDevControl[index], option);
				break;
			case SPI_TYPE:
				OSOpenSPI(&BRTOSDevControl[index], option);
				break;
			case I2C_TYPE:
				OSOpenI2C(&BRTOSDevControl[index], option);
				break;
			case GPIO_TYPE:
				OSOpenGPIO(&BRTOSDevControl[index], option);
				break;
			default:
				break;
		}
#endif

		return (OS_Device_Control_t *)&BRTOSDevControl[index];
	}else{
		return (OS_Device_Control_t *)NULL;
	}
}


size_t OSDevWrite(OS_Device_Control_t *dev, const void *string, const size_t bytes){
	return dev->api->write(dev,string,bytes);
}


size_t OSDevRead(OS_Device_Control_t *dev, void *string, const size_t bytes){
	return dev->api->read(dev,string,bytes);
}

size_t OSDevSet(OS_Device_Control_t *dev, uint32_t request, uint32_t value){
	return dev->api->set(dev,request,value);
}

size_t OSDevGet(OS_Device_Control_t *dev, uint32_t request){
	return dev->api->get(dev,request);
}
