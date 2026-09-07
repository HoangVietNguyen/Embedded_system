#ifndef DRIVER_GPIO_H
#define DRIVER_GPIO_H

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Mode Pin Definitions (CNF[1:0] | MODE[1:0] - 4 bits) */
typedef enum {
    /* Input modes */
    GPIO_MODE_INPUT_ANALOG      = 0x0, // CNF=00, MODE=00
    GPIO_MODE_INPUT_FLOATING    = 0x4, // CNF=01, MODE=00
    GPIO_MODE_INPUT_PULLDOWN    = 0x8, // CNF=10, MODE=00 (ODR=0)
    GPIO_MODE_INPUT_PULLUP      = 0x9, // CNF=10, MODE=00 (ODR=1)

    /* Output modes (Speed 10MHz - MODE=01) */
    GPIO_MODE_OUTPUT_PP_10MHZ   = 0x1, // CNF=00, MODE=01
    GPIO_MODE_OUTPUT_OD_10MHZ   = 0x5, // CNF=01, MODE=01
    GPIO_MODE_AF_PP_10MHZ       = 0x9, // CNF=10, MODE=01
    GPIO_MODE_AF_OD_10MHZ       = 0xD, // CNF=11, MODE=01

    /* Output modes (Speed 2MHz - MODE=10) */
    GPIO_MODE_OUTPUT_PP_2MHZ    = 0x2, // CNF=00, MODE=10
    GPIO_MODE_OUTPUT_OD_2MHZ    = 0x6, // CNF=01, MODE=10
    GPIO_MODE_AF_PP_2MHZ        = 0xA, // CNF=10, MODE=10
    GPIO_MODE_AF_OD_2MHZ        = 0xE, // CNF=11, MODE=10

    /* Output modes (Speed 50MHz - MODE=11) */
    GPIO_MODE_OUTPUT_PP_50MHZ   = 0x3, // CNF=00, MODE=11
    GPIO_MODE_OUTPUT_OD_50MHZ   = 0x7, // CNF=01, MODE=11
    GPIO_MODE_AF_PP_50MHZ       = 0xB, // CNF=10, MODE=11
    GPIO_MODE_AF_OD_50MHZ       = 0xF  // CNF=11, MODE=11
} GPIO_Mode_t;

/* Pin State */
typedef enum {
    DRIVER_GPIO_PIN_RESET = 0,
    DRIVER_GPIO_PIN_SET   = 1
} GPIO_PinState_t;

/* Structure for GPIO Pin Configuration */
typedef struct {
    uint8_t         pin;   // Chân từ 0..15
    GPIO_Mode_t     mode;  // Chế độ hoạt động
} GPIO_Config_t;

/**
  * @brief Enable clock cho GPIO port chỉ định bằng thanh ghi RCC->APB2ENR
  */
void driver_gpio_clock_enable(GPIO_TypeDef *port);

#define DRIVER_GPIO_CLOCK_ENABLE(PORT) \
  do { \
    if ((PORT) == GPIOA) { RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; } \
    else if ((PORT) == GPIOB) { RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; } \
    else if ((PORT) == GPIOC) { RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; } \
    else if ((PORT) == GPIOD) { RCC->APB2ENR |= RCC_APB2ENR_IOPDEN; } \
    else if ((PORT) == GPIOE) { RCC->APB2ENR |= RCC_APB2ENR_IOPEEN; } \
  } while (0)



/**
  * @brief Khởi tạo một chân GPIO sử dụng thanh ghi CRL/CRH
  * @param port Thanh ghi port (GPIOA, GPIOB, ...)
  * @param pin Số chân (0 đến 15)
  * @param mode Chế độ hoạt động (GPIO_Mode_t)
  */
void driver_gpio_init_pin(GPIO_TypeDef *port, uint8_t pin, GPIO_Mode_t mode);

/**
  * @brief Ghi trạng thái High/Low cho chân GPIO bằng thanh ghi BSRR/BRR
  */
void driver_gpio_write_pin(GPIO_TypeDef *port, uint8_t pin, GPIO_PinState_t state);

/**
  * @brief Đọc trạng thái chân GPIO từ thanh ghi IDR
  */
GPIO_PinState_t driver_gpio_read_pin(GPIO_TypeDef *port, uint8_t pin);

/**
  * @brief Đảo trạng thái chân GPIO thông qua thanh ghi ODR
  */
void driver_gpio_toggle_pin(GPIO_TypeDef *port, uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_GPIO_H
