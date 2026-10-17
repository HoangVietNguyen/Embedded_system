#ifndef DRIVER_EXTI_H
#define DRIVER_EXTI_H

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Definitionen & Strukturen des EXTI-Treibers (External Interrupt)         */
/* ========================================================================= */

/**
 * @brief EXTI Trigger-Flankenmodus (Rising, Falling, Both)
 */
typedef enum {
    DRIVER_EXTI_TRIGGER_RISING         = 0U, /*!< Auslösung bei steigender Flanke */
    DRIVER_EXTI_TRIGGER_FALLING        = 1U, /*!< Auslösung bei fallender Flanke */
    DRIVER_EXTI_TRIGGER_RISING_FALLING = 2U  /*!< Auslösung bei beiden Flanken */
} EXTI_Trigger_t;

/* Backward compatibility aliases */
#define EXTI_TRIGGER_RISING         DRIVER_EXTI_TRIGGER_RISING
#define EXTI_TRIGGER_FALLING        DRIVER_EXTI_TRIGGER_FALLING
#define EXTI_TRIGGER_RISING_FALLING DRIVER_EXTI_TRIGGER_RISING_FALLING

/* ========================================================================= */
/* Funktionsprototypen der EXTI-Treiberschnittstelle                        */
/* ========================================================================= */

/**
 * @brief Konfiguriert einen GPIO-Pin für externen Interrupt (EXTI) inkl. AFIO & NVIC
 * @param port GPIO-Port (GPIOA, GPIOB, GPIOC, GPIOD)
 * @param pin Pin-Nummer (0 bis 15)
 * @param trigger Auslösemodus (Rising, Falling, Both)
 * @param preempt_priority Preemption-Priorität im NVIC
 * @param sub_priority Sub-Priorität im NVIC
 */
void driver_exti_init(GPIO_TypeDef *port, uint8_t pin, EXTI_Trigger_t trigger, uint32_t preempt_priority, uint32_t sub_priority);

/**
 * @brief Aktiviert den EXTI-Interrupt für den gewählten Pin (IMR-Bit setzen)
 * @param pin Pin-Nummer (0 bis 15)
 */
void driver_exti_enable(uint8_t pin);

/**
 * @brief Deaktiviert den EXTI-Interrupt für den gewählten Pin (IMR-Bit löschen)
 * @param pin Pin-Nummer (0 bis 15)
 */
void driver_exti_disable(uint8_t pin);

/**
 * @brief Prüft, ob das EXTI-Pending-Flag für den gewählten Pin gesetzt ist (PR-Register)
 * @param pin Pin-Nummer (0 bis 15)
 * @return true wenn Interrupt ansteht, sonst false
 */
bool driver_exti_is_pending(uint8_t pin);

/**
 * @brief Löscht das EXTI-Pending-Flag für den gewählten Pin (PR-Register: Write 1 to clear)
 * @param pin Pin-Nummer (0 bis 15)
 */
void driver_exti_clear_pending(uint8_t pin);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_EXTI_H