#include "main.h"
#include "peripheral/rcc/rcc.h"
#include "driver_gpio.h"
#include "driver_adc.h"
#include "driver_pwm.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* 1. Khởi tạo Clock GPIOA & GPIOB */
    driver_gpio_clock_enable(GPIOA);
    driver_gpio_clock_enable(GPIOB);

    /* 2. Cấu hình chân PA0 (Đọc biến trở Analog Input) */
    driver_gpio_init_pin(GPIOA, 0, GPIO_MODE_INPUT_ANALOG);

    /* 3. Cấu hình chân PB6 ở chế độ Alternate Function Push-Pull xuất PWM TIM4_CH1 */
    driver_gpio_init_pin(GPIOB, 6, GPIO_MODE_AF_PP_50MHZ);

    /* 4. Khởi tạo ADC1 (Prescaler /6 = 12MHz, 239.5 cycles lấy mẫu tại PA0 / ADC1_IN0) */
    driver_adc_init(DRIVER_ADC_PRESCALER_DIV6);
    driver_adc_config_channel(ADC_CHANNEL_0, DRIVER_ADC_SAMPLETIME_239CYCLES5);

    /* 5. Khởi tạo PWM TIM4 Kênh 1 (PB6) ở tần số 50 Hz (PSC = 71, ARR = 19999, Chu kỳ T = 20ms) */
    driver_pwm_init(TIM4, PWM_CHANNEL_1, 71, 19999);

    uint16_t adc_value = 0;
    uint16_t pulse_us = 0;

    while (1)
    {
        /* Đọc giá trị biến trở lọc nhiễu 16 lần */
        adc_value = driver_adc_read_avg(ADC_CHANNEL_0, 16);

        /* Khống chế dải đo tối đa 4085 */
        if (adc_value > 4085) {
            adc_value = 4085;
        }

        /* KHÔI PHỤC DẢI ỔN ĐỊNH CHUẨN: Map 0..4085 vào dải 1.3ms .. 1.7ms (1300us .. 1700us) */
        pulse_us = 1300U + ((uint32_t)adc_value * (1700U - 1300U)) / 4085U;

        /* Xuất độ rộng xung trực tiếp vào thanh ghi CCR1 của TIM4 */
        driver_pwm_set_duty_cycle(TIM4, PWM_CHANNEL_1, pulse_us);

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