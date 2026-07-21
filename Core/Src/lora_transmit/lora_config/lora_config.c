//
// Created by Nguyen Hoang Viet on 19/07/2026.
//

#include "lora_config.h"
#include "peripheral/usart/usart.h"
#include "main.h"  // Chứa macro LORA_M0_Pin, LORA_M1_Pin, v.v.
#include <string.h>

#define LORA_CONFIG_TIMEOUT_MS  1500U

// Thay thế hàm chờ AUX bằng Delay tĩnh vì chân AUX bị hỏng phần cứng
static void LoRa_Change_Mode_Blind(GPIO_PinState m0_state, GPIO_PinState m1_state)
{
    HAL_GPIO_WritePin(LORA_M0_GPIO_Port, LORA_M0_Pin, m0_state);
    HAL_GPIO_WritePin(LORA_M1_GPIO_Port, LORA_M1_Pin, m1_state);
    
    // Đợi 100ms cố định để module LoRa đổi chế độ an toàn
    HAL_Delay(100);
}

LoRa_Config_Status LoRa_Configure_Module(uint16_t address, uint8_t channel)
{
    uint8_t config_cmd[6];
    uint8_t config_resp[6] = {0};

    // 1. Tắt ngắt nhận UART và đưa trạng thái về READY an toàn
    __HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE);
    HAL_UART_AbortReceive(&huart1);
    huart1.RxState = HAL_UART_STATE_READY;

    // 2. Chuyển LoRa sang chế độ cấu hình (M0 = 1, M1 = 1)
    LoRa_Change_Mode_Blind(GPIO_PIN_SET, GPIO_PIN_SET);

    // 3. Chuẩn bị tập lệnh cấu hình 6-byte cho E32
    config_cmd[0] = 0xC0;               // Mở đầu: Cấu hình lưu trữ khi mất điện
    config_cmd[1] = (address >> 8) & 0xFF; // ADDH (Địa chỉ cao)
    config_cmd[2] = address & 0xFF;        // ADDL (Địa chỉ thấp)
    config_cmd[3] = 0x1A;               // SPED: 8N1, 9600bps, Air data rate 2.4kbps
    config_cmd[4] = channel;            // CHAN: Kênh tần số (tần số = 410M + CHAN * 1M)
    config_cmd[5] = 0xC4;               // OPTION: Chế độ truyền FIXED TRANSMISSION (0xC4), công suất tối đa

    // Xóa cờ Overrun và giải phóng bộ đệm rác UART trước khi gửi nhận cấu hình
    __HAL_UART_CLEAR_OREFLAG(&huart1);
    uint32_t temp_read = huart1.Instance->DR; 
    (void)temp_read;

    // 4. Gửi 6 byte cấu hình sang LoRa qua USART1 (dạng blocking)
    if (USART1_Send_Data(config_cmd, sizeof(config_cmd), LORA_CONFIG_TIMEOUT_MS) != HAL_OK)
    {
        // Đưa về chế độ Normal và kích hoạt lại ngắt nhận UART
        LoRa_Change_Mode_Blind(GPIO_PIN_RESET, GPIO_PIN_RESET);
        USART1_Start_Receive_IT();
        return LORA_CONFIG_ERROR_TIMEOUT;
    }

    // 5. Chờ LoRa phản hồi lại 6 byte cấu hình (dạng blocking)
    if (USART1_Receive_Data(config_resp, sizeof(config_resp), LORA_CONFIG_TIMEOUT_MS) != HAL_OK)
    {
        LoRa_Change_Mode_Blind(GPIO_PIN_RESET, GPIO_PIN_RESET);
        USART1_Start_Receive_IT();
        return LORA_CONFIG_ERROR_TIMEOUT;
    }

    // 6. Cấu hình thành công, đưa LoRa về chế độ chạy bình thường (M0 = 0, M1 = 0)
    LoRa_Change_Mode_Blind(GPIO_PIN_RESET, GPIO_PIN_RESET);

    // 7. Kích hoạt lại chế độ nhận ngắt IT chuẩn bị nhận dữ liệu không dây
    USART1_Start_Receive_IT();

    // 8. Kiểm tra phản hồi có khớp với lệnh gửi đi không
    if (memcmp(config_cmd, config_resp, sizeof(config_cmd)) != 0)
    {
        return LORA_CONFIG_ERROR_RESPONSE;
    }

    return LORA_CONFIG_OK;
}
