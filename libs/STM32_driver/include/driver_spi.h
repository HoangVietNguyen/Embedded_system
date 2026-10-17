#ifndef DRIVER_SPI_H
#define DRIVER_SPI_H

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Definitionen, Enums & Strukturen des SPI-Treibers                         */
/* ========================================================================= */

/**
 * @brief SPI-Betriebsmodus (Master / Slave)
 */
typedef enum {
    DRIVER_SPI_MODE_SLAVE  = 0U, /*!< Slave-Konfiguration */
    DRIVER_SPI_MODE_MASTER = 1U  /*!< Master-Konfiguration (Standard) */
} SPI_Mode_t;

/**
 * @brief SPI-Taktteiler / Baudrate (basierend auf PCLK)
 */
typedef enum {
    DRIVER_SPI_BAUDRATE_DIV2   = 0U, /*!< f_PCLK / 2 */
    DRIVER_SPI_BAUDRATE_DIV4   = 1U, /*!< f_PCLK / 4 */
    DRIVER_SPI_BAUDRATE_DIV8   = 2U, /*!< f_PCLK / 8 */
    DRIVER_SPI_BAUDRATE_DIV16  = 3U, /*!< f_PCLK / 16 */
    DRIVER_SPI_BAUDRATE_DIV32  = 4U, /*!< f_PCLK / 32 */
    DRIVER_SPI_BAUDRATE_DIV64  = 5U, /*!< f_PCLK / 64 */
    DRIVER_SPI_BAUDRATE_DIV128 = 6U, /*!< f_PCLK / 128 */
    DRIVER_SPI_BAUDRATE_DIV256 = 7U  /*!< f_PCLK / 256 */
} SPI_BaudRate_t;

/**
 * @brief SPI-Taktpolarität (CPOL - Clock Polarity)
 */
typedef enum {
    DRIVER_SPI_CPOL_LOW  = 0U, /*!< Takt ruht auf Logisch 0 (Low) */
    DRIVER_SPI_CPOL_HIGH = 1U  /*!< Takt ruht auf Logisch 1 (High) */
} SPI_CPOL_t;

/**
 * @brief SPI-Taktphase (CPHA - Clock Phase)
 */
typedef enum {
    DRIVER_SPI_CPHA_1EDGE = 0U, /*!< Datenübernahme an der 1. Taktflanke */
    DRIVER_SPI_CPHA_2EDGE = 1U  /*!< Datenübernahme an der 2. Taktflanke */
} SPI_CPHA_t;

/**
 * @brief SPI-Datenformat (8-Bit oder 16-Bit)
 */
typedef enum {
    DRIVER_SPI_DFF_8BIT  = 0U, /*!< 8-Bit Datenframe (Standard) */
    DRIVER_SPI_DFF_16BIT = 1U  /*!< 16-Bit Datenframe */
} SPI_DataFrame_t;

/**
 * @brief SPI Bit-Reihenfolge (MSB zuerst oder LSB zuerst)
 */
typedef enum {
    DRIVER_SPI_FIRSTBIT_MSB = 0U, /*!< Most Significant Bit zuerst */
    DRIVER_SPI_FIRSTBIT_LSB = 1U  /*!< Least Significant Bit zuerst */
} SPI_FirstBit_t;

/**
 * @brief Struktur zur Konfiguration des SPI-Peripheriegeräts
 */
typedef struct {
    SPI_Mode_t      mode;       /*!< Master oder Slave Modus */
    SPI_BaudRate_t  baudrate;   /*!< Baudraten-Teiler (DIV2 bis DIV256) */
    SPI_CPOL_t      cpol;       /*!< Clock Polarity (Low / High) */
    SPI_CPHA_t      cpha;       /*!< Clock Phase (1Edge / 2Edge) */
    SPI_DataFrame_t data_frame; /*!< Datenbreite (8-Bit / 16-Bit) */
    SPI_FirstBit_t  first_bit;  /*!< MSB oder LSB zuerst */
} SPI_Config_t;

/* Backward compatibility aliases */
#define SPI_MODE_MASTER         DRIVER_SPI_MODE_MASTER
#define SPI_MODE_SLAVE          DRIVER_SPI_MODE_SLAVE
#define SPI_BAUDRATE_DIV2       DRIVER_SPI_BAUDRATE_DIV2
#define SPI_BAUDRATE_DIV8       DRIVER_SPI_BAUDRATE_DIV8
#define SPI_BAUDRATE_DIV16      DRIVER_SPI_BAUDRATE_DIV16
#define SPI_BAUDRATE_DIV32      DRIVER_SPI_BAUDRATE_DIV32
#define SPI_CPOL_LOW            DRIVER_SPI_CPOL_LOW
#define SPI_CPOL_HIGH           DRIVER_SPI_CPOL_HIGH
#define SPI_CPHA_1EDGE          DRIVER_SPI_CPHA_1EDGE
#define SPI_CPHA_2EDGE          DRIVER_SPI_CPHA_2EDGE

/* ========================================================================= */
/* Funktionsprototypen der SPI-Treiberschnittstelle (Öffentliche API)         */
/* ========================================================================= */

