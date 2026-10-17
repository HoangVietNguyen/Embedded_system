#include "driver_tim.h"

#include "driver_dma.h"
#include "driver_interrupt.h"
#include "driver_gpio.h" /* NEU: für driver_gpio_init_pin in driver_tim_pwm_set_compare */

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
 * @brief Konfiguriert das Pin-Remapping für einen bestimmten Timer über das Register AFIO->MAPR
 */
void driver_tim_set_remap(TIM_TypeDef *TIMx, TIM_Remap_t remap) {
    if (TIMx == NULL) {
        return;
    }

    /* AFIO Peripherietakt aktivieren */
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    if (TIMx == TIM1) {
        AFIO->MAPR &= ~AFIO_MAPR_TIM1_REMAP;
        AFIO->MAPR |= (((uint32_t)remap & 0x03U) << AFIO_MAPR_TIM1_REMAP_Pos);
    } else if (TIMx == TIM2) {
        AFIO->MAPR &= ~AFIO_MAPR_TIM2_REMAP;
        AFIO->MAPR |= (((uint32_t)remap & 0x03U) << AFIO_MAPR_TIM2_REMAP_Pos);
    } else if (TIMx == TIM3) {
        AFIO->MAPR &= ~AFIO_MAPR_TIM3_REMAP;
        AFIO->MAPR |= (((uint32_t)remap & 0x03U) << AFIO_MAPR_TIM3_REMAP_Pos);
    } else if (TIMx == TIM4) {
        AFIO->MAPR &= ~AFIO_MAPR_TIM4_REMAP;
        if (remap != DRIVER_TIM_REMAP_NONE) {
            AFIO->MAPR |= AFIO_MAPR_TIM4_REMAP;
        }
    }
}

/* NEU: Vorwärtsdeklaration der GPIO-Zuordnung (Definition weiter unten in dieser Datei) */
static void driver_tim_pwm_gpio_init(TIM_TypeDef *TIMx, TIM_Channel_t channel);

/**
 * @brief Initialisiert einen Timer-Kanal für die PWM-Ausgabe
 */
