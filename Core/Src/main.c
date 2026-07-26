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
    // Chờ cho đến khi nhận đủ một dòng dữ liệu (kết thúc bằng '\n')
    if (usart_msg_ready)
    {
      usart_msg_ready = 0;

      int dist_int = 0, dist_dec = 0;

      // Phân tích chuỗi "Dist: XXX.X cm"
      if (sscanf((char *)usart_rx_buf, "Dist: %d.%d cm", &dist_int, &dist_dec) == 2)
      {
        char lcd_row0[17];
        char lcd_row1[17];

        // Dòng 0: khoảng cách cm — ví dụ "Dist: 123.4 cm  "
        snprintf(lcd_row0, sizeof(lcd_row0), "Dist: %d.%d cm  ", dist_int, dist_dec);
        // Dòng 1: khoảng cách mm — ví dụ "     1234 mm    "
        snprintf(lcd_row1, sizeof(lcd_row1), "     %d mm      ", dist_int * 10 + dist_dec);

        lcd_goto_xy(0, 0);
        lcd_send_string(lcd_row0);
        lcd_goto_xy(1, 0);
        lcd_send_string(lcd_row1);
      }
      else
      {
        // Nhận được chuỗi lỗi hoặc định dạng không hợp lệ
        char lcd_row0[17];
        snprintf(lcd_row0, sizeof(lcd_row0), "%-16s", (char *)usart_rx_buf);
        lcd_goto_xy(0, 0);
        lcd_send_string(lcd_row0);
        lcd_goto_xy(1, 0);
        lcd_send_string("Waiting data... ");
      }
    }

    HAL_Delay(10); // Giảm tải CPU trong vòng lặp chờ
  }
}

// ============================================================================
// USART1 Receive Callback — được gọi mỗi khi nhận 1 byte qua PA10 (RX)
// Tích lũy byte vào buffer; khi gặp '\n' thì đánh dấu tin nhắn đã hoàn chỉnh
// ============================================================================
void USART1_RxEventCallback(uint8_t received_data)
{
  if (received_data == '\n')
  {
    // Kết thúc chuỗi — null-terminate và bật cờ xử lý
    usart_rx_buf[usart_rx_idx] = '\0';
    usart_msg_ready = 1;
    usart_rx_idx    = 0;
  }
  else if (received_data != '\r') // Bỏ qua '\r' nếu sender gửi "\r\n"
  {
    if (usart_rx_idx < USART_RX_BUF_SIZE - 1)
    {
      usart_rx_buf[usart_rx_idx++] = received_data;
      usart_msg_ready = 1;
    }
  }
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
