#ifndef MATRIXLEDSPI_INTERRUPT_H
#define MATRIXLEDSPI_INTERRUPT_H

#include "main.h"

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN);
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

#endif //MATRIXLEDSPI_INTERRUPT_H
