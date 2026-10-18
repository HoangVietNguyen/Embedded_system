#include "driver_exti.h"
#include "driver_gpio.h"
#include "driver_interrupt.h"

/* ========================================================================= */
/* Implementierung der bare-metal EXTI-Treiberfunktionen (Skeleton)          */
/* ========================================================================= */

volatile uint8_t register_number;
volatile uint8_t shift;

#define GET_EXTI_REGISTER_NUMBER(PIN) \
do { \
    shift = ((PIN) % 4U) * 4U; \
    if((PIN) <=3) { AFIO->EXTICR[0] &= ~(0xFU << shift); register_number = 0U; } \
    else if((PIN) <=7) { AFIO->EXTICR[1] &= ~(0xFU << shift); register_number = 1U; } \
    else if((PIN) <=11) { AFIO->EXTICR[2] &= ~(0xFU << shift); register_number = 2U; } \
    else { AFIO->EXTICR[3] &= ~(0xFU << shift); register_number = 3U;}\
} while (0)

// AFIO->EXTICR[]-Register:
// ┌──────────────────┬──────────────────┬──────────────────┬──────────────────┐
// │ Bits [15:12]     │ Bits [11:8]      │ Bits [7:4]       │ Bits [3:0]       │
// │ Für Pin 3        │ Für Pin 2        │ Für Pin 1        │ Für Pin 0        │
// └──────────────────┴──────────────────┴──────────────────┴──────────────────┘

#define PORT_SHIFT_BIT(PORT, PIN) \
do { \
    if((PORT) == GPIOA) { AFIO->EXTICR[register_number] |= (0x0U << shift);} \
    else if((PORT) == GPIOB) { AFIO->EXTICR[register_number] |= (0x1U << shift); } \
    else if((PORT) == GPIOC) { AFIO->EXTICR[register_number] |= (0x2U << shift); } \
    else { AFIO->EXTICR[register_number] |= (0x3U << shift); }\
} while (0)

#define EVENT_PIN_CONFIG(TRIGGER, PIN) \
do { \
    EXTI->RTSR &= ~(1U << (PIN)); \
    EXTI->FTSR &= ~(1U << (PIN)); \
    if ((TRIGGER) == DRIVER_EXTI_TRIGGER_RISING) { \
        EXTI->RTSR |= (1U << (PIN)); \
    } else if ((TRIGGER) == DRIVER_EXTI_TRIGGER_FALLING) { \
        EXTI->FTSR |= (1U << (PIN)); \
    } else if ((TRIGGER) == DRIVER_EXTI_TRIGGER_RISING_FALLING) { \
        EXTI->RTSR |= (1U << (PIN)); \
        EXTI->FTSR |= (1U << (PIN)); \
    } \
} while (0)

void driver_exti_init(GPIO_TypeDef *port, uint8_t pin, EXTI_Trigger_t trigger, uint32_t preempt_priority, uint32_t sub_priority) {
    /* Schritt 1 bis Schritt 6 gemäß RM0008 */

    if (port == NULL || pin > 15) {
        return;
    }

    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    driver_gpio_clock_enable(port);
    driver_gpio_init_pin(port, pin, GPIO_MODE_INPUT_PULLUP);

    GET_EXTI_REGISTER_NUMBER(pin);
    PORT_SHIFT_BIT(port, pin);

    // Schritt 4: Konfiguration des Auslösemodus (Flanke: Rising / Falling / Both)
    EVENT_PIN_CONFIG(trigger, pin);

    // Schritt 5: Deaktivieren der Maskierung im IMR-Register (Interrupt Request freischalten)
    EXTI->IMR |= (1U << pin);

    // Schritt 6: Bestimmung des NVIC-Interrupt-Kanals (IRQn_Type) basierend auf der Pin-Nummer und Konfiguration des NVIC
    IRQn_Type irq_n;
    if (pin == 0U) {
        irq_n = EXTI0_IRQn;
    } else if (pin == 1U) {
        irq_n = EXTI1_IRQn;
    } else if (pin == 2U) {
        irq_n = EXTI2_IRQn;
    } else if (pin == 3U) {
        irq_n = EXTI3_IRQn;
    } else if (pin == 4U) {
        irq_n = EXTI4_IRQn;
    } else if (pin <= 9U) {
        irq_n = EXTI9_5_IRQn; /* Pins 5..9 teilen sich EXTI9_5_IRQn */
    } else {
        irq_n = EXTI15_10_IRQn; /* Pins 10..15 teilen sich EXTI15_10_IRQn */
    }

    driver_interrupt_nvic_config(irq_n, preempt_priority, sub_priority, true);
}

void driver_exti_enable(uint8_t pin) {
    if (pin <= 15U) {
        EXTI->IMR |= (1U << pin);
    }
}

void driver_exti_disable(uint8_t pin) {
    if (pin <= 15U) {
        EXTI->IMR &= ~(1U << pin);
    }
}

bool driver_exti_is_pending(uint8_t pin) {
    if (pin > 15U) {
        return false;
    }
    return (EXTI->PR & (1U << pin)) != 0U;
}

void driver_exti_clear_pending(uint8_t pin) {
    if (pin <= 15U) {
        EXTI->PR = (1U << pin);
    }
}