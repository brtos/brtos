/*
 * device.h
 *
 *  Created on: 20 de jul de 2016
 *      Author: gustavo
 */

#ifndef BRTOS_DEVICE_H_
#define BRTOS_DEVICE_H_

/* Standard includes. */
#include <stdint.h>
#include <stddef.h>

#include "BRTOS.h"
#include "OSDevConfig.h"

/**
 * @brief Static device information (In ROM) Per driver instance
 * @param name name of the device
 * @param init init function for the driver
 * @param config_info address of driver instance config information
 */
typedef struct BRTOS_Device_{
	char *name;
	Device_Types_t device_type;
	unsigned long base_address;
	void *DriverData;
}OS_Device_t;

/* Device handles are void * for data hiding purposes. */
typedef const void *Device_Descriptor_t;

/* Types that define read(), write(), set() and get() functions. */
typedef size_t (*Device_Control_write_t)(Device_Descriptor_t const device, const void *pvBuffer, const size_t xBytes );
typedef size_t (*Device_Control_read_t)(Device_Descriptor_t const device, void *pvBuffer, const size_t xBytes );
typedef size_t (*Device_Control_set_t)(Device_Descriptor_t const device, uint32_t ulRequest, uint32_t value );
typedef size_t (*Device_Control_get_t)(Device_Descriptor_t const device, uint32_t ulRequest);


typedef struct device_api_t_{
	Device_Control_write_t 	write;
	Device_Control_read_t 	read;
	Device_Control_set_t 	set;
	Device_Control_get_t 	get;
}device_api_t;
/**
 * @brief Runtime device structure (In memory) Per driver instance
 * @param device_config Build time config information
 * @param driver_api pointer to structure containing the API functions for
 * the device type. This pointer is filled in by the driver at init time.
 * @param driver_data river instance data. For driver use only
 */
typedef struct BRTOS_Device_Control_t_{
	OS_Device_t 		*device;
	int8_t 				device_number;			// Quando houver mais de uma uart por exemplo
	const device_api_t 	*api;
}OS_Device_Control_t;


typedef void (*OSOpenFunc)(OS_Device_Control_t *dev, void *parameters);

typedef struct
{
	int 		type;
	void 		*func;
}driver_installer_t;

#if 0
#ifndef OS_UART_DEVICE
#define OSOpenUART(x,y)
#else
void OSOpenUART(OS_Device_Control_t *dev, void *parameters);
#endif

#ifndef OS_SPI_DEVICE
#define OSOpenSPI(x,y)
#else
void OSOpenSPI(OS_Device_Control_t *dev, void *parameters);
#endif

#ifndef OS_I2C_DEVICE
#define OSOpenI2C(x,y)
#else
void OSOpenI2C(OS_Device_Control_t *dev, void *parameters);
#endif

#ifndef OS_GPIO_DEVICE
#define OSOpenGPIO(x,y)
#else
void OSOpenGPIO(OS_Device_Control_t *dev, void *parameters);
#endif
#endif


OS_Device_Control_t *OSDevOpen(char *name, void *option);
size_t OSDevWrite(OS_Device_Control_t *dev, const void *string, const size_t bytes);
size_t OSDevRead(OS_Device_Control_t *dev, void *string, const size_t bytes);
size_t OSDevSet(OS_Device_Control_t *dev, uint32_t request, uint32_t value );
size_t OSDevGet(OS_Device_Control_t *dev, uint32_t request);

#define OSGPIOWrite(x,y,z) OSDevWrite(x, (const void *)(y), z)
#define OSGPIORead(x,y) OSDevRead(x, y, 0)

// UART types
#define INF_TIMEOUT		0
#ifndef NO_TIMEOUT
#define NO_TIMEOUT                  (ostick_t)(MAX_TIMER - 1)
#endif

typedef enum{
	CTRL_ACQUIRE_READ_MUTEX = 10,
	CTRL_ACQUIRE_WRITE_MUTEX,
	CTRL_RELEASE_WRITE_MUTEX,
	CTRL_RELEASE_READ_MUTEX
}ctrl_t;

