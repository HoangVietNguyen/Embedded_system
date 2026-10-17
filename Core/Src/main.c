#include "main.h"

#include <stdio.h>

#include "driver_adc.h"
#include "peripheral/rcc/rcc.h"
#include "driver_gpio.h"
#include "driver_i2c.h"
#include "driver_tim.h"
#include "driver_usart.h"
#include "peripheral/i2c/lcd_config/lcd_config.h"

static volatile uint16_t latest_adc_value;
enum { USART_RX_BUFFER_SIZE = 64U };
static volatile uint8_t usart_rx_buffer[USART_RX_BUFFER_SIZE];
static volatile uint8_t usart_rx_head;
static volatile uint8_t usart_rx_tail;
static uint8_t usart_rx_dma_byte;
static volatile uint8_t usart_rx_restart_failed;

static void usart_dma_rx_callback(USART_TypeDef *USARTx,
                                  const uint8_t *buffer,
                                  uint16_t length)
{
    for (uint16_t i = 0U; i < length; ++i) {
        uint8_t next = (uint8_t)((usart_rx_head + 1U) % USART_RX_BUFFER_SIZE);
        if (next != usart_rx_tail) {
            usart_rx_buffer[usart_rx_head] = buffer[i];
            usart_rx_head = next;
        }
    }

    /* One-byte DMA transfer: arm the next RX byte before leaving the DMA ISR. */
    if (!dma_usart_receive_it(USARTx, &usart_rx_dma_byte, 1U,
                              usart_dma_rx_callback, 1U, 0U)) {
        usart_rx_restart_failed = 1U;
    }
}

//bắt sự kiện thay đổi giá trị ADC
static void adc_value_changed(uint16_t value)
{
    uint16_t previous = latest_adc_value;
    uint16_t difference = (value >= previous) ? (value - previous) : (previous - value);
    if (difference > 50U) {
        latest_adc_value = value;

        uint16_t adc_val = value;
        if (adc_val < 6U) {
            adc_val = 6U;
        } else if (adc_val > 4070U) {
            adc_val = 4070U;
        }

        uint32_t duty_percent = 20U +
            ((uint32_t)(adc_val - 6U) * 70U) / (4070U - 6U);
        driver_tim_pwm_set_compare(TIM4, TIM_CHANNEL_1, (uint16_t)duty_percent);

        char buffer[32];
        lcd_goto_xy(0, 0);
        snprintf(buffer, sizeof(buffer), "ADC value: %4u", (unsigned int)value);
        i2c_send_string(buffer);
    }
}

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

    /* 5. Khởi tạo PWM TIM4 Kanal 1 (PB6) tần số 10kHz (PSC = 71, ARR = 99 -> Chu kỳ T = 100us) */
    driver_tim_pwm_init(TIM4, TIM_CHANNEL_1, 71, 99);

    driver_adc_init(DRIVER_ADC_PRESCALER_DIV6);
    driver_adc_config_channel(ADC_CHANNEL_0, DRIVER_ADC_SAMPLETIME_239CYCLES5);
    static volatile uint32_t adc_value = 0;

    /* USART1 bare-metal RX: 9600 baud, 8N1. */
    driver_usart_init_simple(USART1, 9600U);
    if (!dma_usart_receive_it(USART1, &usart_rx_dma_byte, 1U,
                              usart_dma_rx_callback, 1U, 0U)) {
        Error_Handler();
    }

    /* I2C/LCD must be ready before ADC interrupts can call the callback. */
    init_i2c_modul(I2C1, I2C_SPEED_FAST_400KHz);
    lcd_init();
    lcd_goto_xy(0, 0);

    driver_tim_pwm_set_compare(TIM4, TIM_CHANNEL_1, 20U);

    driver_adc_set_value_changed_callback(adc_value_changed);
    read_adc_dma(DMA1_Channel1, &adc_value);


    while (1)
    {
        if (usart_rx_restart_failed != 0U) {
            Error_Handler();
        }

        if (usart_rx_tail != usart_rx_head) {
            uint8_t received = usart_rx_buffer[usart_rx_tail];
            usart_rx_tail = (uint8_t)((usart_rx_tail + 1U) % USART_RX_BUFFER_SIZE);
            /* Echo the received byte outside the interrupt handler. */
            driver_usart_send_char(USART1, (char)received);
        }

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
