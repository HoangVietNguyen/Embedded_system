#ifndef DRIVER_TIM_H
#define DRIVER_TIM_H

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Definitionen & Strukturen für den TIM (Timer) Treiber                      */
/* ========================================================================= */

/**
 * @brief Timer-Kanäle (Channel 1 bis Channel 4)
 */
typedef enum {
    DRIVER_TIM_CHANNEL_1 = 1U,
    DRIVER_TIM_CHANNEL_2 = 2U,
    DRIVER_TIM_CHANNEL_3 = 3U,
    DRIVER_TIM_CHANNEL_4 = 4U
} TIM_Channel_t;

/* Backward compatibility aliases */
#ifndef TIM_CHANNEL_1
#define TIM_CHANNEL_1 DRIVER_TIM_CHANNEL_1
#endif
#ifndef TIM_CHANNEL_2
#define TIM_CHANNEL_2 DRIVER_TIM_CHANNEL_2
#endif
#ifndef TIM_CHANNEL_3
#define TIM_CHANNEL_3 DRIVER_TIM_CHANNEL_3
#endif
#ifndef TIM_CHANNEL_4
#define TIM_CHANNEL_4 DRIVER_TIM_CHANNEL_4
#endif

/* PWM Channel Aliases */
typedef TIM_Channel_t PWM_Channel_t;
#define DRIVER_PWM_CHANNEL_1 DRIVER_TIM_CHANNEL_1
#define DRIVER_PWM_CHANNEL_2 DRIVER_TIM_CHANNEL_2
#define DRIVER_PWM_CHANNEL_3 DRIVER_TIM_CHANNEL_3
#define DRIVER_PWM_CHANNEL_4 DRIVER_TIM_CHANNEL_4
#define PWM_CHANNEL_1 DRIVER_TIM_CHANNEL_1
#define PWM_CHANNEL_2 DRIVER_TIM_CHANNEL_2
#define PWM_CHANNEL_3 DRIVER_TIM_CHANNEL_3
#define PWM_CHANNEL_4 DRIVER_TIM_CHANNEL_4

/* ========================================================================= */
/* Funktionsprototypen der TIM-Treiberschnittstelle (Öffentliche API)         */
/* ========================================================================= */

/**
 * @brief Aktiviert den Peripherietakt für einen Timer (TIM1, TIM2, TIM3, TIM4)
 * @param TIMx Zeiger auf das Timer-Register
 */
void driver_tim_clock_enable(TIM_TypeDef *TIMx);

/**
 * @brief Initialisiert die grundlegende Timebase (PSC & ARR) eines Timers
 * @param TIMx Zeiger auf das Timer-Register (TIM1, TIM2, TIM3, TIM4)
 * @param prescaler Frequenzteiler-Wert (PSC)
 * @param auto_reload Zähler-Nachladewert (ARR)
 */
void driver_tim_base_init(TIM_TypeDef *TIMx, uint16_t prescaler, uint16_t auto_reload);

/**
 * @brief Startet den Zähler des Timers (CEN-Bit setzen)
 * @param TIMx Zeiger auf das Timer-Register
 */
void driver_tim_start(TIM_TypeDef *TIMx);

/**
 * @brief Stoppt den Zähler des Timers (CEN-Bit löschen)
 * @param TIMx Zeiger auf das Timer-Register
 */
void driver_tim_stop(TIM_TypeDef *TIMx);

/**
 * @brief Erzeugt eine Verzögerung in Mikrosekunden (us) mit einem Timer
 * @param TIMx Zeiger auf das Timer-Register
 * @param us Verzögerungszeit in Mikrosekunden
 */
void driver_tim_delay_us(TIM_TypeDef *TIMx, uint32_t us);

/**
 * @brief Erzeugt eine Verzögerung in Millisekunden (ms) mit einem Timer
 * @param TIMx Zeiger auf das Timer-Register
 * @param ms Verzögerungszeit in Millisekunden
 */
void driver_tim_delay_ms(TIM_TypeDef *TIMx, uint32_t ms);

/**
 * @brief Aktiviert den Update-Interrupt (UIE) eines Timers und konfiguriert den NVIC
 * @param TIMx Zeiger auf das Timer-Register (TIM1, TIM2, TIM3, TIM4)
 * @param preempt_priority Preemption-Priorität im NVIC
 * @param sub_priority Sub-Priorität im NVIC
 */
void driver_tim_interrupt_enable(TIM_TypeDef *TIMx, uint32_t preempt_priority, uint32_t sub_priority);

/**
 * @brief Deaktiviert den Update-Interrupt (UIE) eines Timers
 * @param TIMx Zeiger auf das Timer-Register
 */
void driver_tim_interrupt_disable(TIM_TypeDef *TIMx);

/**
 * @brief Initialisiert einen Timer-Kanal für die PWM-Ausgabe
 * @param TIMx Zeiger auf das Timer-Register (TIM1, TIM2, TIM3, TIM4)
 * @param channel Gewünschter PWM-Kanal (TIM_CHANNEL_1 .. 4)
 * @param prescaler Frequenzteiler-Wert (PSC)
 * @param auto_reload Zähler-Nachladewert (ARR)
 */
void driver_tim_pwm_init(TIM_TypeDef *TIMx, TIM_Channel_t channel, uint16_t prescaler, uint16_t auto_reload);

/**
 * @brief Setzt den Impulsbreitenwert (Pulse / CCR) für einen bestimmten PWM-Kanal
 * @param TIMx Zeiger auf das Timer-Register (TIM1, TIM2, TIM3, TIM4)
 * @param channel Gewünschter PWM-Kanal (TIM_CHANNEL_1 .. 4)
 * @param compare_value Impulsbreitenwert (CCR-Register)
 */
void driver_tim_pwm_set_compare(TIM_TypeDef *TIMx, TIM_Channel_t channel, uint16_t compare_value);

/**
 * @brief Setzt das Tastverhältnis (Duty Cycle) in Prozent (0% bis 100%) für einen PWM-Kanal
 * @param TIMx Zeiger auf das Timer-Register (TIM1, TIM2, TIM3, TIM4)
 * @param channel Gewünschter PWM-Kanal (TIM_CHANNEL_1 .. 4)
 * @param percent Tastverhältnis in Prozent (0.0f bis 100.0f)
 */
void driver_tim_pwm_set_duty_percent(TIM_TypeDef *TIMx, TIM_Channel_t channel, float percent);

/**
 * @brief Setzt das Tastverhältnis (Duty Cycle) als Ganzzahl (0% bis 100%)
 * @param TIMx Zeiger auf das Timer-Register (TIM1, TIM2, TIM3, TIM4)
 * @param channel Gewünschter PWM-Kanal (TIM_CHANNEL_1 .. 4)
 * @param percent Tastverhältnis als Ganzzahl (0 bis 100)
 */
void driver_tim_pwm_set_duty_percent_u8(TIM_TypeDef *TIMx, TIM_Channel_t channel, uint8_t percent);

/**
 * @brief Berechnet und setzt den Servo-Stellwinkel (0 bis 180 Grad) für SG90 (50Hz PWM)
 * @param TIMx Zeiger auf das Timer-Register (TIM1, TIM2, TIM3, TIM4)
 * @param channel Gewünschter PWM-Kanal (TIM_CHANNEL_1 .. 4)
 * @param angle Stellwinkel in Grad (0 bis 180)
 */
void driver_tim_pwm_set_servo_angle(TIM_TypeDef *TIMx, TIM_Channel_t channel, uint8_t angle);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_TIM_H