typedef enum{
	UART_PAR_NONE,
	UART_PAR_ODD,
	UART_PAR_EVEN
}uart_par_t;

typedef enum{
	UART_STOP_ONE = 1,
	UART_STOP_TWO,
}uart_stop_t;

typedef enum{
	UART_POLLING,
	UART_IRQ,
}uart_irq_t;

typedef enum{
	UART_BAUDRATE,
	UART_PARITY,
	UART_STOP_BITS,
	UART_QUEUE_SIZE,
	UART_TIMEOUT
}uart_request_t;

typedef struct uart_config_t_{
	int 		baudrate;
	uart_par_t 	parity;
	uart_stop_t stop_bits;
	uart_irq_t 	polling_irq;
	int 		queue_size;
	ostick_t 	timeout;
	bool		read_mutex;
	bool		write_mutex;
}uart_config_t;


// GPIO types
typedef enum{
	GPIO_DIR_IN,
	GPIO_DIR_OUT
}gpio_dir_t;

#define GPIO_PIN_0              0x00000001
#define GPIO_PIN_1              0x00000002
#define GPIO_PIN_2              0x00000004
#define GPIO_PIN_3              0x00000008
#define GPIO_PIN_4              0x00000010
#define GPIO_PIN_5              0x00000020
#define GPIO_PIN_6              0x00000040
#define GPIO_PIN_7              0x00000080
#define GPIO_PIN_8              0x00000100
#define GPIO_PIN_9              0x00000200
#define GPIO_PIN_10             0x00000400
#define GPIO_PIN_11             0x00000800
#define GPIO_PIN_12             0x00001000
#define GPIO_PIN_13             0x00002000
#define GPIO_PIN_14             0x00004000
#define GPIO_PIN_15             0x00008000
#define GPIO_PIN_16             0x00010000
#define GPIO_PIN_17             0x00020000
#define GPIO_PIN_18             0x00040000
#define GPIO_PIN_19             0x00080000
#define GPIO_PIN_20             0x00100000
#define GPIO_PIN_21             0x00200000
#define GPIO_PIN_22             0x00400000
#define GPIO_PIN_23             0x00800000
#define GPIO_PIN_24             0x01000000
#define GPIO_PIN_25             0x02000000
#define GPIO_PIN_26             0x04000000
#define GPIO_PIN_27             0x08000000
#define GPIO_PIN_28             0x10000000
#define GPIO_PIN_29             0x20000000
#define GPIO_PIN_30             0x40000000
#define GPIO_PIN_31             0x80000000


typedef struct gpio_config_t_{
	unsigned long used_pins_in;
	unsigned long used_pins_out;
	unsigned long pull;
	unsigned long irq_pins;
}gpio_config_t;

typedef struct mux_gpio_config_t_{
	unsigned long base;
	unsigned long pin;
	unsigned long pin_number;
}mux_gpio_config_t;

//
//! Edge-aligned PWM High-true pulses
//
#ifndef PWM_MODE_EDGE_ALIGNED1
#define PWM_MODE_EDGE_ALIGNED1  0x00000000
#endif

//
//! Edge-aligned PWM Low-true pulses
//
#ifndef PWM_MODE_EDGE_ALIGNED2
#define PWM_MODE_EDGE_ALIGNED2  0x00000001
#endif

//
//! Center-aligned PWM High-true pulses
//
#ifndef PWM_MODE_CENTER_ALIGNED1
#define PWM_MODE_CENTER_ALIGNED1 0x00000002
#endif

//
//! Center-aligned PWM Low-true pulses
//
#ifndef PWM_MODE_CENTER_ALIGNED2
#define PWM_MODE_CENTER_ALIGNED2 0x00000003
#endif

typedef struct pwm_config_t_{
	int 		  *channel_list;
	int			  frequency;
	int			  mode;
	int		  	  *init_duty;
}pwm_config_t;

#endif /* BRTOS_DEVICE_H_ */
