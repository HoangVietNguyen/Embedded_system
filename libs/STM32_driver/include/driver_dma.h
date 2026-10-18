#ifndef DRIVER_DMA_H
#define DRIVER_DMA_H

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Definitionen für Enums, Strukturen & Makros des DMA-Treibers             */
/* ========================================================================= */

/**
 * @brief DMA Übertragungsrichtung (Direction)
 */
typedef enum {
    DRIVER_DMA_DIR_PERIPH_TO_MEM = 0U, /*!< Lesen von Peripherie in den RAM (DIR = 0) */
    DRIVER_DMA_DIR_MEM_TO_PERIPH = 1U, /*!< Lesen vom RAM in die Peripherie (DIR = 1) */
    DRIVER_DMA_DIR_MEM_TO_MEM    = 2U  /*!< Lesen vom RAM in einen anderen RAM-Bereich (MEM2MEM = 1) */
} DMA_Direction_t;

/**
 * @brief DMA Datengröße für Peripherie und Speicher (Data Size)
 */
typedef enum {
    DRIVER_DMA_DATA_SIZE_8BIT  = 0U, /*!< 8-Bit (Byte) */
    DRIVER_DMA_DATA_SIZE_16BIT = 1U, /*!< 16-Bit (Half-Word) */
    DRIVER_DMA_DATA_SIZE_32BIT = 2U  /*!< 32-Bit (Word) */
} DMA_DataSize_t;

/**
 * @brief DMA Prioritätsstufe der Kanäle (Priority Level)
 */
typedef enum {
    DRIVER_DMA_PRIORITY_LOW       = 0U, /*!< Niedrig (00) */
    DRIVER_DMA_PRIORITY_MEDIUM    = 1U, /*!< Mittel (01) */
    DRIVER_DMA_PRIORITY_HIGH      = 2U, /*!< Hoch (10) */
    DRIVER_DMA_PRIORITY_VERY_HIGH = 3U  /*!< Sehr hoch (11) */
} DMA_Priority_t;

/**
 * @brief DMA Betriebsmodus (Mode: Normal oder Zirkulär)
 */
typedef enum {
    DRIVER_DMA_MODE_NORMAL   = 0U, /*!< Normaler Modus (Einmalige Übertragung - Stoppt nach Fertigstellung) */
    DRIVER_DMA_MODE_CIRCULAR = 1U  /*!< Zirkulärer Modus (Circular Mode - Automatisches Neuladen von CNDTR) */
} DMA_Mode_t;

/* Backward compatibility aliases */
#define DMA_DIR_PERIPH_TO_MEM DRIVER_DMA_DIR_PERIPH_TO_MEM
#define DMA_DIR_MEM_TO_PERIPH DRIVER_DMA_DIR_MEM_TO_PERIPH
#define DMA_DIR_MEM_TO_MEM    DRIVER_DMA_DIR_MEM_TO_MEM

#define DMA_DATA_SIZE_8BIT  DRIVER_DMA_DATA_SIZE_8BIT
#define DMA_DATA_SIZE_16BIT DRIVER_DMA_DATA_SIZE_16BIT
#define DMA_DATA_SIZE_32BIT DRIVER_DMA_DATA_SIZE_32BIT

#define DMA_PRIORITY_LOW       DRIVER_DMA_PRIORITY_LOW
#define DMA_PRIORITY_MEDIUM    DRIVER_DMA_PRIORITY_MEDIUM
#define DMA_PRIORITY_HIGH      DRIVER_DMA_PRIORITY_HIGH
#define DMA_PRIORITY_VERY_HIGH DRIVER_DMA_PRIORITY_VERY_HIGH

#define DMA_MODE_NORMAL   DRIVER_DMA_MODE_NORMAL
#define DMA_MODE_CIRCULAR DRIVER_DMA_MODE_CIRCULAR

/**
 * @brief Struktur zur vollständigen Konfiguration eines DMA-Kanals
 */
typedef struct {
    uint32_t         periph_base_addr; /*!< Peripherie-Hardwareadresse (CPAR) */
    uint32_t         mem_base_addr;    /*!< RAM-Speicheradresse (CMAR) */
    uint16_t         data_length;      /*!< Anzahl der zu übertragenden Datenelemente (CNDTR) */
    DMA_Direction_t  direction;        /*!< Übertragungsrichtung (Peripheral to Mem, Mem to Peripheral, Mem to Mem) */
    DMA_DataSize_t   periph_data_size; /*!< Peripherie-Datengröße (8-, 16- oder 32-Bit) */
    DMA_DataSize_t   mem_data_size;    /*!< RAM-Speicher-Datengröße (8-, 16- oder 32-Bit) */
    bool             periph_inc_enable;/*!< Automatische Adressinkrementierung der Peripherie (PINC) */
    bool             mem_inc_enable;   /*!< Automatische Adressinkrementierung des RAM (MINC) */
    DMA_Priority_t   priority;         /*!< Prioritätsstufe des Kanals (Low, Medium, High, Very High) */
    DMA_Mode_t       mode;             /*!< Betriebsmodus (Normal / Circular) */
} DMA_Config_t;

/* ========================================================================= */
/* Funktionsprototypen der DMA-Treiberschnittstelle (Öffentliche API)       */
/* ========================================================================= */

/**
 * @brief Aktiviert den Peripherietakt für den DMA-Controller (DMA1 oder DMA2)
 * @param dma Zeiger auf den DMA-Controller (DMA1 oder DMA2)
 */
void driver_dma_clock_enable(DMA_TypeDef *dma);

/**
 * @brief Initialisiert einen bestimmten DMA-Kanal
 * @param channel Zeiger auf den DMA-Kanal (z.B. DMA1_Channel1)
 * @param config Zeiger auf die Konfigurationsstruktur (DMA_Config_t)
 */
void driver_dma_init(DMA_Channel_TypeDef *channel, const DMA_Config_t *config);

/**
 * @brief Startet die Datenübertragung auf dem gewählten DMA-Kanal (EN-Bit setzen)
 * @param channel Zeiger auf den DMA-Kanal (z.B. DMA1_Channel1)
 */
void driver_dma_start(DMA_Channel_TypeDef *channel);

/**
 * @brief Stoppt die Datenübertragung auf dem gewählten DMA-Kanal (EN-Bit löschen)
 * @param channel Zeiger auf den DMA-Kanal (z.B. DMA1_Channel1)
 */
void driver_dma_stop(DMA_Channel_TypeDef *channel);

/**
 * @brief Prüft, ob die Datenübertragung auf dem gewählten Kanal abgeschlossen ist (TCIF-Flag)
 * @param dma Zeiger auf den DMA-Controller (DMA1 oder DMA2)
 * @param channel_num Kanalnummer (1 bis 7)
 * @return true if Transfer Complete, false otherwise
 */
bool driver_dma_is_transfer_complete(DMA_TypeDef *dma, uint8_t channel_num);

/**
 * @brief Löscht die Interrupt-Flags des gewählten DMA-Kanals im IFCR-Register
 * @param dma Zeiger auf den DMA-Controller (DMA1 oder DMA2)
 * @param channel_num Kanalnummer (1 bis 7)
 * @return true if successful
 */
bool driver_dma_clear_flags(DMA_TypeDef *dma, uint8_t channel_num);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_DMA_H
