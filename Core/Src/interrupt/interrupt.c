#include "interrupt.h"
#include "tetris_game/generate_piece_driver/pieces.h"

// hàm click button, khối hình dịch sang trái hoặc phảo
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN) {
  if (GPIO_PIN == BUTTON_SHIFT_LEFT_Pin) {
    MAX7219_Shift_Left_Pieces();
  }

  if (GPIO_PIN == BUTTON_SHIFT_RIGHT_Pin) {
    MAX7219_Shift_Right_Pieces();
  }
}

//hàm interrupt bắt sự kiện đếm hết 500ms, tạo ra một hình khối
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
  {
    MAX7219_Generate_Pieces();
  }
}

//interrupt mỗi 500ms, dịch khối hình xuống một nấc
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM1) {
    MAX7219_Shift_Down_Pieces();
  }
}
