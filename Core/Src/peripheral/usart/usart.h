//
// Created by Nguyen Hoang Viet on 16/07/2026.
//

#ifndef MATRIXLEDSPI_USART_H
#define MATRIXLEDSPI_USART_H

#include "stm32f1xx_hal.h"
#include "stdint.h"

extern UART_HandleTypeDef huart1;
extern volatile uint8_t usart1_rx_byte;
extern volatile uint8_t usart1_rx_flag;

void MX_USART1_UART_Init(void);
HAL_StatusTypeDef USART1_Send_Data(uint8_t *data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef USART1_Receive_Data(uint8_t *data, uint16_t size, uint32_t timeout);
HAL_StatusTypeDef USART1_Send_String(char *str);
HAL_StatusTypeDef USART1_Start_Receive_IT(void);
uint8_t USART1_Get_Received_Byte(void);
void USART1_RxEventCallback(uint8_t received_data);

#endif //MATRIXLEDSPI_USART_H
