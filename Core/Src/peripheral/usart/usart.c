//
// Created by Nguyen Hoang Viet on 16/07/2026.
//

#include "usart.h"
#include "main.h"
#include "string.h"

UART_HandleTypeDef huart1;
volatile uint8_t usart1_rx_byte = 0;
volatile uint8_t usart1_rx_flag = 0;

void MX_USART1_UART_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 9600;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }
    USART1_Start_Receive_IT();
}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if(huart->Instance==USART1)
    {
        /* USER CODE BEGIN USART1_MspInit 0 */

        /* USER CODE END USART1_MspInit 0 */
        /* Peripheral clock enable */
        __HAL_RCC_USART1_CLK_ENABLE();

        __HAL_RCC_GPIOA_CLK_ENABLE();
        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10    ------> USART1_RX
        */
        GPIO_InitStruct.Pin = GPIO_PIN_9;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        GPIO_InitStruct.Pin = GPIO_PIN_10;
        GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* USART1 interrupt Init */
        HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(USART1_IRQn);

        /* USER CODE BEGIN USART1_MspInit 1 */

        /* USER CODE END USART1_MspInit 1 */
    }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
    if(huart->Instance==USART1)
    {
        /* USER CODE BEGIN USART1_MspDeInit 0 */

        /* USER CODE END USART1_MspDeInit 0 */
        /* Peripheral clock disable */
        __HAL_RCC_USART1_CLK_DISABLE();

        /**USART1 GPIO Configuration
        PA9     ------> USART1_TX
        PA10    ------> USART1_RX
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

        /* USART1 interrupt DeInit */
        HAL_NVIC_DisableIRQ(USART1_IRQn);

        /* USER CODE BEGIN USART1_MspDeInit 1 */

        /* USER CODE END USART1_MspDeInit 1 */
    }
}

HAL_StatusTypeDef USART1_Send_Data(uint8_t *data, uint16_t size, uint32_t timeout)
{
    return HAL_UART_Transmit(&huart1, data, size, timeout);
}

HAL_StatusTypeDef USART1_Receive_Data(uint8_t *data, uint16_t size, uint32_t timeout)
{
    return HAL_UART_Receive(&huart1, data, size, timeout);
}

HAL_StatusTypeDef USART1_Send_String(char *str)
{
    return HAL_UART_Transmit(&huart1, (uint8_t *)str, strlen(str), HAL_MAX_DELAY);
}

HAL_StatusTypeDef USART1_Start_Receive_IT(void)
{
    return HAL_UART_Receive_IT(&huart1, (uint8_t *)&usart1_rx_byte, 1);
}

uint8_t USART1_Get_Received_Byte(void)
{
    usart1_rx_flag = 0;
    return usart1_rx_byte;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        usart1_rx_flag = 1;
        USART1_RxEventCallback(usart1_rx_byte);
        USART1_Start_Receive_IT();
    }
}

__weak void USART1_RxEventCallback(uint8_t received_data)
{
    (void)received_data;
}
