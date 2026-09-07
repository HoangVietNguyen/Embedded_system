#include "main.h"
#include <stdlib.h>
#include "driver_i2c.h"
#include "peripheral/rcc/rcc.h"
#include "peripheral/i2c/i2c/i2c.h"
#include "sensor_manager/dht11_driver/dht11.h"
#include "peripheral/i2c/lcd_config/lcd_config.h"
#include "driver_gpio.h"
#include "driver_iwdg.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* 1. Khởi tạo chân LED Onboard PC13 (Kit STM32 BluePill) */
    driver_gpio_clock_enable(GPIOB);
    driver_gpio_init_pin(GPIOB, 3, GPIO_MODE_OUTPUT_PP_50MHZ);

    /* 2. Kiểm tra xem lần khởi động này có phải do WATCHDOG RESET cứu hay không */
    if (driver_iwdg_is_reset_caused_by_watchdog()) {
        /* NẾU VỪA BỊ WATCHDOG RESET: Nháy LED PC13 siêu nhanh 5 lần để báo hiệu */
        for (int i = 0; i < 5; i++) {
            driver_gpio_toggle_pin(GPIOC, 13);
            HAL_Delay(100);
        }
        driver_iwdg_clear_reset_flags(); // Xóa cờ báo Reset
    } else {
        /* NẾU KHỜI ĐỘNG BÌNH THƯỜNG: Sáng LED 1 lần */
        driver_gpio_write_pin(GPIOC, 13, DRIVER_GPIO_PIN_RESET); // Bật LED PC13 (Active Low)
        HAL_Delay(500);
        driver_gpio_write_pin(GPIOC, 13, DRIVER_GPIO_PIN_SET);   // Tắt LED PC13
    }

    /* 3. Khởi tạo LCD I2C Bare-Metal */
    MX_I2C1_Init();
    lcd_init();

    /* 4. Khởi tạo Watchdog IWDG với Timeout = 1.5 giây */
    // Prescaler /64 (625 Hz), Reload = 937 -> Timeout = 937 / 625 = 1.5 giây
    driver_iwdg_init(IWDG_PRESCALER_64, 937);

    uint8_t feed_count = 0;

    while (1)
    {
        if (feed_count < 5) {
            /* 5 lần đầu: "Cho chó ăn" bình thường mỗi 500ms -> Chip chạy mượt mà */
            lcd_goto_xy(0, 0);
            lcd_send_string("WDT: FEEDING... ");

            driver_iwdg_refresh(); // "Cho chó ăn" / Refresh Counter về 1.5s
            HAL_Delay(500);
            feed_count++;
        }
        else {
            /* Lần 6: GIẢ LẬP HỆ THỐNG BỊ TREO!! */
            /* Cố tình đứng chôn chân 3.0 giây mà KHÔNG gọi driver_iwdg_refresh() */
            lcd_goto_xy(0, 0);
            lcd_send_string("SYSTEM FREEZE!! ");

            HAL_Delay(3000); // Hết 1.5s -> WATCHDOG CỨNG TỰ RESET CHIP NGAY LẬP TỨC!
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