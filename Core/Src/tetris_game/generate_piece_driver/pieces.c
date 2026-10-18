//
// Created by Nguyen Hoang Viet on 04/07/2026.
//

#include "pieces.h"
#include "stdint.h"
#include "stdlib.h"
#include "tetris_game/tetris_factory/tetris_factory.h"
#include "tetris_game/ICMax7219_driver/ICMax7219_driver.h"
#include "stdbool.h"


volatile uint16_t CURRENT_PIECE[8][8];
volatile uint8_t random_index;

bool Is_Matrix_Empty(uint16_t matrix[8][8]) {
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (matrix[i][j] != 0x00) {
                return false; // Phát hiện có "vết", mảng không trống!
            }
        }
    }
    return true; // Quét hết sạch không thấy gì -> Mảng trống hoàn toàn
}

void MAX7219_Generate_Pieces() {
    if (!Is_Matrix_Empty(CURRENT_PIECE)) {
        return;
    }
    random_index = rand() % 7;
    for (uint8_t row = 0; row < 8; row++) {
        MAX7219_Write_Command(row, TETRIX_SHAPE[random_index][row]);
        CURRENT_PIECE[random_index][row] = TETRIX_SHAPE[random_index][row];
    }
}

void MAX7219_Shift_Down_Pieces() {
    for (uint8_t row = 0; row < 8; row++) {
        // Lấy dữ liệu ra, dịch trái 1 bit rồi mới gửi sang IC
        CURRENT_PIECE[random_index][row] = CURRENT_PIECE[random_index][row] << 1;

        MAX7219_Write_Command(row, CURRENT_PIECE[random_index][row]);
    }
}

void MAX7219_Shift_Left_Pieces() {
    for (uint8_t i = 0; i < 8; i++) {
        CURRENT_PIECE[random_index][i] = CURRENT_PIECE[random_index][i+1];
    }
}

void MAX7219_Shift_Right_Pieces() {
    for (uint8_t i = 8; i > 8; i--) {
        CURRENT_PIECE[random_index][i] = CURRENT_PIECE[random_index][i-1];
    }
}
