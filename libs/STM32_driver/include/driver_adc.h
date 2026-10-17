#ifndef DRIVER_ADC_H
#define DRIVER_ADC_H

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Undefine HAL macros if HAL ADC header was included before to prevent enum conflicts */
#ifdef ADC_CHANNEL_0
#undef ADC_CHANNEL_0
#undef ADC_CHANNEL_1
#undef ADC_CHANNEL_2
#undef ADC_CHANNEL_3
#undef ADC_CHANNEL_4
#undef ADC_CHANNEL_5
#undef ADC_CHANNEL_6
#undef ADC_CHANNEL_7
#undef ADC_CHANNEL_8
#undef ADC_CHANNEL_9
#undef ADC_CHANNEL_10
#undef ADC_CHANNEL_11
#undef ADC_CHANNEL_12
#undef ADC_CHANNEL_13
#undef ADC_CHANNEL_14
#undef ADC_CHANNEL_15
#endif

/* ========================================================================= */
/* Definitionen für Enums, Strukturen & Makros des ADC-Treibers             */
/* ========================================================================= */

/**
 * @brief ADC-Kanäle (ADC1_IN0 bis ADC1_IN15)
 */
typedef enum {
    DRIVER_ADC_CHANNEL_0  = 0U,
    DRIVER_ADC_CHANNEL_1  = 1U,
    DRIVER_ADC_CHANNEL_2  = 2U,
    DRIVER_ADC_CHANNEL_3  = 3U,
    DRIVER_ADC_CHANNEL_4  = 4U,
    DRIVER_ADC_CHANNEL_5  = 5U,
    DRIVER_ADC_CHANNEL_6  = 6U,
    DRIVER_ADC_CHANNEL_7  = 7U,
    DRIVER_ADC_CHANNEL_8  = 8U,
    DRIVER_ADC_CHANNEL_9  = 9U,
    DRIVER_ADC_CHANNEL_10 = 10U,
    DRIVER_ADC_CHANNEL_11 = 11U,
    DRIVER_ADC_CHANNEL_12 = 12U,
    DRIVER_ADC_CHANNEL_13 = 13U,
    DRIVER_ADC_CHANNEL_14 = 14U,
    DRIVER_ADC_CHANNEL_15 = 15U
} ADC_Channel_t;

/* Backward compatibility aliases */
#define ADC_CHANNEL_0  DRIVER_ADC_CHANNEL_0
#define ADC_CHANNEL_1  DRIVER_ADC_CHANNEL_1
#define ADC_CHANNEL_2  DRIVER_ADC_CHANNEL_2
#define ADC_CHANNEL_3  DRIVER_ADC_CHANNEL_3
#define ADC_CHANNEL_4  DRIVER_ADC_CHANNEL_4
#define ADC_CHANNEL_5  DRIVER_ADC_CHANNEL_5
#define ADC_CHANNEL_6  DRIVER_ADC_CHANNEL_6
#define ADC_CHANNEL_7  DRIVER_ADC_CHANNEL_7
#define ADC_CHANNEL_8  DRIVER_ADC_CHANNEL_8
#define ADC_CHANNEL_9  DRIVER_ADC_CHANNEL_9
#define ADC_CHANNEL_10 DRIVER_ADC_CHANNEL_10
#define ADC_CHANNEL_11 DRIVER_ADC_CHANNEL_11
#define ADC_CHANNEL_12 DRIVER_ADC_CHANNEL_12
#define ADC_CHANNEL_13 DRIVER_ADC_CHANNEL_13
#define ADC_CHANNEL_14 DRIVER_ADC_CHANNEL_14
#define ADC_CHANNEL_15 DRIVER_ADC_CHANNEL_15

/**
 * @brief ADC Prescaler Werte (RCC_CFGR ADCPRE[1:0])
 * Max. ADC-Frequenz beträgt 14 MHz. bei 72 MHz APB2 wird /6 (12 MHz) oder /8 (9 MHz) gewählt.
 */
typedef enum {
    DRIVER_ADC_PRESCALER_DIV2 = 0U, /*!< PCLK2 / 2 */
    DRIVER_ADC_PRESCALER_DIV4 = 1U, /*!< PCLK2 / 4 */
    DRIVER_ADC_PRESCALER_DIV6 = 2U, /*!< PCLK2 / 6 (Empfohlen für 72MHz) */
    DRIVER_ADC_PRESCALER_DIV8 = 3U  /*!< PCLK2 / 8 */
} ADC_Prescaler_t;

#define ADC_PRESCALER_DIV2 DRIVER_ADC_PRESCALER_DIV2
#define ADC_PRESCALER_DIV4 DRIVER_ADC_PRESCALER_DIV4
#define ADC_PRESCALER_DIV6 DRIVER_ADC_PRESCALER_DIV6
#define ADC_PRESCALER_DIV8 DRIVER_ADC_PRESCALER_DIV8

