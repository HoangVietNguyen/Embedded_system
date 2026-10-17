#include "driver_gpio.h"

/* Makro zur internen Taktaktivierung des entsprechenden GPIO-Ports */
#define DRIVER_GPIO_CLOCK_ENABLE(PORT) \
do { \
if ((PORT) == GPIOA) { RCC->APB2ENR |= RCC_APB2ENR_IOPAEN; } \
else if ((PORT) == GPIOB) { RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; } \
else if ((PORT) == GPIOC) { RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; } \
else if ((PORT) == GPIOD) { RCC->APB2ENR |= RCC_APB2ENR_IOPDEN; } \
else if ((PORT) == GPIOE) { RCC->APB2ENR |= RCC_APB2ENR_IOPEEN; } \
} while (0)

void driver_gpio_clock_enable(GPIO_TypeDef *port) {
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
    DRIVER_GPIO_CLOCK_ENABLE(port);
}

void driver_gpio_init_pin(GPIO_TypeDef *port, uint8_t pin, GPIO_Mode_t mode)
{
    if (port == NULL || pin > 15) {
        return;
    }

    /* Peripherietakt für den Zielport aktivieren */
    driver_gpio_clock_enable(port);

    /* Maskierung der 4-Bit-Konfiguration (CNF[1:0] | MODE[1:0]) */
    uint8_t mode_val = (uint8_t)(mode & 0x0F);

    /* Sonderbehandlung für Pull-Up und Pull-Down Konfiguration:
     * Beide Modi verwenden den Wert 0x8 (CNF=10, MODE=00) im CRL/CRH Register.
     * Das ODR-Register steuert den internen Widerstand (1 = Pull-Up, 0 = Pull-Down).
     */
    if (mode == GPIO_MODE_INPUT_PULLUP) {
        mode_val = 0x8; // CNF = 10, MODE = 00
        port->ODR |= (1U << pin);  // ODR = 1 aktiviert Pull-Up
    } else if (mode == GPIO_MODE_INPUT_PULLDOWN) {
        mode_val = 0x8; // CNF = 10, MODE = 00
        port->ODR &= ~(1U << pin); // ODR = 0 aktiviert Pull-Down
    }

    /* Pins 0 bis 7 werden über das CRL-Register gesteuert (4 Bits pro Pin) */
    if (pin < 8) {
        uint8_t shift = pin * 4;
        port->CRL &= ~(0x0FUL << shift); // Vorherige 4 Bits löschen
        port->CRL |= ((uint32_t)mode_val << shift); // Neue 4 Bits einschreiben
    } else {
        /* Pins 8 bis 15 werden über das CRH-Register gesteuert (4 Bits pro Pin) */
        uint8_t shift = (pin - 8) * 4;
        port->CRH &= ~(0x0FUL << shift); // Vorherige 4 Bits löschen
        port->CRH |= ((uint32_t)mode_val << shift); // Neue 4 Bits einschreiben
    }
}

void driver_gpio_write_pin(GPIO_TypeDef *port, uint8_t pin, GPIO_PinState_t state)
{
    if (port == NULL || pin > 15) {
        return;
    }

    if (state == DRIVER_GPIO_PIN_SET) {
        /* Atomares Setzen auf HIGH über das BSRR-Register */
        port->BSRR = (1U << pin);
    } else {
        /* Atomares Rücksetzen auf LOW über das BRR-Register */
        port->BRR = (1U << pin);
    }
}

GPIO_PinState_t driver_gpio_read_pin(GPIO_TypeDef *port, uint8_t pin)
{
    if (port == NULL || pin > 15) {
        return DRIVER_GPIO_PIN_RESET;
    }

    /* Auslesen des Eingangszustands über das IDR-Register */
    return (port->IDR & (1U << pin)) ? DRIVER_GPIO_PIN_SET : DRIVER_GPIO_PIN_RESET;
}

void driver_gpio_toggle_pin(GPIO_TypeDef *port, uint8_t pin)
{
    if (port == NULL || pin > 15) {
        return;
    }

    /* Invertieren des Ausgangszustands über das ODR-Register */
    port->ODR ^= (1U << pin);
}
