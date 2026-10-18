//
// Created by Nguyen Hoang Viet on 16/07/2026.
//

#ifndef MATRIXLEDSPI_LORA_SEND_H
#define MATRIXLEDSPI_LORA_SEND_H

#include "stm32f1xx_hal.h"
#include "stdint.h"

#ifndef LORA_ADDRESS
#define LORA_ADDRESS        8U
#endif
#ifndef LORA_CHANNEL
#define LORA_CHANNEL        23U
#endif
#ifndef LORA_USART_TIMEOUT
#define LORA_USART_TIMEOUT  1000U
#endif

typedef enum {
    LORA_SEND_OK = 0,
    LORA_SEND_ERROR,
    LORA_SEND_ERROR_ARGUMENT
} LoRa_Send_Status;

void LoRa_Send_Init(void);
LoRa_Send_Status LoRa_Send_Data(uint8_t *data, uint16_t size);
LoRa_Send_Status LoRa_Send_String(char *str);
LoRa_Send_Status LoRa_Send_Fixed_Data(uint8_t address, uint8_t channel, uint8_t *data, uint16_t size);

#endif //MATRIXLEDSPI_LORA_SEND_H
