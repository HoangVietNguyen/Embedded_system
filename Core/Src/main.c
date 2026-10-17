#include "main.h"

#include "driver_exti.h"
#include "driver_gpio.h"
#include "driver_spi.h"
#include "driver_usart.h"
#include "peripheral/rcc/rcc.h"

static const uint8_t spi_message[] = "SPI1 test message\r\n";
static const char usart_message[] = "USART1 DMA TX ready\r\n";
static volatile uint8_t usart_button_pressed;
static volatile uint8_t spi_button_pressed;

void EXTI0_IRQHandler(void)
{
    if (driver_exti_is_pending(0U)) {
        driver_exti_clear_pending(0U);
        usart_button_pressed = 1U;
    }
}

void EXTI1_IRQHandler(void)
{
    if (driver_exti_is_pending(1U)) {
        driver_exti_clear_pending(1U);
        spi_button_pressed = 1U;
    }
}

static uint8_t take_button_event(volatile uint8_t *event)
{
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    uint8_t was_pressed = *event;
    *event = 0U;
    __set_PRIMASK(primask);
    return was_pressed;
}

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* SPI1 master, 8-bit, MSB first, mode 0. PA4 is the manual chip select. */
    const SPI_Config_t spi_config = {
        .mode = DRIVER_SPI_MODE_MASTER,
        .baudrate = DRIVER_SPI_BAUDRATE_DIV128,
        .cpol = DRIVER_SPI_CPOL_LOW,
        .cpha = DRIVER_SPI_CPHA_1EDGE,
        .data_frame = DRIVER_SPI_DFF_8BIT,
        .first_bit = DRIVER_SPI_FIRSTBIT_MSB
    };
    driver_spi_init(SPI1, &spi_config);
    driver_spi_cs_high(GPIOA, 4U);

    /* USART1 TX only, 9600 baud, 8 data bits, no parity, 1 stop bit. */
    driver_usart_clock_enable(USART1);
    const USART_Config_t usart_config = {
        .baudrate = 9600U,
        .word_length = DRIVER_USART_WORDLENGTH_8B,
        .stop_bits = DRIVER_USART_STOPBITS_1,
        .parity = DRIVER_USART_PARITY_NONE,
        .mode = DRIVER_USART_MODE_TX,
        .hw_flow_control = DRIVER_USART_HWCONTROL_NONE
    };
    driver_usart_init(USART1, &usart_config);

    /* PB0 button sends the USART payload; PB1 button sends the SPI payload. */
    driver_exti_init(GPIOB, 0U, DRIVER_EXTI_TRIGGER_FALLING, 2U, 0U);
    driver_exti_init(GPIOB, 1U, DRIVER_EXTI_TRIGGER_FALLING, 2U, 0U);

    while (1) {
        if (take_button_event(&usart_button_pressed) != 0U) {
            dma_usart_send_string(usart_message);
        }

        if (take_button_event(&spi_button_pressed) != 0U) {
            driver_spi_cs_low(GPIOA, 4U);
            driver_spi_transmit(SPI1, spi_message,
                                (uint16_t)(sizeof(spi_message) - 1U));
            driver_spi_cs_high(GPIOA, 4U);
        }

        __WFI();
    }
}

void Error_Handler(void)
{
    __disable_irq();
    while (1) {
    }
}
