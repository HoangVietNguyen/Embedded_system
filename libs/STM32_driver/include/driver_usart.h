#ifndef DRIVER_USART_H
#define DRIVER_USART_H

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "driver_dma.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Definitionen für Enums, Strukturen & Makros des USART-Treibers           */
/* ========================================================================= */

/**
 * @brief Wortlänge der USART-Datenübertragung (Word Length: M-Bit im CR1)
 */
typedef enum {
    DRIVER_USART_WORDLENGTH_8B = 0U, /*!< 8 Datenbits (M = 0) */
    DRIVER_USART_WORDLENGTH_9B = 1U  /*!< 9 Datenbits (M = 1) */
} USART_WordLength_t;

/**
 * @brief Anzahl der Stopp-Bits (Stop Bits: STOP[1:0] im CR2)
 */
typedef enum {
    DRIVER_USART_STOPBITS_1   = 0U, /*!< 1 Stopp-Bit (00) */
    DRIVER_USART_STOPBITS_0_5 = 1U, /*!< 0.5 Stopp-Bit (01) */
    DRIVER_USART_STOPBITS_2   = 2U, /*!< 2 Stopp-Bits (10) */
    DRIVER_USART_STOPBITS_1_5 = 3U  /*!< 1.5 Stopp-Bits (11) */
} USART_StopBits_t;

/**
 * @brief Paritätsprüfung (Parity Control: PCE & PS im CR1)
 */
typedef enum {
    DRIVER_USART_PARITY_NONE = 0U, /*!< Keine Paritätsprüfung (PCE = 0) */
    DRIVER_USART_PARITY_EVEN = 1U, /*!< Gerade Parität (PCE = 1, PS = 0) */
    DRIVER_USART_PARITY_ODD  = 2U  /*!< Ungerade Parität (PCE = 1, PS = 1) */
} USART_Parity_t;

/**
 * @brief Betriebsmodus der USART-Schnittstelle (Mode: TE & RE im CR1)
 */
typedef enum {
    DRIVER_USART_MODE_RX    = 1U, /*!< Nur Empfänger aktivieren (RE = 1) */
    DRIVER_USART_MODE_TX    = 2U, /*!< Nur Sender aktivieren (TE = 1) */
    DRIVER_USART_MODE_TX_RX = 3U  /*!< Sender und Empfänger aktivieren (TE = 1, RE = 1) */
} USART_Mode_t;

/**
 * @brief Hardware-Flusssteuerung (Hardware Flow Control: RTSE & CTSE im CR3)
 */
typedef enum {
    DRIVER_USART_HWCONTROL_NONE    = 0U, /*!< Keine Flusssteuerung */
    DRIVER_USART_HWCONTROL_RTS     = 1U, /*!< RTS-Flusssteuerung (RTSE = 1) */
    DRIVER_USART_HWCONTROL_CTS     = 2U, /*!< CTS-Flusssteuerung (CTSE = 1) */
    DRIVER_USART_HWCONTROL_RTS_CTS = 3U  /*!< RTS und CTS Flusssteuerung (RTSE = 1, CTSE = 1) */
} USART_HardwareFlowControl_t;

/* Backward compatibility aliases */
#define USART_WORDLENGTH_8B DRIVER_USART_WORDLENGTH_8B
#define USART_WORDLENGTH_9B DRIVER_USART_WORDLENGTH_9B

#define USART_STOPBITS_1   DRIVER_USART_STOPBITS_1
#define USART_STOPBITS_0_5 DRIVER_USART_STOPBITS_0_5
#define USART_STOPBITS_2   DRIVER_USART_STOPBITS_2
#define USART_STOPBITS_1_5 DRIVER_USART_STOPBITS_1_5

#define USART_PARITY_NONE DRIVER_USART_PARITY_NONE
#define USART_PARITY_EVEN DRIVER_USART_PARITY_EVEN
#define USART_PARITY_ODD  DRIVER_USART_PARITY_ODD

#define USART_MODE_RX    DRIVER_USART_MODE_RX
#define USART_MODE_TX    DRIVER_USART_MODE_TX
#define USART_MODE_TX_RX DRIVER_USART_MODE_TX_RX

#define USART_HWCONTROL_NONE    DRIVER_USART_HWCONTROL_NONE
#define USART_HWCONTROL_RTS     DRIVER_USART_HWCONTROL_RTS
#define USART_HWCONTROL_CTS     DRIVER_USART_HWCONTROL_CTS
#define USART_HWCONTROL_RTS_CTS DRIVER_USART_HWCONTROL_RTS_CTS

/**
 * @brief Struktur zur vollständigen Konfiguration des USART-Moduls
 */
typedef struct {
    uint32_t                    baudrate;        /*!< Baudrate in bps (z.B. 9600, 115200) */
    USART_WordLength_t          word_length;     /*!< Wortlänge (8-Bit oder 9-Bit) */
    USART_StopBits_t            stop_bits;       /*!< Anzahl der Stopp-Bits (1, 0.5, 2 oder 1.5) */
    USART_Parity_t              parity;          /*!< Paritätsprüfung (Keine, Gerade, Ungerade) */
    USART_Mode_t                mode;            /*!< Betriebsmodus (TX, RX oder TX_RX) */
    USART_HardwareFlowControl_t hw_flow_control; /*!< Hardware-Flusssteuerung (Keine, RTS, CTS, RTS_CTS) */
} USART_Config_t;

/* ========================================================================= */
/* Funktionsprototypen der USART-Treiberschnittstelle (Öffentliche API)      */
/* ========================================================================= */

void driver_usart_clock_enable(USART_TypeDef *USARTx);
void driver_usart_init(USART_TypeDef *USARTx, const USART_Config_t *config);
void driver_usart_init_simple(USART_TypeDef *USARTx, uint32_t baudrate);

void driver_usart_send_char(USART_TypeDef *USARTx, char ch);
void driver_usart_send_string(USART_TypeDef *USARTx, const char *str);
char driver_usart_receive_char(USART_TypeDef *USARTx);

/** Register a byte-received callback and enable RXNE interrupts for that USART.
 * The callback runs inside the USART IRQ handler; keep it short. */
typedef void (*Driver_USART_RxCallback_t)(USART_TypeDef *USARTx, uint8_t data);
bool driver_usart_set_rx_callback(USART_TypeDef *USARTx, Driver_USART_RxCallback_t callback,
                                 uint32_t preempt_priority, uint32_t sub_priority);

DMA_Config_t create_config_dma_usart_tx(const char *str);
void dma_usart_send_string(const char *str);

DMA_Config_t create_config_dma_usart_rx(USART_TypeDef *USARTx, uint8_t *buffer, uint16_t length);
bool dma_usart_receive_string(USART_TypeDef *USARTx, uint8_t *buffer, uint16_t length);

/** Fixed-length DMA receive. Completion callback runs in the DMA IRQ context. */
typedef void (*Driver_USART_DmaRxCallback_t)(USART_TypeDef *USARTx,
                                              const uint8_t *buffer,
                                              uint16_t length);
bool dma_usart_receive_it(USART_TypeDef *USARTx, uint8_t *buffer, uint16_t length,
                          Driver_USART_DmaRxCallback_t callback,
                          uint32_t preempt_priority, uint32_t sub_priority);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_USART_H
