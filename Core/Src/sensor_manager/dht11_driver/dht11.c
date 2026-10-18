//
// Created by Nguyen Hoang Viet on 16/07/2026.
//

#include "dht11.h"

#define DHT11_START_SIGNAL_MS 18U
#define DHT11_RESPONSE_TIMEOUT_US 120U
#define DHT11_BIT_TIMEOUT_US 100U
#define DHT11_BIT_HIGH_THRESHOLD_US 40U

static GPIO_TypeDef *dht11_gpio_port = NULL;
static uint16_t dht11_gpio_pin = 0;

static void dht11_delay_us(uint32_t delay_us);
static void dht11_set_pin_output(void);
static void dht11_set_pin_input(void);
static DHT11_Status dht11_wait_for_pin(GPIO_PinState state, uint32_t timeout_us);

void DHT11_Init(GPIO_TypeDef *gpio_port, uint16_t gpio_pin) {
    dht11_gpio_port = gpio_port;
    dht11_gpio_pin = gpio_pin;

    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;

    dht11_set_pin_input();
}

DHT11_Status DHT11_Read(DHT11_Data *data) {
    uint8_t received_data[5] = {0};

    if (data == NULL || dht11_gpio_port == NULL || dht11_gpio_pin == 0) {
        return DHT11_ERROR_ARGUMENT;
    }

    dht11_set_pin_output();
    HAL_GPIO_WritePin(dht11_gpio_port, dht11_gpio_pin, GPIO_PIN_RESET);
    HAL_Delay(DHT11_START_SIGNAL_MS);

    dht11_set_pin_input();
    dht11_delay_us(30);

    if (dht11_wait_for_pin(GPIO_PIN_RESET, DHT11_RESPONSE_TIMEOUT_US) != DHT11_OK) {
        return DHT11_ERROR_TIMEOUT;
    }
    if (dht11_wait_for_pin(GPIO_PIN_SET, DHT11_RESPONSE_TIMEOUT_US) != DHT11_OK) {
        return DHT11_ERROR_TIMEOUT;
    }
    if (dht11_wait_for_pin(GPIO_PIN_RESET, DHT11_RESPONSE_TIMEOUT_US) != DHT11_OK) {
        return DHT11_ERROR_TIMEOUT;
    }

    for (uint8_t byte_index = 0; byte_index < 5; byte_index++) {
        for (uint8_t bit_index = 0; bit_index < 8; bit_index++) {
            uint32_t high_time_us = 0;

            if (dht11_wait_for_pin(GPIO_PIN_SET, DHT11_BIT_TIMEOUT_US) != DHT11_OK) {
                return DHT11_ERROR_TIMEOUT;
            }

            while (HAL_GPIO_ReadPin(dht11_gpio_port, dht11_gpio_pin) == GPIO_PIN_SET) {
                dht11_delay_us(1);
                high_time_us++;

                if (high_time_us > DHT11_BIT_TIMEOUT_US) {
                    return DHT11_ERROR_TIMEOUT;
                }
            }

            received_data[byte_index] <<= 1;
            if (high_time_us > DHT11_BIT_HIGH_THRESHOLD_US) {
                received_data[byte_index] |= 1;
            }
        }
    }

    uint8_t checksum = received_data[0] + received_data[1] + received_data[2] + received_data[3];
    if (checksum != received_data[4]) {
        return DHT11_ERROR_CHECKSUM;
    }

    data->humidity = received_data[0];
    data->humidity_decimal = received_data[1];
    data->temperature = received_data[2];
    data->temperature_decimal = received_data[3];

    return DHT11_OK;
}

static void dht11_delay_us(uint32_t delay_us) {
    uint32_t start_tick = DWT->CYCCNT;
    uint32_t delay_tick = delay_us * (SystemCoreClock / 1000000U);

    while ((DWT->CYCCNT - start_tick) < delay_tick) {
    }
}

static void dht11_set_pin_output(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = dht11_gpio_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(dht11_gpio_port, &GPIO_InitStruct);
}

static void dht11_set_pin_input(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = dht11_gpio_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(dht11_gpio_port, &GPIO_InitStruct);
}

static DHT11_Status dht11_wait_for_pin(GPIO_PinState state, uint32_t timeout_us) {
    uint32_t waited_us = 0;

    while (HAL_GPIO_ReadPin(dht11_gpio_port, dht11_gpio_pin) != state) {
        dht11_delay_us(1);
        waited_us++;

        if (waited_us > timeout_us) {
            return DHT11_ERROR_TIMEOUT;
        }
    }

    return DHT11_OK;
}
