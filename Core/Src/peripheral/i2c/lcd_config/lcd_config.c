#include "lcd_config.h"
#include "peripheral/i2c/i2c/i2c.h"
#include "driver_i2c.h"

void lcd_send_cmd(char cmd) {
    char data_u, data_l;
    uint8_t data_t[4];
    data_u = (cmd & 0xF0 );
    data_l = ((cmd << 4) & 0xF0);
    data_t[0] = data_u | 0x0C;  // RS = 0, EN = 1
    data_t[1] = data_u | 0x08;  // RS = 0, EN = 0
    data_t[2] = data_l | 0x0C;  // RS = 0, EN = 1
    data_t[3] = data_l | 0x08;  // RS = 0, EN = 0
    driver_i2c_master_transmit(I2C1, LCD_ADDR, data_t, 4);
}

void lcd_send_data(char data) {
    char data_u, data_l;
    uint8_t data_t[4];
    data_u = (data & 0xF0 );
    data_l = ((data << 4) & 0xF0);
    data_t[0] = data_u | 0x0D;  // RS = 1, EN = 1  (Vì 0x0C | 0x01 = 0x0D)
    data_t[1] = data_u | 0x09;  // RS = 1, EN = 0  (Vì 0x08 | 0x01 = 0x09)
    data_t[2] = data_l | 0x0D;  // RS = 1, EN = 1
    data_t[3] = data_l | 0x09;  // RS = 1, EN = 0
    driver_i2c_master_transmit(I2C1, LCD_ADDR, data_t, 4);
}


void lcd_init(void) {
    HAL_Delay(50);
    lcd_send_cmd(0x30);
    HAL_Delay(5);
    lcd_send_cmd(0x30);
    HAL_Delay(1);
    lcd_send_cmd(0x32);
    HAL_Delay(10);

    lcd_send_cmd(0x28);
    HAL_Delay(1);
    lcd_send_cmd(0x0C);
    HAL_Delay(1);
    lcd_send_cmd(0x01);
    HAL_Delay(2);
    lcd_send_cmd(0x06);
}

void lcd_goto_xy(uint8_t row, uint8_t col) {
    uint8_t address;
    if (row == 0) {
        address = 0x80 + col;
    } else {
        address = 0xC0 + col;
    }
    lcd_send_cmd(address);
}

void lcd_send_string(char *str) {
    while (*str) lcd_send_data(*str++);
}
