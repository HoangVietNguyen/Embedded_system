//
// Created by Nguyen Hoang Viet on 16/07/2026.
//

#ifndef MATRIXLEDSPI_LORA_RECEIVE_H
#define MATRIXLEDSPI_LORA_RECEIVE_H

#include "stm32f1xx_hal.h"
#include "stdint.h"

#ifndef LORA_ADDRESS
#define LORA_ADDRESS        8U
#endif
#ifndef LORA_CHANNEL
#define LORA_CHANNEL        23U
#endif
#ifndef LORA_RX_BUFFER_SIZE
#define LORA_RX_BUFFER_SIZE 128U
#endif

typedef enum {
    LORA_RECEIVE_OK = 0,
    LORA_RECEIVE_ERROR_ARGUMENT,
    LORA_RECEIVE_ERROR_BUFFER_EMPTY
} LoRa_Receive_Status;

void LoRa_Receive_Init(void);
uint16_t LoRa_Receive_Available(void);
LoRa_Receive_Status LoRa_Receive_Read(uint8_t *buffer, uint16_t buffer_size, uint16_t *received_size);
void LoRa_Receive_Clear_Buffer(void);
void LoRa_Receive_RxEventCallback(uint8_t received_data);

#endif //MATRIXLEDSPI_LORA_RECEIVE_H
