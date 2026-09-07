#ifndef DRIVER_I2C_H
#define DRIVER_I2C_H

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Definitionen für Enums, Strukturen & Makros des I2C-Treibers             */
/* ========================================================================= */

/**
 * @brief Enums für I2C-Busgeschwindigkeiten (Standard 100kHz / Fast 400kHz)
 */
typedef enum {
    I2C_SPEED_STANDARD_100KHz = 100000,
    I2C_SPEED_FAST_400KHz = 400000
} I2C_Speed_t;

/**
 * @brief Enums für I2C-Slave-Adressierungsmodi (Einzel- oder Dual-Adresse)
 */
typedef enum {
    I2C_DUAL_ADDRESS = 0,
    I2C_SINGLE_ADDRESS = 1
} I2C_Address_Mode;

/* ========================================================================= */
/* Funktionsprototypen der I2C-Treiberschnittstelle (Öffentliche API)       */
/* ========================================================================= */

/**
 * @brief Initialisiert das I2C-Modul und konfiguriert die Busgeschwindigkeit
 * @param i2c Zeiger auf das I2C-Peripheriegerät (z.B. I2C1, I2C2)
 * @param speed Gewünschte Busgeschwindigkeit (100kHz oder 400kHz)
 */
    void init_i2c_modul(I2C_TypeDef *i2c, I2C_Speed_t speed);

    void init_gpio_i2c(uint8_t scl_gpio);
/**
 * @brief Konfiguriert die eigenen Slave-Adressen der MCU (Falls als Slave betrieben)
 * @param i2c Zeiger auf das I2C-Peripheriegerät
 * @param mode Adressierungsmodus (Single oder Dual Address)
 * @param own_addr1 Primäre Slave-Adresse (7-Bit)
 * @param own_addr2 Sekundäre Slave-Adresse (7-Bit, optional bei Dual Address)
 */
    void set_owner_address(I2C_TypeDef *i2c, I2C_Address_Mode mode, uint8_t own_addr1, uint8_t own_addr2);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_I2C_H