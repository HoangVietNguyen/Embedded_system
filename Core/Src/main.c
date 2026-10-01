#include "main.h"
#include "peripheral/rcc/rcc.h"
#include "driver_gpio.h"
#include "driver_adc.h"
#include "driver_dma.h"
#include "driver_tim.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* 1. Takt für GPIOA & GPIOB aktivieren */
    driver_gpio_clock_enable(GPIOA);
    driver_gpio_clock_enable(GPIOB);

    /* 2. Pin PA0 als analogen Eingang konfigurieren (Potentiometer) */
    driver_gpio_init_pin(GPIOA, 0, GPIO_MODE_INPUT_ANALOG);

    /* 3. Pin PB6 als Alternate Function Push-Pull für PWM TIM4_CH1 konfigurieren */
    driver_gpio_init_pin(GPIOB, 6, GPIO_MODE_AF_PP_50MHZ);

    /* 4. ADC1 initialisieren (Prescaler /6 = 12MHz, 239.5 Zyklen Abtastzeit an PA0 / ADC1_IN0) */
    driver_adc_init(DRIVER_ADC_PRESCALER_DIV6);
    driver_adc_config_channel(ADC_CHANNEL_0, DRIVER_ADC_SAMPLETIME_239CYCLES5);

    /* 5. PWM TIM4 Kanal 1 (PB6) mit 10kHz initialisieren (PSC = 71, ARR = 99, Periodendauer T = 100us) */
    driver_tim_pwm_init(TIM4, TIM_CHANNEL_1, 71, 99);

    static volatile uint32_t adc_value = 0;
    uint16_t pulse_us = 0;

    /* DMA1 für kontinuierliches Auslesen von ADC1 in die RAM-Variable adc_value aktivieren */
    read_adc_dma((uint32_t*)&adc_value);

    while (1)
    {
        /* ADC-Wert auslesen, der vom DMA automatisch im Hintergrund in adc_value aktualisiert wird */
        uint16_t adc_value_read = (uint16_t)adc_value;

        /* Maximalen Messwert auf 4085 begrenzen */
        if (adc_value_read > 4085) {
            adc_value_read = 4085;
        }

        /* Skalierung: Zuordnung 0..4085 auf den Impulsbereich 1.3ms .. 1.7ms (1300us .. 1700us) */
        pulse_us = 1300U + ((uint32_t)adc_value_read * (1700U - 1300U)) / 4050U;

        /* Impulsbreite direkt in das Register TIM4->CCR1 schreiben */
        driver_tim_pwm_set_compare(TIM4, TIM_CHANNEL_1, pulse_us);

        HAL_Delay(20);
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1)
    {
    }
}