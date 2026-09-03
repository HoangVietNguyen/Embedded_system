#include "gpio.h"

/**
  * @brief GPIO Initialization Function (Die Bibliothek, nämlich 'driver_gpio')
  * @retval None
  */

void MX_GPIO_ENABLE(GPIO_TypeDef *port) {
  driver_gpio_clock_enable(port);
}

void MX_GPIO_INIT_Pin(GPIO_TypeDef *port, uint8_t pin, GPIO_Mode_t mode) {
  if (port == NULL || pin > 15) {
    return;
  }
  driver_gpio_init_pin(port, pin, mode);
}

void MX_GPIO_Write_Pin(GPIO_TypeDef *port, uint8_t pin, GPIO_PinState_t state) {
  if (port == NULL || pin > 15) {
    return;
  }
  driver_gpio_write_pin(port, pin, state);
}

GPIO_PinState_t MX_GPIO_Read_Pin(GPIO_TypeDef *port, uint8_t pin) {
  if (port == NULL || pin > 15) {
    return DRIVER_GPIO_PIN_RESET;
  }
  return driver_gpio_read_pin(port, pin);
}

void MX_GPIO_Toggle_Pin(GPIO_TypeDef *port, uint8_t pin) {
  if (port == NULL || pin > 15) {
    return;
  }
  driver_gpio_toggle_pin(port, pin);
}
