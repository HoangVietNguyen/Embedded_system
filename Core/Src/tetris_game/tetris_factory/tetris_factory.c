//
// Created by Nguyen Hoang Viet on 28/06/2026.
//
#include "tetris_factory.h"

uint16_t TETRIX_SHAPE[8][8] = {
    // 1. Khối ô vuông
    {
        0x00,
        0x00, //
        0x00, //
        0x00, //
        0x03,
        0x03,
        0x00,
        0x00//
    },
    // 2. Khối chữ L thẳng
    {
        0x00,
        0x00, //
        0x00, //
        0x00, //
        0x0F,
        0x00,
        0x00,
        0x00//
    },
    // 3. Khối chữ T nằm dọc
    {0x00,
        0x00,
        0x00,
        0x00, //   ■
        0x07, // ■■■
        0x02, //
        0x00,
        0x00//
    },
    //4. khối chữ Z
    {
        0x00,
        0x00,
        0x01,
        0x03,
        0x02,
        0x00,
        0x00,
        0x00
    },
    {
        0x00,
        0x00,
        0x01,
        0x03,
        0x02,
        0x00,
        0x00,
        0x00
    },
    // 5. Khối chữ S (S-Piece)
    {
        0x00,
        0x00, //  ■■
        0x00, // ■■
        0x02, //
        0x03,
        0x01,
        0x00,
        0x00//
    },
    // 6. Khối chữ T (T-Piece)
    {
        0x00,
        0x00, //  ■
        0x00, // ■■■
        0x01, //
        0x03,
        0x01,
        0x00,
        0x00//
    },
    // 7. Khối chữ  L nằm ngang
    {
        0x00,
        0x00, // ■■
        0x00, //  ■■
        0x01, //
        0x01,
        0x01,
        0x01,
        0x00//
    }
};
