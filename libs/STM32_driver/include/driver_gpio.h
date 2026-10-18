#ifndef DRIVER_GPIO_H
#define DRIVER_GPIO_H

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Pin-Betriebsmodi (CNF[1:0] | MODE[1:0] - 4 Bits) */
typedef enum {
    /* Eingangsmodi */
    GPIO_MODE_INPUT_ANALOG      = 0x0, // CNF=00, MODE=00
    GPIO_MODE_INPUT_FLOATING    = 0x4, // CNF=01, MODE=00
    GPIO_MODE_INPUT_PULLDOWN    = 0x8, // CNF=10, MODE=00 (ODR=0)
    GPIO_MODE_INPUT_PULLUP      = 0x9, // CNF=10, MODE=00 (ODR=1)

    /* Ausgangsmodi (Geschwindigkeit 10MHz - MODE=01) */
    GPIO_MODE_OUTPUT_PP_10MHZ   = 0x1, // CNF=00, MODE=01
    GPIO_MODE_OUTPUT_OD_10MHZ   = 0x5, // CNF=01, MODE=01
    GPIO_MODE_AF_PP_10MHZ       = 0x9, // CNF=10, MODE=01
    GPIO_MODE_AF_OD_10MHZ       = 0xD, // CNF=11, MODE=01

    /* Ausgangsmodi (Geschwindigkeit 2MHz - MODE=10) */
    GPIO_MODE_OUTPUT_PP_2MHZ    = 0x2, // CNF=00, MODE=10
    GPIO_MODE_OUTPUT_OD_2MHZ    = 0x6, // CNF=01, MODE=10
    GPIO_MODE_AF_PP_2MHZ        = 0xA, // CNF=10, MODE=10
    GPIO_MODE_AF_OD_2MHZ        = 0xE, // CNF=11, MODE=10

    /* Ausgangsmodi (Geschwindigkeit 50MHz - MODE=11) */
    GPIO_MODE_OUTPUT_PP_50MHZ   = 0x3, // CNF=00, MODE=11
    GPIO_MODE_OUTPUT_OD_50MHZ   = 0x7, // CNF=01, MODE=11
    GPIO_MODE_AF_PP_50MHZ       = 0xB, // CNF=10, MODE=11
    GPIO_MODE_AF_OD_50MHZ       = 0xF  // CNF=11, MODE=11
} GPIO_Mode_t;

/* Pin-Zustand (HIGH/LOW) */
typedef enum {
    DRIVER_GPIO_PIN_RESET = 0,
    DRIVER_GPIO_PIN_SET   = 1
} GPIO_PinState_t;

/* Struktur zur GPIO-Pin-Konfiguration */
typedef struct {
    uint8_t         pin;   // Pin-Nummer (0..15)
    GPIO_Mode_t     mode;  // Betriebsmodus
} GPIO_Config_t;

/**
  * @brief Peripherietakt für den angegebenen GPIO-Port über das Register RCC->APB2ENR aktivieren
  * @param port Zeiger auf das GPIO-Port-Register (GPIOA, GPIOB, ...)
  */
void driver_gpio_clock_enable(GPIO_TypeDef *port);

/**
  * @brief Initialisiert einen einzelnen GPIO-Pin über die Konfigurationsregister CRL/CRH
  * @param port Zeiger auf das GPIO-Port-Register (GPIOA, GPIOB, ...)
  * @param pin Pin-Nummer (0 bis 15)
  * @param mode Gewünschter Betriebsmodus (GPIO_Mode_t)
  */
void driver_gpio_init_pin(GPIO_TypeDef *port, uint8_t pin, GPIO_Mode_t mode);

/**
  * @brief Setzt den Ausgangszustand (HIGH/LOW) eines Pins über die Register BSRR/BRR
  * @param port Zeiger auf das GPIO-Port-Register (GPIOA, GPIOB, ...)
  * @param pin Pin-Nummer (0 bis 15)
  * @param state Zielzustand (DRIVER_GPIO_PIN_SET oder DRIVER_GPIO_PIN_RESET)
  */
void driver_gpio_write_pin(GPIO_TypeDef *port, uint8_t pin, GPIO_PinState_t state);

/**
  * @brief Liest den aktuellen Eingangszustand eines Pins aus dem Register IDR
  * @param port Zeiger auf das GPIO-Port-Register (GPIOA, GPIOB, ...)
  * @param pin Pin-Nummer (0 bis 15)
  * @return Aktueller Pin-Zustand (DRIVER_GPIO_PIN_SET oder DRIVER_GPIO_PIN_RESET)
  */
GPIO_PinState_t driver_gpio_read_pin(GPIO_TypeDef *port, uint8_t pin);

/**
  * @brief Invertiert den aktuellen Ausgangszustand eines Pins über das Register ODR
  * @param port Zeiger auf das GPIO-Port-Register (GPIOA, GPIOB, ...)
  * @param pin Pin-Nummer (0 bis 15)
  */
void driver_gpio_toggle_pin(GPIO_TypeDef *port, uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_GPIO_H
