#ifndef MATRIXLEDSPI_GPIO_H
#define MATRIXLEDSPI_GPIO_H

#include "main.h"
#include "driver_gpio.h"

void MX_GPIO_ENABLE(GPIO_TypeDef *port);

void MX_GPIO_INIT_Pin(GPIO_TypeDef *port, uint8_t pin, GPIO_Mode_t mode) ;

void MX_GPIO_Write_Pin(GPIO_TypeDef *port, uint8_t pin, GPIO_PinState_t mode);

GPIO_PinState_t MX_GPIO_Read_Pin(GPIO_TypeDef *port, uint8_t pin);

void MX_GPIO_Toggle_Pin(GPIO_TypeDef *port, uint8_t pin);

#endif //MATRIXLEDSPI_GPIO_H
