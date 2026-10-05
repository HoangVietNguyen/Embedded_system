#include "main.h"
#include "peripheral/rcc/rcc.h"
#include "driver_gpio.h"
#include "driver_adc.h"
#include "driver_tim.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* 1. Kích hoạt Clock Port A và Port B bằng Bare-Metal Driver */
    driver_gpio_clock_enable(GPIOA);
    driver_gpio_clock_enable(GPIOB);

    /* 2. Cấu hình chân PA0 làm ngõ vào Analog cho Biến trở (ADC1_IN0) */
    driver_gpio_init_pin(GPIOA, 0, GPIO_MODE_INPUT_ANALOG);

    /* 3. Cấu hình chân PB6 làm Alternate Function Push-Pull cho PWM (TIM4_CH1) */
    driver_gpio_init_pin(GPIOB, 6, GPIO_MODE_AF_PP_50MHZ);

    /* 4. Khởi tạo ADC1 Bare-Metal (Bộ chia 6 = 12MHz, lấy mẫu 239.5 chu kỳ cho PA0 / ADC1_IN0) */
    driver_adc_init(DRIVER_ADC_PRESCALER_DIV6);
    driver_adc_config_channel(ADC_CHANNEL_0, DRIVER_ADC_SAMPLETIME_239CYCLES5);

    /* 5. Khởi tạo PWM TIM4 Kanal 1 (PB6) tần số 10kHz (PSC = 71, ARR = 99 -> Chu kỳ T = 100us) */
    driver_tim_pwm_init(TIM4, TIM_CHANNEL_1, 71, 99);

    while (1)
    {
        /* Đọc giá trị điện áp biến trở ở chân PA0 qua Bare-Metal Driver (Lấy trung bình 16 mẫu) */
        uint16_t adc_val = driver_adc_read_avg(ADC_CHANNEL_0, 16);

        /* Giới hạn dải đầu vào ADC trong khoảng [6, 4070] */
        if (adc_val < 6U) {
            adc_val = 6U;
        } else if (adc_val > 4070U) {
            adc_val = 4070U;
        }

        /* Ánh xạ tuyến tính dải ADC [6 .. 4070] sang dải Duty Cycle [20% .. 90%] */
        /* Với ARR = 99 (100 ticks), Duty Cycle X% tương ứng với Compare CCR1 = X */
        uint32_t duty_percent = 20U + ((uint32_t)(adc_val - 6U) * (90U - 20U)) / (4070U - 6U);

        /* Cập nhật giá trị CCR1 phần cứng để phát xung vuông Duty Cycle từ 20% đến 90% ở PB6 */
        driver_tim_pwm_set_compare(TIM4, TIM_CHANNEL_1, (uint16_t)duty_percent);

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