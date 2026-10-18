//
// Created by Nguyen Hoang Viet on 04/07/2026.
//

#include "ICMax7219_driver.h"
#include "peripheral/spi/spi.h"

#include "main.h"
#include "stdint.h"
#include "stm32f1xx_hal_gpio.h"

void MAX7219_Init_System(void) {
    // Dòng 1 bị lỗi vì đang để trống, ông cần điền thanh ghi TEST và giá trị tắt (0x00) vào đây
    MAX7219_Write_Command(REG_DISPLAY_TEST, 0x00); // Tắt chế độ test cưỡng bức

    // Các dòng dưới bản chất code chạy CHỈ CẦN thế này là đủ:
    MAX7219_Write_Command(REG_SCAN_LIMIT, 0x07);   // Quét sạch cả 8 hàng LED
    MAX7219_Write_Command(REG_DECODE_MODE, 0x00);  // Chế độ ma trận điểm thuần túy
    MAX7219_Write_Command(REG_INTENSITY, 0x03);    // Độ sáng vừa phải chống sụt nguồn
    MAX7219_Write_Command(REG_SHUTDOWN, 0x01);     // Đánh thức IC hoạt động bình thường
}

void MAX7219_Write_Command(uint8_t reg_address, uint8_t data_value) {
    uint8_t packet[2] = {reg_address, data_value};
    uint8_t no_op_packet[2] = {0x00, 0x00}; // Thanh ghi No-Op (0x00) bảo IC không làm gì cả

    // 1. Hạ chân CS (PA6) xuống mức LOW để mở cổng nhận dữ liệu
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);

    // 2. Gửi 3 gói No-Op trước để đẩy dữ liệu cũ ra các con IC phía sau (IC 4, IC 3, IC 2)
    for(int i = 0; i < 3; i++) {
        HAL_SPI_Transmit(&hspi1, no_op_packet, 2, HAL_MAX_DELAY);
    }

    // 3. Gửi gói dữ liệu thật CUỐI CÙNG -> Nó sẽ nằm lại ở con IC đầu tiên (IC 1)
    HAL_SPI_Transmit(&hspi1, packet, 2, HAL_MAX_DELAY);

    // 4. Kéo chân CS (PA6) lên mức HIGH để chốt dữ liệu
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
}
