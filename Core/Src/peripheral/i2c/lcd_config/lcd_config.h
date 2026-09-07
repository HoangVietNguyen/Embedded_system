#ifndef MATRIXLEDSPI_LCD_CONFIG_H
#define MATRIXLEDSPI_LCD_CONFIG_H

#include "main.h"

#define LCD_ADDR 0x27

void lcd_send_cmd(char cmd);
void lcd_send_data(char data);
void lcd_init(void);
void lcd_goto_xy(uint8_t row, uint8_t col);
void lcd_send_string(char *str);

#endif
