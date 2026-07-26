#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lora_transmit/lora_config/lora_config.h"
#include "lora_transmit/lora_send/lora_send.h"
#include "peripheral/spi/spi.h"
#include "peripheral/gpio/gpio.h"
#include "peripheral/rcc/rcc.h"
#include "peripheral/tim/tim.h"
#include "peripheral/i2c/i2c/i2c.h"
#include "peripheral/rtc/rtc.h"
#include "peripheral/usart/usart.h"
#include "sensor_manager/hc-sr04-driver/hc_sr04.h"

#define HC_SR04_TRIG_PORT GPIOB
#define HC_SR04_TRIG_PIN  GPIO_PIN_6
#define HC_SR04_ECHO_PORT GPIOB
#define HC_SR04_ECHO_PIN  GPIO_PIN_7

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

  // LoRa_Send_Init();
  // LoRa_Config_Status config_status = LoRa_Configure_Module(LORA_ADDRESS, LORA_CHANNEL);
  // if (config_status == LORA_CONFIG_OK) {
  //   for (int i = 0; i < 3; i++) {
  //     HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_SET);
  //     HAL_Delay(100);
  //     HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_RESET);
  //     HAL_Delay(100);
  //   }
  // } else {
  //   HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_SET);
  //   HAL_Delay(2000);
  //   HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_RESET);
  // }

  HC_SR04_Init(HC_SR04_TRIG_PORT, HC_SR04_TRIG_PIN,
               HC_SR04_ECHO_PORT, HC_SR04_ECHO_PIN);

  while (1)
  {
    HC_SR04_Data distance;
    char payload[32];

    if (HC_SR04_Read(&distance) == HC_SR04_OK) {
      // Dùng %d.%d thay %.1f để tránh giới hạn printf float của newlib-nano
      int dist_int = (int)distance.distance_cm;
      int dist_dec = (int)((distance.distance_cm - dist_int) * 10);
      snprintf(payload, sizeof(payload), "Dist: %d.%d cm\n", dist_int, dist_dec);

    } else {
      snprintf(payload, sizeof(payload), "HC-SR04 Err");
    }

    if (distance.distance_cm < 5) {
      HAL_UART_Transmit(&huart1, payload, strlen(payload), HAL_MAX_DELAY);
      HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_SET);
      HAL_Delay(100);
      HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_RESET);
    }

    // Wait 60ms before next measurement (HC-SR04 minimum cycle time)
    HAL_Delay(60);
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