/**
 * @brief Initialisiert das SPI-Peripheriegerät (z.B. SPI1, SPI2) mit der angegebenen Konfiguration
 * @param SPIx Zeiger auf das SPI-Register (SPI1, SPI2, SPI3)
 * @param config Zeiger auf die Konfigurationsstruktur (SPI_Config_t)
 */
void driver_spi_init(SPI_TypeDef *SPIx, const SPI_Config_t *config);

/**
 * @brief Sendet ein Byte über SPI und empfängt gleichzeitig das Antwort-Byte (Full-Duplex)
 * @param SPIx Zeiger auf das SPI-Register
 * @param tx_data Zu sendendes Byte
 * @return Empfangenes Byte vom Slave
 */
uint8_t driver_spi_transmit_receive_byte(SPI_TypeDef *SPIx, uint8_t tx_data);

/**
 * @brief Sendet einen Datenpuffer über SPI (Blocking Master Transmit)
 * @param SPIx Zeiger auf das SPI-Register
 * @param data Zeiger auf den Datenpuffer
 * @param size Anzahl der zu sendenden Bytes
 */
void driver_spi_transmit(SPI_TypeDef *SPIx, const uint8_t *data, uint16_t size);

/**
 * @brief Empfängt einen Datenpuffer über SPI durch Senden von Dummy-Bytes (0xFF)
 * @param SPIx Zeiger auf das SPI-Register
 * @param data Zeiger auf den Puffer zum Speichern der empfangenen Daten
 * @param size Anzahl der zu empfangenden Bytes
 */
void driver_spi_receive(SPI_TypeDef *SPIx, uint8_t *data, uint16_t size);

/**
 * @brief Zieht die Chip-Select-Leitung (CS/NSS) auf LOW (Aktiviert den Slave)
 * @param port GPIO-Port des CS-Pins (z.B. GPIOA)
 * @param pin Pin-Nummer des CS-Pins (z.B. 4 für PA4)
 */
void driver_spi_cs_low(GPIO_TypeDef *port, uint8_t pin);

/**
 * @brief Setzt die Chip-Select-Leitung (CS/NSS) auf HIGH (Deaktiviert den Slave)
 * @param port GPIO-Port des CS-Pins (z.B. GPIOA)
 * @param pin Pin-Nummer des CS-Pins (z.B. 4 für PA4)
 */
void driver_spi_cs_high(GPIO_TypeDef *port, uint8_t pin);

/**
 * @brief Sendet ein 16-Bit Word über SPI und empfängt gleichzeitig das Antwort-Word (Full-Duplex 16-Bit)
 * -> Gửi một Word 16-Bit qua SPI và đồng thời nhận lại Word phản hồi từ Slave (Song công 16-Bit)
 * @param SPIx Zeiger auf das SPI-Register
 * -> Con trỏ tới thanh ghi SPI
 * @param tx_data Zu sendendes 16-Bit Word
 * -> Dữ liệu 16-bit cần gửi
 * @return Empfangenes 16-Bit Word vom Slave
 * -> Giá trị 16-bit nhận được từ Slave
 */
uint16_t driver_spi_transmit_receive_16bit(SPI_TypeDef *SPIx, uint16_t tx_data);

/**
 * @brief Sendet einen 16-Bit Datenpuffer über SPI (Blocking Master Transmit 16-Bit)
 * -> Gửi một bộ đệm dữ liệu 16-Bit qua SPI (Chế độ Master gửi chặn 16-Bit)
 * @param SPIx Zeiger auf das SPI-Register
 * -> Con trỏ tới thanh ghi SPI
 * @param data Zeiger auf den 16-Bit Datenpuffer
 * -> Con trỏ tới bộ đệm dữ liệu 16-bit
 * @param size Anzahl der zu sendenden 16-Bit Worte
 * -> Số lượng từ 16-bit cần gửi
 */
void driver_spi_transmit_16bit(SPI_TypeDef *SPIx, const uint16_t *data, uint16_t size);

/**
 * @brief Empfängt einen 16-Bit Datenpuffer über SPI durch Senden von Dummy-Words (0xFFFF)
 * -> Nhận một bộ đệm dữ liệu 16-Bit qua SPI bằng cách gửi các từ Dummy (0xFFFF)
 * @param SPIx Zeiger auf das SPI-Register
 * -> Con trỏ tới thanh ghi SPI
 * @param data Zeiger auf den Puffer zum Speichern der empfangenen 16-Bit Daten
 * -> Con trỏ tới bộ đệm lưu dữ liệu 16-bit nhận được
 * @param size Anzahl der zu empfangenden 16-Bit Worte
 * -> Số lượng từ 16-bit cần nhận
 */
void driver_spi_receive_16bit(SPI_TypeDef *SPIx, uint16_t *data, uint16_t size);

/** Fixed-length SPI1 slave DMA receive. Completion callback runs in DMA1 Channel 2 IRQ. */
typedef void (*Driver_SPI_DmaRxCallback_t)(SPI_TypeDef *SPIx,
                                           const uint8_t *buffer,
                                           uint16_t length);
bool dma_spi_receive_it(SPI_TypeDef *SPIx, uint8_t *buffer, uint16_t length,
                        Driver_SPI_DmaRxCallback_t callback,
                        uint32_t preempt_priority, uint32_t sub_priority);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_SPI_H