/**
 * @brief Abtastzeit für ADC-Kanäle (Sample Time in Zyklen)
 */
typedef enum {
    DRIVER_ADC_SAMPLETIME_1CYCLE5   = 0U, /*!< 1.5 Zyklen */
    DRIVER_ADC_SAMPLETIME_7CYCLES5  = 1U, /*!< 7.5 Zyklen */
    DRIVER_ADC_SAMPLETIME_13CYCLES5 = 2U, /*!< 13.5 Zyklen */
    DRIVER_ADC_SAMPLETIME_28CYCLES5 = 3U, /*!< 28.5 Zyklen */
    DRIVER_ADC_SAMPLETIME_41CYCLES5 = 4U, /*!< 41.5 Zyklen */
    DRIVER_ADC_SAMPLETIME_55CYCLES5 = 5U, /*!< 55.5 Zyklen */
    DRIVER_ADC_SAMPLETIME_71CYCLES5 = 6U, /*!< 71.5 Zyklen */
    DRIVER_ADC_SAMPLETIME_239CYCLES5= 7U  /*!< 239.5 Zyklen (Höchste Genauigkeit für Biens tre) */
} ADC_SampleTime_t;

#define ADC_SAMPLETIME_1CYCLE5   DRIVER_ADC_SAMPLETIME_1CYCLE5
#define ADC_SAMPLETIME_7CYCLES5  DRIVER_ADC_SAMPLETIME_7CYCLES5
#define ADC_SAMPLETIME_13CYCLES5 DRIVER_ADC_SAMPLETIME_13CYCLES5
#define ADC_SAMPLETIME_28CYCLES5 DRIVER_ADC_SAMPLETIME_28CYCLES5
#define ADC_SAMPLETIME_41CYCLES5 DRIVER_ADC_SAMPLETIME_41CYCLES5
#define ADC_SAMPLETIME_55CYCLES5 DRIVER_ADC_SAMPLETIME_55CYCLES5
#define ADC_SAMPLETIME_71CYCLES5 DRIVER_ADC_SAMPLETIME_71CYCLES5
#define ADC_SAMPLETIME_239CYCLES5 DRIVER_ADC_SAMPLETIME_239CYCLES5

/* ========================================================================= */
/* Funktionsprototypen der ADC-Treiberschnittstelle (Öffentliche API)       */
/* ========================================================================= */

/**
 * @brief Initialisiert das ADC1-Peripheriegerät inkl. Clock und Kalibrierung
 * @param prescaler ADC-Taktteiler (ADC_PRESCALER_DIV6 wird für 72MHz empfohlen)
 */
void driver_adc_init(ADC_Prescaler_t prescaler);

/**
 * @brief Konfiguriert die Abtastzeit eines bestimmten ADC-Kanals
 * @param channel Zu konfigurierender ADC-Kanal (DRIVER_ADC_CHANNEL_0 .. 15)
 * @param sample_time Abtastzeit in Zyklen
 */
void driver_adc_config_channel(ADC_Channel_t channel, ADC_SampleTime_t sample_time);

/**
 * @brief Führt eine Einzelwandlung durch und liefert den 12-Bit-ADC-Wert zurück
 * @param channel Gewünschter ADC-Kanal (DRIVER_ADC_CHANNEL_0 .. 15)
 * @return 12-Bit ADC-Ergebnis (0 bis 4095)
 */
uint16_t driver_adc_read_channel(ADC_Channel_t channel);

/**
 * @brief Liest den ADC-Wert mehrmals und berechnet den Mittelwert zur Rauschunterdrückung
 * @param channel Gewünschter ADC-Kanal (DRIVER_ADC_CHANNEL_0 .. 15)
 * @param samples Anzahl der Abtastungen (z. B. 8, 16, 32)
 * @return Gemittelter 12-Bit ADC-Wert (0 bis 4095)
 */
uint16_t driver_adc_read_avg(ADC_Channel_t channel, uint8_t samples);

// NEU: alter Prototyp uint32_t read_adc_dma(uint32_t *adc_value) ersetzt durch die Version mit DMA-Kanal-Parameter
// NEU: DMA-Kanal wird dynamisch übergeben (nur DMA1_Channel1 ist für ADC1 gültig), Puffer ist volatile
uint32_t read_adc_dma(DMA_Channel_TypeDef *dma_channel, volatile uint32_t *adc_value);

/** Called from DMA1_Channel1_IRQHandler when a new ADC sample differs. */
typedef void (*Driver_ADC_ValueChangedCallback_t)(uint16_t value);
void driver_adc_set_value_changed_callback(Driver_ADC_ValueChangedCallback_t callback);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_ADC_H
