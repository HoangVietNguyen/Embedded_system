//
// Created by Nguyen Hoang Viet on 26/07/2026.
//

#include "hc_sr04.h"

// ============================================================================
// Private variables
// ============================================================================
static GPIO_TypeDef *hc_trig_port = NULL;
static uint16_t      hc_trig_pin  = 0;
static GPIO_TypeDef *hc_echo_port = NULL;
static uint16_t      hc_echo_pin  = 0;

// ============================================================================
// Private function prototypes
// ============================================================================
static void     hcsr04_delay_us(uint32_t us);
static void     hcsr04_init_trig_pin(void);
static void     hcsr04_init_echo_pin(void);

// ============================================================================
// Public functions
// ============================================================================

void HC_SR04_Init(GPIO_TypeDef *trig_port, uint16_t trig_pin,
                  GPIO_TypeDef *echo_port, uint16_t echo_pin)
{
    hc_trig_port = trig_port;
    hc_trig_pin  = trig_pin;
    hc_echo_port = echo_port;
    hc_echo_pin  = echo_pin;

    // Enable DWT cycle counter for microsecond timing (same as DHT11 driver)
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;

    // Initialize GPIO pins
    hcsr04_init_trig_pin();
    hcsr04_init_echo_pin();

    // Ensure TRIG is LOW initially
    HAL_GPIO_WritePin(hc_trig_port, hc_trig_pin, GPIO_PIN_RESET);
}

HC_SR04_Status HC_SR04_Read(HC_SR04_Data *data)
{
    uint32_t pulse_start = 0;
    uint32_t pulse_end   = 0;
    uint32_t elapsed_us  = 0;
    uint32_t timeout_us  = 0;

    if (data == NULL || hc_trig_port == NULL || hc_echo_port == NULL) {
        return HC_SR04_ERROR_ARGUMENT;
    }

    // ------------------------------------------------------------------
    // STEP 1: Send a 10 us HIGH pulse on the TRIG pin
    // ------------------------------------------------------------------
    HAL_GPIO_WritePin(hc_trig_port, hc_trig_pin, GPIO_PIN_RESET);
    hcsr04_delay_us(2);

    HAL_GPIO_WritePin(hc_trig_port, hc_trig_pin, GPIO_PIN_SET);
    hcsr04_delay_us(10);
    HAL_GPIO_WritePin(hc_trig_port, hc_trig_pin, GPIO_PIN_RESET);

    // ------------------------------------------------------------------
    // STEP 2: Wait for ECHO to go HIGH (sensor starts transmitting)
    // ------------------------------------------------------------------
    timeout_us = 0;
    while (HAL_GPIO_ReadPin(hc_echo_port, hc_echo_pin) == GPIO_PIN_RESET) {
        hcsr04_delay_us(1);
        timeout_us++;
        if (timeout_us > HC_SR04_TIMEOUT_US) {
            return HC_SR04_ERROR_TIMEOUT;
        }
    }
    pulse_start = DWT->CYCCNT;

    // ------------------------------------------------------------------
    // STEP 3: Wait for ECHO to go LOW (echo received back)
    // ------------------------------------------------------------------
    timeout_us = 0;
    while (HAL_GPIO_ReadPin(hc_echo_port, hc_echo_pin) == GPIO_PIN_SET) {
        hcsr04_delay_us(1);
        timeout_us++;
        if (timeout_us > HC_SR04_TIMEOUT_US) {
            return HC_SR04_ERROR_TIMEOUT;
        }
    }
    pulse_end = DWT->CYCCNT;

    // ------------------------------------------------------------------
    // STEP 4: Calculate distance
    // Pulse width (us) = (DWT ticks difference) / (SystemCoreClock / 1000000)
    // Distance (cm) = pulse_us / 58
    // (Speed of sound ≈ 340 m/s = 0.034 cm/us → round trip / 2: distance = us * 0.034 / 2 = us / 58.82)
    // ------------------------------------------------------------------
    elapsed_us = (pulse_end - pulse_start) / (SystemCoreClock / 1000000U);

    data->pulse_us    = elapsed_us;
    data->distance_cm = (float)elapsed_us / 58.0f;

    // Clamp to valid range
    if (data->distance_cm > (float)HC_SR04_MAX_DISTANCE_CM) {
        return HC_SR04_ERROR_TIMEOUT;
    }

    return HC_SR04_OK;
}

// ============================================================================
// Private functions
// ============================================================================

static void hcsr04_delay_us(uint32_t us)
{
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = us * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < ticks) {
    }
}

static void hcsr04_init_trig_pin(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin   = hc_trig_pin;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;  // Push-pull output
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(hc_trig_port, &GPIO_InitStruct);
}

static void hcsr04_init_echo_pin(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin  = hc_echo_pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;       // Floating input (ECHO outputs 3.3V max)
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(hc_echo_port, &GPIO_InitStruct);
}
