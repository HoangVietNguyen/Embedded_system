#include "driver_adc.h"
#include "driver_dma.h"

/**
 * @brief Initialisiert das ADC1-Peripheriegerät inkl. Clock und Kalibrierung
 */
void driver_adc_init(ADC_Prescaler_t prescaler) {
    /* 1. Takt für ADC1 im RCC APB2 Peripheral Clock Enable Register aktivieren */
    // Bit 9 ADC1EN: ADC 1 interface clock enable
    // Page 147/1136 RM0008 Manual pdf
    // Set and cleared by software.
    // 0: ADC 1 interface disabled
    // 1: ADC 1 interface clock enabled
    // Bit 9 (ADC1EN) wird durch eine bitweise ODER-Verknüpfung mit einer Bitmaske auf den Wert 1 gesetzt
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    /* 2. ADC Prescaler im RCC CFGR Register konfigurieren (Bits ADCPRE[1:0]) */
    // Bit 14 und 15 werden auf den Wert 00 gelöscht
    RCC->CFGR &= ~RCC_CFGR_ADCPRE;

    // Page 102/1136 RM0008 Manual pdf
    // 00: PCLK2 divided by 2
    // 01: PCLK2 divided by 4
    // 10: PCLK2 divided by 6
    // 11: PCLK2 divided by 8
    // Der Wert des Prescalers wird auf dem CFGR-Register gespeichert
    RCC->CFGR |= ((uint32_t)prescaler << RCC_CFGR_ADCPRE_Pos);

    /* 3. ADC1 einschalten (ADON-Bit setzen) */
    // Page 243/1136 RM0008 Manual pdf
    // 0: Disable ADC conversion/calibration and go to power down mode.
    // 1: Enable ADC and to start conversion
    // Der im ADC_CR2-Register gespeicherte Wert muss 1 sein, um den ADC einzuschalten
    ADC1->CR2 |= ADC_CR2_ADON;

    // Kurze Stabilisierungsverzögerung nach dem Einschalten des ADC (tSTAB)
    // Stabilisierungsverzögerung zum Warten auf die Systemstabilisierung
    for (volatile uint32_t i = 0; i < 1000; i++) {
        __NOP();
    }

    /* 4. ADC-Kalibrierungsregister zurücksetzen (RSTCAL) */
    // Page 242/1136
    // RSTCAL: Reset calibration
    // This bit is set by software and cleared by hardware. It is cleared after the calibration registers are initialized.
    // 0: Calibration register initialized.
    // 1: Initialize calibration register.
    ADC1->CR2 |= ADC_CR2_RSTCAL;

    while (ADC1->CR2 & ADC_CR2_RSTCAL) {
        /* Warten, bis das Zurücksetzen der Kalibrierung abgeschlossen ist */
    }

    /* 5. ADC-Kalibrierung starten (CAL) */
    // Nachdem Bit 3 RSTCAL auf den Wert 1 gesetzt wurde, muss Bit 2 CAL auf den Wert 1 gesetzt werden.
    // This bit is set by software to start the calibration. It is reset by hardware after calibration is complete.
    // 0: Calibration completed
    // 1: Enable calibration
    ADC1->CR2 |= ADC_CR2_CAL;
    while (ADC1->CR2 & ADC_CR2_CAL) {
        /* Warten, bis die Kalibrierung abgeschlossen ist */
    }
    /**
     * @brief Durchführung der ADC-Selbstkalibrierung mittels RSTCAL und CAL-Bits
     *        zur Korrektur von Offset-Fehlern und Kalibrierung der Messskala.
     */
    // 0V -> 3.3V ------- von 0 bis 4095 (12 Bits)
    // Zurücksetzen der Kalibrierung mittels RSTCAL- und CAL-Bits zur Korrektur der Skalierung

    /* 6. Standard-Konfiguration: Rechtsbündige Datenausrichtung & Einzelwandlungsmodus */
    ADC1->CR2 &= ~(ADC_CR2_ALIGN | ADC_CR2_CONT);
    
    /* 7. External Trigger aktivieren und Triggerquelle auf SWSTART (111 = Software Trigger) setzen */
    ADC1->CR2 |= ADC_CR2_EXTTRIG | (7U << ADC_CR2_EXTSEL_Pos);

    /* 8. Sequenzlänge auf 1 Wandlung setzen (L[3:0] = 0000 in SQR1) */
    ADC1->SQR1 &= ~ADC_SQR1_L;
}

/**
 * @brief Konfiguriert die Abtastzeit eines bestimmten ADC-Kanals
 */
