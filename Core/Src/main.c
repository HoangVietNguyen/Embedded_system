#include "main.h"
#include <stdio.h>
#include <stdlib.h>

#include "lora_transmit/lora_config/lora_config.h"
#include "lora_transmit/lora_send/lora_send.h"
#include "peripheral/spi/spi.h"
#include "peripheral/gpio/gpio.h"
#include "peripheral/rcc/rcc.h"
#include "peripheral/tim/tim.h"
#include "peripheral/i2c/i2c/i2c.h"
#include "peripheral/rtc/rtc.h"
#include "peripheral/usart/usart.h"
#include "sensor_manager/dht11_driver/dht11.h"

#define DHT11_PORT GPIOA
#define DHT11_PIN GPIO_PIN_12

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  MX_RTC_Init();       // Khởi tạo RTC trước khi dùng RTC_Sleep_Seconds
  MX_USART1_UART_Init();

  HAL_DBGMCU_EnableDBGStopMode(); // Debugmodul erhalten als Processor geschlaft hat

  LoRa_Send_Init();
  LoRa_Config_Status config_status = LoRa_Configure_Module(LORA_ADDRESS, LORA_CHANNEL);
  if (config_status == LORA_CONFIG_OK) {
    for (int i = 0; i < 3; i++) {
      HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_SET);
      HAL_Delay(100);
      HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_RESET);
      HAL_Delay(100);
    }
  } else {
    HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_SET);
    HAL_Delay(2000);
    HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_RESET);
  }

  DHT11_Init(DHT11_PORT, DHT11_PIN);
  HAL_Delay(1500);

  while (1)
  {
    DHT11_Data data;
    char payload[32];

    if (DHT11_Read(&data) == DHT11_OK) {
      snprintf(payload, sizeof(payload), "Term: %d C, Hum: %d%%", data.temperature, data.humidity);
    } else {
      snprintf(payload, sizeof(payload), "DHT11 Read Err");
    }

    // USART1_Send_String(payload);
    LoRa_Send_String(payload);

    HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_RESET);

    HAL_Delay(2000);
  }

}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
#ifdef USE_FULL_ASSERT
#endif
