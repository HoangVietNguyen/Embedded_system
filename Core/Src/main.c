#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "peripheral/spi/spi.h"
#include "peripheral/gpio/gpio.h"
#include "peripheral/rcc/rcc.h"
#include "peripheral/tim/tim.h"
#include "peripheral/i2c/i2c/i2c.h"
#include "peripheral/i2c/lcd_config/lcd_config.h"
#include "peripheral/usart/usart.h"

// ============================================================================
// USART Receive Buffer
// Tích lũy từng byte từ interrupt, xử lý khi gặp ký tự '\n'
// ============================================================================
#define USART_RX_BUF_SIZE 64

static volatile uint8_t usart_rx_buf[USART_RX_BUF_SIZE];
static volatile uint8_t usart_rx_idx    = 0;
static volatile uint8_t usart_msg_ready = 0;

#define RX_BUFFER_SIZE 64

uint8_t rx_data;                         // Biến tạm chứa 1 byte vừa nhận
char rx_buffer[RX_BUFFER_SIZE];          // Mảng chứa cả chuỗi "Dist: %d.%d cm\n"
uint8_t rx_index = 0;                    // Con trỏ vị trí mảng
volatile uint8_t string_complete = 0;
uint8_t is_capturing = 0; // Cờ đánh dấu đã tìm thấy đầu chuỗi chưa

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  // Khởi tạo ngoại vi
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_I2C1_Init();
  MX_TIM1_Init();

  // USART1: PA9=TX, PA10=RX — nhận dữ liệu từ board truyền qua dây nối trực tiếp
  MX_USART1_UART_Init();

  // Màn hình LCD
  lcd_init();
  lcd_goto_xy(0, 0);
  lcd_send_string("USART Receiver  ");
  lcd_goto_xy(1, 0);
  lcd_send_string("Waiting data... ");

  while (1)
  {
    if (string_complete) {
      int dist_int = 0;  // Phần nguyên (ví dụ: 12)
      int dist_dec = 0;  // Phần thập phân (ví dụ: 5)

      // Bóc tách dữ liệu từ chuỗi rx_buffer
      // Hàm sscanf sẽ tìm đúng định dạng "Dist: %d.%d cm" để trích xuất 2 số
      int parsed_items = sscanf(rx_buffer, "Dist: %d.%d cm", &dist_int, &dist_dec);

      if (parsed_items == 2) {
        // Bóc tách thành công! Ông đã có 2 giá trị dist_int và dist_dec
        // Ví dụ: In ra màn hình TFT hoặc xử lý logic tiếp theo tại đây

        char lcd_buffer[20];
        sprintf(lcd_buffer, "Distance: %d.%d cm", dist_int, dist_dec);
        lcd_goto_xy(1, 0);
        lcd_send_string(lcd_buffer);
      }

      // Dọn dẹp cờ và bộ đệm để đón chuỗi mới
      rx_index = 0;
      string_complete = 0;
    }

    /* USER CODE END WHILE */
  }
}

// ============================================================================
// USART1 Receive Callback — được gọi mỗi khi nhận 1 byte qua PA10 (RX)
// Tích lũy byte vào buffer; khi gặp '\n' thì đánh dấu tin nhắn đã hoàn chỉnh
// ============================================================================
/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
  if (huart->Instance == USART1) {
    // Nếu bắt gặp ký tự bắt đầu 'D' -> Bắt đầu ghi vào index 0
    if (rx_data == 'D') {
      is_capturing = 1;
      rx_index = 0;
    }

    // Nếu đang trong tiến trình bắt chuỗi
    if (is_capturing) {
      rx_buffer[rx_index] = rx_data;

      // Gặp ký tự xuống dòng '\n' -> Chốt sổ!
      if (rx_data == '\n') {
        rx_buffer[rx_index] = '\0'; // Ghi đè '\n' thành '\0' luôn
        string_complete = 1;        // Báo cho loop chính biết đã xong
        is_capturing = 0;           // Tắt cờ bắt
      } else {
        rx_index++;
      }
    }

    // Kích hoạt lại ngắt nhận
    HAL_UART_Receive_IT(&huart1, &rx_data, 1);
  }
}
/* USER CODE END 4 */

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
#ifdef USE_FULL_ASSERT
#endif