void driver_adc_config_channel(ADC_Channel_t channel, ADC_SampleTime_t sample_time) {
    if (channel <= ADC_CHANNEL_9) {
        /* Kanäle 0 bis 9 befinden sich im Register SMPR2 */
        uint32_t shift = (uint32_t)channel * 3U;
        ADC1->SMPR2 &= ~(0x7U << shift);
        ADC1->SMPR2 |= ((uint32_t)sample_time << shift);
    } else if (channel <= ADC_CHANNEL_15) {
        /* Kanäle 10 bis 15 befinden sich im Register SMPR1 */
        uint32_t shift = ((uint32_t)channel - 10U) * 3U;
        ADC1->SMPR1 &= ~(0x7U << shift);
        ADC1->SMPR1 |= ((uint32_t)sample_time << shift);
    }
}

/**
 * @brief Führt eine Einzelwandlung durch und liefert den 12-Bit-ADC-Wert zurück
 */
uint16_t driver_adc_read_channel(ADC_Channel_t channel) {
    /* 1. Kanal als erste und einzige Wandlung in SQR3 festlegen (SQ1[4:0]) */
    ADC1->SQR3 &= ~ADC_SQR3_SQ1;
    ADC1->SQR3 |= ((uint32_t)channel & 0x1FU);

    /* 2. Clear EOC Flag (End of Conversion) */
    ADC1->SR &= ~ADC_SR_EOC;

    /* 3. Wandlung durch Setzen von SWSTART & ADON starten */
    ADC1->CR2 |= ADC_CR2_SWSTART | ADC_CR2_ADON;

    /* 4. Warten, bis die Wandlung abgeschlossen ist (EOC == 1) mit Timeout-Schutz gegen Deadlock */
    uint32_t timeout = 100000U;
    while (!(ADC1->SR & ADC_SR_EOC) && --timeout) {
        /* Warten auf EOC-Flag */
    }

    /* 5. 12-Bit Konvertierungsergebnis aus dem Datenregister (DR) auslesen */
    return (uint16_t)(ADC1->DR & 0x0FFF);
}

/**
 * @brief Liest den ADC-Wert mehrmals und berechnet den Mittelwert
 */
uint16_t driver_adc_read_avg(ADC_Channel_t channel, uint8_t samples) {
    if (samples == 0) {
        samples = 1;
    }

    uint32_t sum = 0;
    for (uint8_t i = 0; i < samples; i++) {
        sum += driver_adc_read_channel(channel);
    }

    return (uint16_t)(sum / samples);
}

/**
 * @brief Startet kontinuierliche ADC1-Messungen über DMA1-Kanal 1 in den Speicherpuffer
 * @param adc_value Zeiger auf den Speicherbereich (RAM)
 * @return Adresse des Speichers
 */
uint32_t read_adc_dma(uint32_t *adc_value) {
    DMA_Config_t dma_config = {
        .periph_base_addr = (uint32_t)&(ADC1->DR),
        .mem_base_addr = (uint32_t)adc_value,
        .data_length = 1,
        .direction = DRIVER_DMA_DIR_PERIPH_TO_MEM,
        .periph_data_size = DRIVER_DMA_DATA_SIZE_16BIT,
        .mem_data_size = DRIVER_DMA_DATA_SIZE_16BIT,
        .periph_inc_enable = false,
        .mem_inc_enable = false,
        .priority = DRIVER_DMA_PRIORITY_HIGH,
        .mode = DRIVER_DMA_MODE_CIRCULAR
    };

    driver_dma_clock_enable(DMA1);
    driver_dma_init(DMA1_Channel1, &dma_config);
    driver_dma_start(DMA1_Channel1);

    // 240/1136 RM0008 pdf
    // Bit 8 DMA: Direct memory access mode
    // This bit is set and cleared by software. Refer to the DMA controller chapter for more details.
    // 0: DMA mode disabled
    // 1: DMA mode enabled
    // Only ADC1 and ADC3 can generate a DMA request.

    // Bit 8 (DMA) im Register ADC1->CR2 aktivieren
    ADC1->CR2 |= ADC_CR2_DMA;
    // Bit 1 (CONT) im Register ADC1->CR2 für kontinuierlichen Wandlungsmodus aktivieren
    ADC1->CR2 |= ADC_CR2_CONT;
    // Bit 0 ADON: A/D-Wandler einschalten
    ADC1->CR2 |= ADC_CR2_ADON;
    return (uint32_t)adc_value;
}
