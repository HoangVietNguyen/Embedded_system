#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include "peripheral/spi/spi.h"
#include "peripheral/gpio/gpio.h"
#include "peripheral/rcc/rcc.h"
#include "peripheral/tim/tim.h"
#include "peripheral/i2c/i2c/i2c.h"
#include "sensor_manager/dht11_driver/dht11.h"

RTC_HandleTypeDef hrtc;

int main(void)
{
  HAL_Init();

  SystemClock_Config();

  MX_GPIO_ENABLE(GPIOA);

  MX_SPI1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();

  while (1)
  {
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
