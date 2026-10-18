//
// Created by Nguyen Hoang Viet on 16/07/2026.
//

#include "lora_send.h"
#include "peripheral/usart/usart.h"
#include "string.h"

void LoRa_Send_Init(void)
{
    MX_USART1_UART_Init();
}

LoRa_Send_Status LoRa_Send_Data(uint8_t *data, uint16_t size)
{
    if (data == NULL || size == 0) {
        return LORA_SEND_ERROR_ARGUMENT;
    }

    if (USART1_Send_Data(data, size, LORA_USART_TIMEOUT) != HAL_OK) {
        return LORA_SEND_ERROR;
    }

    return LORA_SEND_OK;
}

LoRa_Send_Status LoRa_Send_String(char *str)
{
    if (str == NULL) {
        return LORA_SEND_ERROR_ARGUMENT;
    }

    return LoRa_Send_Data((uint8_t *)str, strlen(str));
}

LoRa_Send_Status LoRa_Send_Fixed_Data(uint8_t address, uint8_t channel, uint8_t *data, uint16_t size)
{
    uint8_t header[3];

    if (data == NULL || size == 0) {
        return LORA_SEND_ERROR_ARGUMENT;
    }

    header[0] = 0x00;
    header[1] = address;
    header[2] = channel;

    if (USART1_Send_Data(header, sizeof(header), LORA_USART_TIMEOUT) != HAL_OK) {
        return LORA_SEND_ERROR;
    }

    if (USART1_Send_Data(data, size, LORA_USART_TIMEOUT) != HAL_OK) {
        return LORA_SEND_ERROR;
    }

    return LORA_SEND_OK;
}
