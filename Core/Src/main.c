#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lora_transmit/lora_receive/lora_receive.h"
#include "lora_transmit/lora_config/lora_config.h" // Import thư viện cấu hình LoRa
#include "peripheral/spi/spi.h"
#include "peripheral/gpio/gpio.h"
#include "peripheral/rcc/rcc.h"
#include "peripheral/tim/tim.h"
#include "peripheral/i2c/i2c/i2c.h"
#include "peripheral/i2c/lcd_config/lcd_config.h"
#include "peripheral/usart/usart.h"

// Buffer tạm thời để đọc chuỗi ra xử lý hiển thị LCD
#define RX_BUFFER_SIZE 64
uint8_t rx_string_buffer[RX_BUFFER_SIZE];

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  // Khởi tạo phần cứng cơ bản
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();
  
  // Khởi tạo USART1 (Cấu hình cứng và bật ngắt nhận 1-byte)
  MX_USART1_UART_Init();

  // Khởi tạo bộ đệm phần mềm cho LoRa
  LoRa_Receive_Init();

  // Khởi động màn hình LCD
  lcd_init();
  lcd_goto_xy(0, 0);
  lcd_send_string("System Init...  ");
  HAL_Delay(500);

  // Thực hiện cấu hình module LoRa E32 sang chế độ Transparent, cùng Address & Channel
  // (Đoạn này tự động kéo chân M0/M1 lên 1 để nạp cấu hình và kéo về 0 sau khi xong)
  LoRa_Config_Status config_status = LoRa_Configure_Module(LORA_ADDRESS, LORA_CHANNEL);

  lcd_goto_xy(0, 0);
  if (config_status == LORA_CONFIG_OK) {
    lcd_send_string("Config: OK      ");
  } else {
    lcd_send_string("Config: FAIL    ");
  }
  
  lcd_goto_xy(1, 0);
  lcd_send_string("Waiting data... ");

  while (1)
  {
    // Kiểm tra bộ đệm nhận LoRa có byte nào không
    uint16_t available = LoRa_Receive_Available();
    if (available > 0)
    {
      // Chờ 50ms để dữ liệu UART truyền hết trọn vẹn toàn bộ chuỗi ký tự
      HAL_Delay(50);

      uint16_t rx_len = 0;
      memset(rx_string_buffer, 0, sizeof(rx_string_buffer));

      // Đọc toàn bộ gói tin từ Ring Buffer mềm
      if (LoRa_Receive_Read(rx_string_buffer, sizeof(rx_string_buffer) - 1, &rx_len) == LORA_RECEIVE_OK && rx_len > 0)
      {
        rx_string_buffer[rx_len] = '\0'; // Kết thúc chuỗi C-String

        int dist_int = 0, dist_dec = 0;
        // Phân tích chuỗi định dạng khoảng cách: "Dist: XXX.X cm"
        // Dùng %d.%d thay vì %f để tránh giới hạn sscanf float của newlib-nano
        if (sscanf((char *)rx_string_buffer, "Dist: %d.%d cm", &dist_int, &dist_dec) == 2) {
          char lcd_row0[17];
          char lcd_row1[17];

          // Dòng 1: "Φ 00.0 cm  " - khoảng cách cm
          snprintf(lcd_row0, sizeof(lcd_row0), "Dist: %d.%d cm  ", dist_int, dist_dec);
          // Dòng 2: khoảng cách mm (dist_int * 10 + dist_dec)
          snprintf(lcd_row1, sizeof(lcd_row1), "     %d mm      ", dist_int * 10 + dist_dec);

          lcd_goto_xy(0, 0);
          lcd_send_string(lcd_row0);
          lcd_goto_xy(1, 0);
          lcd_send_string(lcd_row1);
        }
        else
        {
          // Nếu nhận chuỗi lỗi (như "HC-SR04 Err") hoặc định dạng không hợp lệ
          char lcd_row0[17];
          snprintf(lcd_row0, sizeof(lcd_row0), "%-16s", (char *)rx_string_buffer);
          lcd_goto_xy(0, 0);
          lcd_send_string(lcd_row0);

          lcd_goto_xy(1, 0);
          lcd_send_string("Waiting data... ");
        }
      }
    }
    HAL_Delay(10); // Giảm tải CPU
  }
}

// Bắt sự kiện ngắt nhận UART và đẩy thẳng byte vào buffer LoRa
void USART1_RxEventCallback(uint8_t received_data)
{
  LoRa_Receive_Push_Byte(received_data);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
#ifdef USE_FULL_ASSERT
#endif
