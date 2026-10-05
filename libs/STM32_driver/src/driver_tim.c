#include "driver_tim.h"
#include "driver_interrupt.h"

/**
 * @brief Aktiviert den Peripherietakt für einen Timer (TIM1, TIM2, TIM3, TIM4)
 */
void driver_tim_clock_enable(TIM_TypeDef *TIMx) {
    if (TIMx == TIM1) {
        RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;
    } else if (TIMx == TIM2) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    } else if (TIMx == TIM3) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
    } else if (TIMx == TIM4) {
        RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;
    }
}

/**
 * @brief Initialisiert die grundlegende Timebase (PSC & ARR) eines Timers
 */
void driver_tim_base_init(TIM_TypeDef *TIMx, uint16_t prescaler, uint16_t auto_reload) {
    if (TIMx == NULL) {
        return;
    }

    /* 1. Peripherietakt aktivieren */
    driver_tim_clock_enable(TIMx);

    /* 2. Prescaler (PSC) und Auto-Reload Register (ARR) konfigurieren */
    TIMx->PSC = prescaler;
    TIMx->ARR = auto_reload;

    /* 3. Zähler zurücksetzen */
    TIMx->CNT = 0U;

    /* 4. Event-Generierung erzwingen (UG-Bit), um Prescaler & ARR sofort zu laden */
    TIMx->EGR |= TIM_EGR_UG;

    /* 5. Auto-Reload Preload Enable (ARPE) im Control Register 1 aktivieren */
    TIMx->CR1 |= TIM_CR1_ARPE;
}

/**
 * @brief Startet den Zähler des Timers (CEN-Bit setzen)
 */
void driver_tim_start(TIM_TypeDef *TIMx) {
    if (TIMx != NULL) {
        TIMx->CR1 |= TIM_CR1_CEN;
    }
}

/**
 * @brief Stoppt den Zähler des Timers (CEN-Bit löschen)
 */
void driver_tim_stop(TIM_TypeDef *TIMx) {
    if (TIMx != NULL) {
        TIMx->CR1 &= ~TIM_CR1_CEN;
    }
}

/**
 * @brief Erzeugt eine Verzögerung in Mikrosekunden (us) mit einem Timer
 */
void driver_tim_delay_us(TIM_TypeDef *TIMx, uint32_t us) {
    if (TIMx == NULL || us == 0U) {
        return;
    }

    /* 1. Timer initialisieren: 1MHz Taktfrequenz (72MHz / 72 = 1MHz -> 1 Takt = 1us) */
    driver_tim_base_init(TIMx, 71U, 0xFFFFU);
    driver_tim_start(TIMx);

    /* 2. Zähler auf 0 zurücksetzen */
    TIMx->CNT = 0U;

    /* 3. Warten bis der Zählerwert 'us' erreicht */
    while (TIMx->CNT < us);

    /* 4. Timer stoppen */
    driver_tim_stop(TIMx);
}

/**
 * @brief Erzeugt eine Verzögerung in Millisekunden (ms) mit einem Timer
 */
void driver_tim_delay_ms(TIM_TypeDef *TIMx, uint32_t ms) {
    if (TIMx == NULL || ms == 0U) {
        return;
    }

    while (ms--) {
        driver_tim_delay_us(TIMx, 1000U);
    }
}

/**
 * @brief Aktiviert den Update-Interrupt (UIE) eines Timers und konfiguriert den NVIC
 */
void driver_tim_interrupt_enable(TIM_TypeDef *TIMx, uint32_t preempt_priority, uint32_t sub_priority) {
    if (TIMx == NULL) {
        return;
    }

    /* Update-Pending Flag löschen */
    TIMx->SR &= ~TIM_SR_UIF;

    /* Update Interrupt Enable (UIE) setzen */
    TIMx->DIER |= TIM_DIER_UIE;

    /* NVIC-Kanal bestimmen und konfigurieren */
    IRQn_Type irq_n;
    if (TIMx == TIM1) {
        irq_n = TIM1_UP_IRQn;
    } else if (TIMx == TIM2) {
        irq_n = TIM2_IRQn;
    } else if (TIMx == TIM3) {
        irq_n = TIM3_IRQn;
    } else if (TIMx == TIM4) {
        irq_n = TIM4_IRQn;
    } else {
        return;
    }

    driver_interrupt_nvic_config(irq_n, preempt_priority, sub_priority, true);
}

/**
 * @brief Deaktiviert den Update-Interrupt (UIE) eines Timers
 */
void driver_tim_interrupt_disable(TIM_TypeDef *TIMx) {
    if (TIMx != NULL) {
        TIMx->DIER &= ~TIM_DIER_UIE;
    }
}

/**
 * @brief Initialisiert einen Timer-Kanal für die PWM-Ausgabe
 */
