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
#include "sensor_manager/dht11_driver/dht11.h"
#include "sensor_manager/hc-sr04-driver/hc_sr04.h"

#define HC_SR04_TRIG_PORT GPIOB
#define HC_SR04_TRIG_PIN  GPIO_PIN_6
#define HC_SR04_ECHO_PORT GPIOB
#define HC_SR04_ECHO_PIN  GPIO_PIN_7

#define DHT11_PORT GPIOB
#define DHT11_PIN  GPIO_PIN_8

#define TARGET_LORA_CHANNEL  0x17U
#define TARGET_LORA_ADDRESS  0x08U

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

  HC_SR04_Init(HC_SR04_TRIG_PORT, HC_SR04_TRIG_PIN,
               HC_SR04_ECHO_PORT, HC_SR04_ECHO_PIN);

  DHT11_Init(DHT11_PORT, DHT11_PIN);

  while (1)
  {
    HC_SR04_Data distance;
    char distance_payload[32];

    DHT11_Data dht11;
    char dht11_payload_temp[32];
    char dht11_payload_huminity[32];

    if (HC_SR04_Read(&distance) == HC_SR04_OK) {
      // Dùng %d.%d thay %.1f để tránh giới hạn printf float của newlib-nano
      uint8_t dist_int = (int)distance.distance_cm;
      uint8_t dist_dec = (int)((distance.distance_cm - dist_int) * 10);
      snprintf(distance_payload, sizeof(distance_payload), "Dist: %d.%d cm\n", dist_int, dist_dec);

    } else {
      snprintf(distance_payload, sizeof(distance_payload), "HC-SR04 Err");
    }

    if (distance.distance_cm < 5) {
      // HAL_UART_Transmit(&huart1, distance_payload, strlen(distance_payload), HAL_MAX_DELAY);
      // HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_SET);
      // HAL_Delay(50);
      // HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_RESET);

      LoRa_Send_Status lora_send_status =
        LoRa_Send_Fixed_Data(
          TARGET_LORA_ADDRESS,
          TARGET_LORA_CHANNEL,
          (uint8_t *)distance_payload,
          strlen(distance_payload)
          );
      if (lora_send_status == LORA_SEND_OK) {
        HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_RESET);
      }
    }

    DHT11_Status dht_status = DHT11_Read(&dht11);
    if (dht_status == DHT11_OK) {
      LoRa_Send_Status lora_send_status = LORA_SEND_OK;
      uint8_t temperature = dht11.temperature;
      snprintf(dht11_payload_temp, sizeof(dht11_payload_temp), "Temp: %d", (int) temperature);
       lora_send_status = LoRa_Send_Fixed_Data(
         TARGET_LORA_ADDRESS,
         TARGET_LORA_CHANNEL,
         (uint8_t *)dht11_payload_temp,
         strlen(dht11_payload_temp)
         );
      if (lora_send_status == LORA_SEND_OK) {
        HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_RESET);
      }

      HAL_Delay(500);

      uint8_t humidity = dht11.humidity;
      snprintf(dht11_payload_huminity, sizeof(dht11_payload_huminity), "Humidity: %d", (int) humidity);
      lora_send_status =
      LoRa_Send_Fixed_Data(
        TARGET_LORA_ADDRESS,
        TARGET_LORA_CHANNEL,
        (uint8_t *)dht11_payload_huminity,
        strlen(dht11_payload_huminity)
        );
      if (lora_send_status == LORA_SEND_OK) {
        HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_SET);
        HAL_Delay(50);
        HAL_GPIO_WritePin(LED_TEST_GPIO_Port, LED_TEST_Pin, GPIO_PIN_RESET);
      }
    }

    // Put processor into STOP mode for 1 second using RTC alarm wakeup
    // RTC_Sleep_Seconds(60);
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
