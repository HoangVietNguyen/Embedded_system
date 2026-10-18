//
// Created by Nguyen Hoang Viet on 16/07/2026.
//

#ifndef MATRIXLEDSPI_ADC_H
#define MATRIXLEDSPI_ADC_H

#include "stm32f1xx_hal.h"
#include "stdint.h"

#define ADC1_REFERENCE_MV 3300U
#define ADC1_MAX_VALUE    4095U

extern ADC_HandleTypeDef hadc1;

void MX_ADC1_Init(void);
uint16_t ADC1_Read_Raw(void);
uint16_t ADC1_Read_Millivolt(void);

#endif //MATRIXLEDSPI_ADC_H
