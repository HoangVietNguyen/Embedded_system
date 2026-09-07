#include "i2c.h"
#include "stm32f1xx_hal_i2c.h"
#include "driver_i2c.h"

void MX_I2C1_Init(void)
{
    init_i2c_modul(I2C1, I2C_SPEED_FAST_400KHz);
}

void transmit_data_I2C() {

}
