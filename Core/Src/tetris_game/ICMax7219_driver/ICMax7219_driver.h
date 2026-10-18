//
// Created by Nguyen Hoang Viet on 04/07/2026.
//

#include "stdint.h"

#ifndef MATRIXLEDSPI_ICMAX7219_DRIVER_H
#define MATRIXLEDSPI_ICMAX7219_DRIVER_H

#define REG_DIGIT_1      0x01
#define REG_DIGIT_2      0x02
#define REG_DIGIT_3      0x03
#define REG_DIGIT_4      0x04
#define REG_DIGIT_5      0x05
#define REG_DIGIT_6      0x06
#define REG_DIGIT_7      0x07
#define REG_DIGIT_8      0x08
#define REG_DECODE_MODE  0x09
#define REG_INTENSITY    0x0A
#define REG_SCAN_LIMIT   0x0B
#define REG_SHUTDOWN     0x0C
#define REG_DISPLAY_TEST 0x0F

void MAX7219_Init_System(void);
void MAX7219_Write_Command(uint8_t reg_address, uint8_t data_value);
// void MAX7219_Test_Checkerboard(void);
#endif //MATRIXLEDSPI_ICMAX7219_DRIVER_H
