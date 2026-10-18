//
// Created by Nguyen Hoang Viet on 26/07/2026.
//

#ifndef EMBEDDED_SYSTEM_HC_SR04_H
#define EMBEDDED_SYSTEM_HC_SR04_H

#include "stm32f1xx_hal.h"
#include "stdint.h"

// Maximum measurable distance is ~400cm (timeout = 400 * 58 = 23200 us)
#define HC_SR04_MAX_DISTANCE_CM     400U
#define HC_SR04_TIMEOUT_US          24000U

typedef enum {
    HC_SR04_OK = 0,
    HC_SR04_ERROR_TIMEOUT,
    HC_SR04_ERROR_ARGUMENT
} HC_SR04_Status;

typedef struct {
    float    distance_cm;   // Measured distance in centimeters
    uint32_t pulse_us;      // Raw echo pulse width in microseconds
} HC_SR04_Data;

/**
 * @brief  Initialize HC-SR04 driver and the GPIO pins for TRIG and ECHO.
 * @param  trig_port  GPIO port of TRIG pin (e.g. GPIOB)
 * @param  trig_pin   GPIO pin number of TRIG pin (e.g. GPIO_PIN_0)
 * @param  echo_port  GPIO port of ECHO pin (e.g. GPIOB)
 * @param  echo_pin   GPIO pin number of ECHO pin (e.g. GPIO_PIN_1)
 */
void HC_SR04_Init(GPIO_TypeDef *trig_port, uint16_t trig_pin,
                  GPIO_TypeDef *echo_port, uint16_t echo_pin);

/**
 * @brief  Trigger a distance measurement and read the result.
 * @param  data  Pointer to HC_SR04_Data struct to store the result.
 * @retval HC_SR04_OK on success, HC_SR04_ERROR_TIMEOUT if no echo received,
 *         HC_SR04_ERROR_ARGUMENT if data or pins are invalid.
 */
HC_SR04_Status HC_SR04_Read(HC_SR04_Data *data);

#endif // EMBEDDED_SYSTEM_HC_SR04_H
