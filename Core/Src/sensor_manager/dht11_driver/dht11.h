//
// Created by Nguyen Hoang Viet on 16/07/2026.
//

#ifndef MATRIXLEDSPI_DHT11_H
#define MATRIXLEDSPI_DHT11_H

#include "stm32f1xx_hal.h"
#include "stdint.h"

typedef enum {
    DHT11_OK = 0,
    DHT11_ERROR_TIMEOUT,
    DHT11_ERROR_CHECKSUM,
    DHT11_ERROR_ARGUMENT
} DHT11_Status;

typedef struct {
    uint8_t humidity;
    uint8_t humidity_decimal;
    uint8_t temperature;
    uint8_t temperature_decimal;
} DHT11_Data;

void DHT11_Init(GPIO_TypeDef *gpio_port, uint16_t gpio_pin);
DHT11_Status DHT11_Read(DHT11_Data *data);

#endif //MATRIXLEDSPI_DHT11_H