void driver_tim_pwm_init(TIM_TypeDef *TIMx, TIM_Channel_t channel, uint16_t prescaler, uint16_t auto_reload) {
    /* 1. AFIO Peripherietakt aktivieren */
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    /* 2. Timer-Timebase initialisieren */
    driver_tim_base_init(TIMx, prescaler, auto_reload);

    /* 3. PWM-Modus 1 (OCxM = 110), Preload-Puffer (OCxPE) und Standard-CCR (1500us) setzen */
    switch (channel) {
        case DRIVER_TIM_CHANNEL_1:
            TIMx->CCR1 = auto_reload / 2U;
            TIMx->CCMR1 &= ~(TIM_CCMR1_OC1M | TIM_CCMR1_CC1S);
            TIMx->CCMR1 |= (6U << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE;
            TIMx->CCER |= TIM_CCER_CC1E;
            break;

        case DRIVER_TIM_CHANNEL_2:
            TIMx->CCR2 = auto_reload / 2U;
            TIMx->CCMR1 &= ~(TIM_CCMR1_OC2M | TIM_CCMR1_CC2S);
            TIMx->CCMR1 |= (6U << TIM_CCMR1_OC2M_Pos) | TIM_CCMR1_OC2PE;
            TIMx->CCER |= TIM_CCER_CC2E;
            break;

        case DRIVER_TIM_CHANNEL_3:
            TIMx->CCR3 = auto_reload / 2U;
            TIMx->CCMR2 &= ~(TIM_CCMR2_OC3M | TIM_CCMR2_CC3S);
            TIMx->CCMR2 |= (6U << TIM_CCMR2_OC3M_Pos) | TIM_CCMR2_OC3PE;
            TIMx->CCER |= TIM_CCER_CC3E;
            break;

        case DRIVER_TIM_CHANNEL_4:
            TIMx->CCR4 = auto_reload / 2U;
            TIMx->CCMR2 &= ~(TIM_CCMR2_OC4M | TIM_CCMR2_CC4S);
            TIMx->CCMR2 |= (6U << TIM_CCMR2_OC4M_Pos) | TIM_CCMR2_OC4PE;
            TIMx->CCER |= TIM_CCER_CC4E;
            break;
    }

    /* 4. Event-Generierung erzwingen (UG-Bit) */
    TIMx->EGR |= TIM_EGR_UG;

    /* 5. Für TIM1 (Erweiterter Timer): Hauptausgang aktivieren (MOE-Bit im BDTR) */
    if (TIMx == TIM1) {
        TIMx->BDTR |= TIM_BDTR_MOE;
    }

    /* 6. Timer-Zähler starten */
    driver_tim_start(TIMx);
}

/**
 * @brief Setzt den Impulsbreitenwert (Pulse / CCR) für einen bestimmten PWM-Kanal
 */
void driver_tim_pwm_set_compare(TIM_TypeDef *TIMx, TIM_Channel_t channel, uint16_t compare_value) {
    if (TIMx == NULL) {
        return;
    }

    switch (channel) {
        case DRIVER_TIM_CHANNEL_1:
            TIMx->CCR1 = compare_value;
            break;
        case DRIVER_TIM_CHANNEL_2:
            TIMx->CCR2 = compare_value;
            break;
        case DRIVER_TIM_CHANNEL_3:
            TIMx->CCR3 = compare_value;
            break;
        case DRIVER_TIM_CHANNEL_4:
            TIMx->CCR4 = compare_value;
            break;
    }
}

/**
 * @brief Setzt das Tastverhältnis (Duty Cycle) in Prozent (0.0% bis 100.0%)
 */
void driver_tim_pwm_set_duty_percent(TIM_TypeDef *TIMx, TIM_Channel_t channel, float percent) {
    if (percent < 0.0f) {
        percent = 0.0f;
    } else if (percent > 100.0f) {
        percent = 100.0f;
    }

    uint32_t arr_val = TIMx->ARR;
    uint32_t compare_val = (uint32_t)((percent * (float)(arr_val + 1U)) / 100.0f);
    driver_tim_pwm_set_compare(TIMx, channel, (uint16_t)compare_val);
}

/**
 * @brief Setzt das Tastverhältnis (Duty Cycle) als Ganzzahl (0% bis 100%)
 */
void driver_tim_pwm_set_duty_percent_u8(TIM_TypeDef *TIMx, TIM_Channel_t channel, uint8_t percent) {
    if (percent > 100U) {
        percent = 100U;
    }

    uint32_t arr_val = TIMx->ARR;
    uint32_t compare_val = ((uint32_t)percent * (arr_val + 1U)) / 100U;
    driver_tim_pwm_set_compare(TIMx, channel, (uint16_t)compare_val);
}

/**
 * @brief Berechnet und setzt den Servo-Stellwinkel (0 bis 180 Grad) für SG90 (50Hz PWM: 0.5ms bis 2.5ms = 500us bis 2500us)
 */
void driver_tim_pwm_set_servo_angle(TIM_TypeDef *TIMx, TIM_Channel_t channel, uint8_t angle) {
    if (angle > 180U) {
        angle = 180U;
    }

    /* SG90 Servo Impulsbreite: 0 Grad -> 500us (0.5ms), 180 Grad -> 2500us (2.5ms) bei PSC=71 (1MHz) */
    uint16_t ccr_value = 500U + ((uint32_t)angle * 2000U) / 180U;
    driver_tim_pwm_set_compare(TIMx, channel, ccr_value);
}
