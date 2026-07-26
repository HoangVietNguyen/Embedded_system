//
// Created by Nguyen Hoang Viet on 19/07/2026.
//

#ifndef MATRIXLEDSPI_LORA_CONFIG_H
#define MATRIXLEDSPI_LORA_CONFIG_H

#include "stm32f1xx_hal.h"
#include "stdint.h"

// Địa chỉ mặc định cho thiết bị gửi
#define LORA_ADDRESS        0x0000U  
#define LORA_CHANNEL        0x17U    // Channel 23 (433 MHz)

typedef enum {
    LORA_CONFIG_OK = 0,
    LORA_CONFIG_ERROR_TIMEOUT,
    LORA_CONFIG_ERROR_RESPONSE,
    LORA_CONFIG_ERROR_PARAMETER
} LoRa_Config_Status;

// Hàm cấu hình mô-đun LoRa E32
// Lưu ý: Trước khi gọi hàm này, chân M0 và M1 của LoRa phải được đặt lên mức HIGH (M0=1, M1=1) để vào chế độ cấu hình.
LoRa_Config_Status LoRa_Configure_Module(uint16_t address, uint8_t channel);

#endif //MATRIXLEDSPI_LORA_CONFIG_H