void driver_tim_pwm_init(TIM_TypeDef *TIMx, TIM_Channel_t channel, uint16_t prescaler, uint16_t auto_reload) {
    /* 1. AFIO Peripherietakt aktivieren */
    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    /* NEU: GPIO-Pin passend zu TIMx / Kanal / aktuellem Remap (AFIO->MAPR) als AF-PP konfigurieren.
     * Ein evtl. gewünschter Remap muss daher VOR driver_tim_pwm_init per driver_tim_set_remap gesetzt werden. */
    driver_tim_pwm_gpio_init(TIMx, channel);

    /* 2. Timer-Timebase initialisieren */
    driver_tim_base_init(TIMx, prescaler, auto_reload);

    /* 3. PWM-Modus 1 (OCxM = 110), Preload-Puffer (OCxPE) und Standard-CCR (1500us) setzen */
    switch (channel) {
        case DRIVER_TIM_CHANNEL_1:
            TIMx->CCR1 = auto_reload / 2U; /* Standard: 1.5ms Impuls (90 Grad) */ // NEU: jetzt 50% Duty-Cycle (auto_reload / 2)
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
 * NEU: Ordnet (TIMx, Kanal) anhand des aktuellen Remap-Zustands in AFIO->MAPR dem GPIO-Pin zu und konfiguriert
 * diesen als Alternate Function Push-Pull (siehe Pin-Tabelle in tim_dockument.md, RM0008 Kapitel 9.3.7 Timer alternate function remapping).
 * Der Pin wird nur umkonfiguriert, wenn er noch nicht im Zustand AF-PP 50MHz (CNF=10, MODE=11) ist,
 * damit beim wiederholten Aufruf von driver_tim_pwm_set_compare kein Glitch auf dem PWM-Ausgang entsteht.
 */
static void driver_tim_pwm_gpio_init(TIM_TypeDef *TIMx, TIM_Channel_t channel) {
    if (channel < DRIVER_TIM_CHANNEL_1 || channel > DRIVER_TIM_CHANNEL_4) {
        return;
    }

    GPIO_TypeDef *port = NULL;
    uint8_t pin = 0U;
    const uint8_t idx = (uint8_t)channel - 1U;
    const uint32_t mapr = AFIO->MAPR;

    if (TIMx == TIM1) {
        /* TIM1_REMAP[1:0] = 11 -> Full Remap (PE9, PE11, PE13, PE14); 00 und 01 -> PA8..PA11 (Partial betrifft nur CHxN / BKIN) */
        static const uint8_t full_pin[4] = {9U, 11U, 13U, 14U};
        if ((mapr & AFIO_MAPR_TIM1_REMAP) == AFIO_MAPR_TIM1_REMAP_FULLREMAP) {
            port = GPIOE;
            pin = full_pin[idx];
        } else {
            port = GPIOA;
            pin = 8U + idx;
        }
    } else if (TIMx == TIM2) {
        /* TIM2_REMAP[1:0]: 00 = PA0..PA3, 01 = PA15,PB3,PA2,PA3, 10 = PA0,PA1,PB10,PB11, 11 = PA15,PB3,PB10,PB11 */
        const uint32_t r = (mapr & AFIO_MAPR_TIM2_REMAP) >> AFIO_MAPR_TIM2_REMAP_Pos;
        const bool remap_low  = (r == 1U || r == 3U);   /* CH1 / CH2 remapped */
        const bool remap_high = (r == 2U || r == 3U);   /* CH3 / CH4 remapped */
        if (idx == 0U) {
            port = GPIOA; /* PA0 bzw. PA15 liegen beide an Port A */
            pin  = remap_low ? 15U : 0U;
        } else if (idx == 1U) {
            port = remap_low ? GPIOB : GPIOA;
            pin  = remap_low ? 3U : 1U;
        } else if (idx == 2U) {
            port = remap_high ? GPIOB : GPIOA;
            pin  = remap_high ? 10U : 2U;
        } else {
            port = remap_high ? GPIOB : GPIOA;
            pin  = remap_high ? 11U : 3U;
        }
    } else if (TIMx == TIM3) {
        /* TIM3_REMAP[1:0]: 00 = PA6,PA7,PB0,PB1, 10 = PB4,PB5,PB0,PB1 (Partial), 11 = PC6..PC9 (Full) */
        const uint32_t r = (mapr & AFIO_MAPR_TIM3_REMAP) >> AFIO_MAPR_TIM3_REMAP_Pos;
        if (r == 3U) {
            port = GPIOC;
            pin = 6U + idx;
        } else if (r == 2U && idx < 2U) {
            port = GPIOB;
            pin = 4U + idx;
        } else if (idx < 2U) {
            port = GPIOA;
            pin = 6U + idx;
        } else {
            port = GPIOB;
            pin = idx - 2U;
        }
    } else if (TIMx == TIM4) {
        /* TIM4_REMAP = 1 -> PD12..PD15, sonst PB6..PB9 */
        if (mapr & AFIO_MAPR_TIM4_REMAP) {
            port = GPIOD;
            pin = 12U + idx;
        } else {
            port = GPIOB;
            pin = 6U + idx;
        }
    } else {
        return;
    }

    /* NEU: Aktuellen Zustand des Pins prüfen (CRL für Pin 0..7, CRH für Pin 8..15) */
    const uint32_t cr = (pin < 8U) ? port->CRL : port->CRH;
    const uint32_t shift = (uint32_t)(pin & 0x7U) * 4U;
    if (((cr >> shift) & 0xFU) == 0xBU) {
        return;
    }

    /* NEU: PA15, PB3 und PB4 sind nach Reset JTAG-Pins -> JTAG deaktivieren, SWD bleibt aktiv (SWJ_CFG = 010) */
    if ((port == GPIOA && pin == 15U) || (port == GPIOB && (pin == 3U || pin == 4U))) {
        RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
        AFIO->MAPR = (AFIO->MAPR & ~AFIO_MAPR_SWJ_CFG) | AFIO_MAPR_SWJ_CFG_JTAGDISABLE;
    }

    driver_gpio_init_pin(port, pin, GPIO_MODE_AF_PP_50MHZ);
}

/**
 * @brief Setzt den Impulsbreitenwert (Pulse / CCR) für einen bestimmten PWM-Kanal
 */
void driver_tim_pwm_set_compare(TIM_TypeDef *TIMx, TIM_Channel_t channel, uint16_t compare_value) {
    if (TIMx == NULL) {
        return;
    }

    /* NEU: Die GPIO-Konfiguration erfolgt jetzt einmalig in driver_tim_pwm_init (nicht bei jedem Aufruf von set_compare) */

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

/**
 * NEU: Ermittelt anhand der DMA-Request-Tabelle (RM0008, Table 78 "Summary of DMA1 requests for each channel")
 * den passenden DMA1-Kanal für die Capture/Compare-DMA-Anforderung (CCxDE) eines Timer-Kanals.
 *
 *   Timer-Kanal | DMA1-Kanal      Timer-Kanal | DMA1-Kanal
 *   TIM1_CH1    | Channel 2       TIM3_CH1    | Channel 6
 *   TIM1_CH2    | Channel 3       TIM3_CH2    | (keine DMA-Anforderung)
 *   TIM1_CH3    | Channel 6       TIM3_CH3    | Channel 2
 *   TIM1_CH4    | Channel 4       TIM3_CH4    | Channel 3
 *   TIM2_CH1    | Channel 5       TIM4_CH1    | Channel 1
 *   TIM2_CH2    | Channel 7       TIM4_CH2    | Channel 4
 *   TIM2_CH3    | Channel 1       TIM4_CH3    | Channel 5
 *   TIM2_CH4    | Channel 7       TIM4_CH4    | (keine DMA-Anforderung)
 *
 * @return true wenn eine Zuordnung existiert, false bei ungültigen Parametern oder fehlender Anforderung
 */
bool driver_tim_get_dma_mapping(TIM_TypeDef *TIMx, TIM_Channel_t channel, Driver_TIM_DMA_Mapping_t *mapping) {
    if (TIMx == NULL || mapping == NULL || channel < DRIVER_TIM_CHANNEL_1 || channel > DRIVER_TIM_CHANNEL_4) {
        return false;
    }

    /* NEU: DMA1-Kanalnummer je Timer und Kanal (0 = keine DMA-Anforderung vorhanden) */
    uint8_t dma_ch_num = 0U;
    switch ((uint32_t)TIMx) {
        case (uint32_t)TIM1: {
            static const uint8_t t[4] = {2U, 3U, 6U, 4U};
            dma_ch_num = t[channel - 1U];
            break;
        }
        case (uint32_t)TIM2: {
            static const uint8_t t[4] = {5U, 7U, 1U, 7U};
            dma_ch_num = t[channel - 1U];
            break;
        }
        case (uint32_t)TIM3: {
            static const uint8_t t[4] = {6U, 0U, 2U, 3U};
            dma_ch_num = t[channel - 1U];
            break;
        }
        case (uint32_t)TIM4: {
            static const uint8_t t[4] = {1U, 4U, 5U, 0U};
            dma_ch_num = t[channel - 1U];
            break;
        }
        default:
            return false;
    }

    if (dma_ch_num == 0U) {
        return false;
    }

    /* NEU: DMA1-Kanal-Zeiger aus Kanalnummer (Index 1..7) */
    DMA_Channel_TypeDef *const dma1_channels[8] = {
        NULL, DMA1_Channel1, DMA1_Channel2, DMA1_Channel3,
        DMA1_Channel4, DMA1_Channel5, DMA1_Channel6, DMA1_Channel7
    };

    /* NEU: Capture/Compare-DMA-Enable-Bit (CCxDE) im Register TIMx->DIER */
    static const uint16_t dier_masks[4] = {
        TIM_DIER_CC1DE, TIM_DIER_CC2DE, TIM_DIER_CC3DE, TIM_DIER_CC4DE
    };

    /* NEU: Zielregister CCRx des Timer-Kanals */
    volatile uint32_t *ccr_regs[4] = { &TIMx->CCR1, &TIMx->CCR2, &TIMx->CCR3, &TIMx->CCR4 };

    mapping->dma_instance  = DMA1;
    mapping->dma_channel   = dma1_channels[dma_ch_num];
    mapping->rcc_ahb_mask  = RCC_AHBENR_DMA1EN;
    mapping->target_ccr    = ccr_regs[channel - 1U];
    mapping->tim_dier_mask = dier_masks[channel - 1U];
    return true;
}
