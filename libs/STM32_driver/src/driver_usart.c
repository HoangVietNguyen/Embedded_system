#include "driver_usart.h"
#include <string.h>
#include "driver_gpio.h"
#include "driver_dma.h"
#include "driver_interrupt.h"

/* ========================================================================= */
/* Implementierung der bare-metal USART-Treiberfunktionen                    */
/* ========================================================================= */

#define DRIVER_USART_CLOCK_ENABLE(PORT) \
do { \
if ((PORT) == USART1) { RCC->APB2ENR |= RCC_APB2ENR_USART1EN; \
    driver_gpio_clock_enable(GPIOA); \
    driver_gpio_init_pin(GPIOA, 9, GPIO_MODE_AF_PP_50MHZ);\
    /* RX ist ein Eingang, der vom TX des anderen Chips angesteuert wird; AF Push-Pull ist für TX. */\
    driver_gpio_init_pin(GPIOA, 10, GPIO_MODE_INPUT_FLOATING);\
} \
else if ((PORT) == USART2) { RCC->APB1ENR |= RCC_APB1ENR_USART2EN; \
    driver_gpio_clock_enable(GPIOA); \
    driver_gpio_init_pin(GPIOA, 2, GPIO_MODE_AF_PP_50MHZ);\
    /* RX ist ein Eingang, der vom TX des anderen Chips angesteuert wird; AF Push-Pull ist für TX. */\
    driver_gpio_init_pin(GPIOA, 3, GPIO_MODE_INPUT_FLOATING);\
} \
else if ((PORT) == USART3) { RCC->APB1ENR |= RCC_APB1ENR_USART3EN; \
    driver_gpio_clock_enable(GPIOB); \
    driver_gpio_init_pin(GPIOB, 10, GPIO_MODE_AF_PP_50MHZ);\
    /* RX ist ein Eingang, der vom TX des anderen Chips angesteuert wird; AF Push-Pull ist für TX. */\
    driver_gpio_init_pin(GPIOB, 11, GPIO_MODE_INPUT_FLOATING);\
} \
} while (0)
/* USART1 läuft auf APB2 (72 MHz), USART2 und USART3 laufen auf APB1 (36 MHz) */

static volatile USART_TypeDef* enable_usart = USART1;
static Driver_USART_RxCallback_t usart1_rx_callback;
static Driver_USART_RxCallback_t usart2_rx_callback;
static Driver_USART_RxCallback_t usart3_rx_callback;

typedef struct {
    USART_TypeDef *usart;
    uint8_t *buffer;
    uint16_t length;
    Driver_USART_DmaRxCallback_t callback;
} Driver_USART_DmaRxState_t;

static Driver_USART_DmaRxState_t usart1_dma_rx_state;
static Driver_USART_DmaRxState_t usart2_dma_rx_state;
static Driver_USART_DmaRxState_t usart3_dma_rx_state;

static IRQn_Type driver_usart_get_irq(USART_TypeDef *target) {
    if (target == USART1) return USART1_IRQn;
    if (target == USART2) return USART2_IRQn;
    if (target == USART3) return USART3_IRQn;
    return (IRQn_Type)-1;
}

bool driver_usart_set_rx_callback(USART_TypeDef *USARTx, Driver_USART_RxCallback_t callback,
                                  uint32_t preempt_priority, uint32_t sub_priority) {
    USART_TypeDef *target = (USARTx != NULL) ? USARTx : (USART_TypeDef*)enable_usart;
    IRQn_Type irq = driver_usart_get_irq(target);
    if ((int32_t)irq < 0) return false;

    if (target == USART1) usart1_rx_callback = callback;
    else if (target == USART2) usart2_rx_callback = callback;
    else usart3_rx_callback = callback;

    if (callback == NULL) {
        target->CR1 &= ~USART_CR1_RXNEIE;
        driver_interrupt_nvic_config(irq, preempt_priority, sub_priority, false);
        return true;
    }

    target->CR1 |= (USART_CR1_RE | USART_CR1_RXNEIE);
    driver_interrupt_nvic_config(irq, preempt_priority, sub_priority, true);
    return true;
}

static void driver_usart_handle_rx_irq(USART_TypeDef *target, Driver_USART_RxCallback_t callback) {
    if ((target->SR & USART_SR_RXNE) != 0U) {
        uint8_t data = (uint8_t)(target->DR & 0xFFU); /* DR read clears RXNE */
        if (callback != NULL) callback(target, data);
    }
}

void USART1_IRQHandler(void) { driver_usart_handle_rx_irq(USART1, usart1_rx_callback); }
void USART2_IRQHandler(void) { driver_usart_handle_rx_irq(USART2, usart2_rx_callback); }
void USART3_IRQHandler(void) { driver_usart_handle_rx_irq(USART3, usart3_rx_callback); }

static void driver_usart_handle_dma_rx_irq(uint8_t channel_num,
                                            Driver_USART_DmaRxState_t *state,
                                            DMA_Channel_TypeDef *channel) {
    if (!driver_dma_is_transfer_complete(DMA1, channel_num)) return;

    driver_dma_stop(channel);
    state->usart->CR3 &= ~USART_CR3_DMAR;
    driver_dma_clear_flags(DMA1, channel_num);

    Driver_USART_DmaRxCallback_t callback = state->callback;
    if (callback != NULL) callback(state->usart, state->buffer, state->length);
}

void DMA1_Channel3_IRQHandler(void) {
    driver_usart_handle_dma_rx_irq(3U, &usart3_dma_rx_state, DMA1_Channel3);
}
void DMA1_Channel5_IRQHandler(void) {
    driver_usart_handle_dma_rx_irq(5U, &usart1_dma_rx_state, DMA1_Channel5);
}
void DMA1_Channel6_IRQHandler(void) {
    driver_usart_handle_dma_rx_irq(6U, &usart2_dma_rx_state, DMA1_Channel6);
}

/**
 * @brief Aktiviert den Peripherietakt für das USART-Modul und AFIO im RCC
 */
void driver_usart_clock_enable(USART_TypeDef *USARTx) {
    DRIVER_USART_CLOCK_ENABLE(USARTx);
    enable_usart = USARTx;
}

/**
 * @brief Initialisiert das USART-Modul vollständig mit den Werten der Konfigurationsstruktur
 */
void driver_usart_init(USART_TypeDef *USARTx, const USART_Config_t *config) {
    USART_TypeDef *target = (USARTx != NULL) ? USARTx : (USART_TypeDef*)enable_usart;

    if (target == NULL || config == NULL || config->baudrate == 0U) {
        return;
    }

    /* 1. Baudraten-Berechnung basierend auf dem Bus-Takt (72 MHz für APB2 / 36 MHz für APB1) */
    /* Formel RM0008: USARTDIV = PCLK / (16 * Baudrate) -> BRR_Wert = (PCLK + Baudrate/2) / Baudrate */
    uint32_t pclk = (target == USART1) ? 72000000U : 36000000U;
    uint32_t brr_val = (pclk + (config->baudrate / 2U)) / config->baudrate;
    target->BRR = brr_val;

    /* 2. CR1-Register konfigurieren (Wortlänge, Parität, Betriebsmodus) */
    uint32_t tmpreg_cr1 = target->CR1;
    tmpreg_cr1 &= ~(USART_CR1_M | USART_CR1_PCE | USART_CR1_PS | USART_CR1_TE | USART_CR1_RE);

    /* Wortlänge (M-Bit: 8-Bit oder 9-Bit) */
    if (config->word_length == DRIVER_USART_WORDLENGTH_9B) {
        tmpreg_cr1 |= USART_CR1_M;
    }

    /* Paritätsprüfung (PCE & PS) */
    if (config->parity == DRIVER_USART_PARITY_EVEN) {
        tmpreg_cr1 |= USART_CR1_PCE;
    } else if (config->parity == DRIVER_USART_PARITY_ODD) {
        tmpreg_cr1 |= (USART_CR1_PCE | USART_CR1_PS);
    }

    /* Betriebsmodus (TE: Transmitter Enable, RE: Receiver Enable) */
    if (config->mode & DRIVER_USART_MODE_RX) {
        tmpreg_cr1 |= USART_CR1_RE;
    }
    if (config->mode & DRIVER_USART_MODE_TX) {
        tmpreg_cr1 |= USART_CR1_TE;
    }

    /* USART-Peripherie aktivieren (UE: USART Enable) */
    tmpreg_cr1 |= USART_CR1_UE;
    target->CR1 = tmpreg_cr1;

    /* 3. CR2-Register konfigurieren (Stopp-Bits: STOP[1:0]) */
    uint32_t tmpreg_cr2 = target->CR2;
    tmpreg_cr2 &= ~USART_CR2_STOP;
    tmpreg_cr2 |= ((uint32_t)config->stop_bits << USART_CR2_STOP_Pos);
    target->CR2 = tmpreg_cr2;

    /* 4. CR3-Register konfigurieren (Hardware-Flusssteuerung: RTSE & CTSE) */
    uint32_t tmpreg_cr3 = target->CR3;
    tmpreg_cr3 &= ~(USART_CR3_RTSE | USART_CR3_CTSE);
    if (config->hw_flow_control & DRIVER_USART_HWCONTROL_RTS) {
        tmpreg_cr3 |= USART_CR3_RTSE;
    }
    if (config->hw_flow_control & DRIVER_USART_HWCONTROL_CTS) {
        tmpreg_cr3 |= USART_CR3_CTSE;
    }
    target->CR3 = tmpreg_cr3;
}

/**
 * @brief Vereinfachte Initialisierung des USART-Moduls mit Standardparametern (8N1, TX_RX)
 */
void driver_usart_init_simple(USART_TypeDef *USARTx, uint32_t baudrate) {
    driver_usart_clock_enable(USARTx);

    USART_Config_t config = {
        .baudrate        = baudrate,
        .word_length     = DRIVER_USART_WORDLENGTH_8B,
        .stop_bits       = DRIVER_USART_STOPBITS_1,
        .parity          = DRIVER_USART_PARITY_NONE,
        .mode            = DRIVER_USART_MODE_TX_RX,
        .hw_flow_control = DRIVER_USART_HWCONTROL_NONE
    };

    driver_usart_init(USARTx, &config);
}

/**
 * @brief Sendet ein einzelnes Zeichen über USART
 * RM0008: "Bit 7 TXE: Transmit data register empty. 1: Data is transferred to the shift register"
 */
void driver_usart_send_char(USART_TypeDef *USARTx, char ch) {
    USART_TypeDef *target = (USARTx != NULL) ? USARTx : (USART_TypeDef*)enable_usart;

    /* Warten, bis das Sendedatenregister leer ist (TXE == 1) */
    while (!(target->SR & USART_SR_TXE)) {}

    /* Vor dem Schreiben in das DR-Register wird das Zeichen mit 0x00FFU maskiert,
     * um die reservierten Bits des 32-Bit-Registers zu schützen. */
    target->DR = ((uint32_t)ch & 0x00FFU);
}

/**
 * @brief Sendet eine nullterminierte Zeichenkette über USART
 * RM0008: "Bit 6 TC: Transmission complete. 1: Transmission is complete"
 */
void driver_usart_send_string(USART_TypeDef *USARTx, const char *str) {
    USART_TypeDef *target = (USARTx != NULL) ? USARTx : (USART_TypeDef*)enable_usart;

    if (str == NULL) {
        return;
    }

    /* Die Zeichen der Zeichenkette nacheinander durchlaufen und senden */
    while (*str != '\0') {
        driver_usart_send_char(target, *str);
        str++;
    }

    /* Warten, bis die Übertragung vollständig abgeschlossen ist (TC == 1) */
    while (!(target->SR & USART_SR_TC)) {}
}

/**
 * @brief Erstellt die DMA-Konfigurationsstruktur für die USART TX-Übertragung
 */
DMA_Config_t create_config_dma_usart_tx(const char *str) {
    DMA_Config_t dma_config = {
        .periph_base_addr = (uint32_t)&(enable_usart->DR),
        .mem_base_addr    = (uint32_t)str,
        .data_length      = (uint16_t)strlen(str),
        .direction        = DRIVER_DMA_DIR_MEM_TO_PERIPH,
        .periph_data_size = DRIVER_DMA_DATA_SIZE_8BIT,
        .mem_data_size    = DRIVER_DMA_DATA_SIZE_8BIT,
        .periph_inc_enable= false,
        .mem_inc_enable   = true,
        .priority         = DRIVER_DMA_PRIORITY_HIGH,
        .mode             = DRIVER_DMA_MODE_NORMAL
    };

    return dma_config;
}

/**
 * @brief Sendet eine Zeichenkette über USART mittels DMA inklusive Bereinigung
 * RM0008: "Bit 7 DMAT: DMA enable transmitter. 1: DMA mode enabled for transmission."
 */
void dma_usart_send_string(const char *str) {
    if (str == NULL || *str == '\0') {
        return;
    }

    DMA_Config_t dma_config = create_config_dma_usart_tx(str);
    driver_dma_clock_enable(DMA1);

    DMA_Channel_TypeDef *dma_channel = NULL;
    uint8_t channel_num = 0;

    /* DMA-Kanal basierend auf der USART-Instanz zuordnen (RM0008 Tabelle 78) */
    if (enable_usart == USART1) {
        dma_channel = DMA1_Channel4;
        channel_num = 4;
    } else if (enable_usart == USART2) {
        dma_channel = DMA1_Channel7;
        channel_num = 7;
    } else if (enable_usart == USART3) {
        dma_channel = DMA1_Channel2;
        channel_num = 2;
    } else {
        return;
    }

    /* 1. DMA-Kanal mit der Konfigurationsstruktur initialisieren */
    driver_dma_init(dma_channel, &dma_config);

    /* 2. Vorheriges Übertragungsabschluss-Flag (TC) löschen und DMAT-Bit im USART_CR3 aktivieren */
    enable_usart->SR &= ~USART_SR_TC;
    enable_usart->CR3 |= USART_CR3_DMAT;

    /* 3. DMA-Kanal starten */
    driver_dma_start(dma_channel);

    /* 4. Warten, bis die DMA-Übertragung abgeschlossen ist (TCIF-Flag im DMA_ISR) */
    while (!driver_dma_is_transfer_complete(DMA1, channel_num)) {}

    /* 5. Warten, bis das letzte Byte das USART-Schieberegister verlassen hat (TC-Flag im USART_SR) */
    while (!(enable_usart->SR & USART_SR_TC)) {}

    /* 6. Hardware-Bereinigung: DMA-Kanal stoppen, DMAT-Bit deaktivieren und DMA-Flags löschen */
    driver_dma_stop(dma_channel);
    enable_usart->CR3 &= ~USART_CR3_DMAT;
    driver_dma_clear_flags(DMA1, channel_num);
}

/**
 * @brief Liest ein einzelnes empfangenes Zeichen von der USART-Schnittstelle
 * RM0008: "Bit 5 RXNE: Read data register not empty. 1: Received data is ready to be read."
 */
char driver_usart_receive_char(USART_TypeDef *USARTx) {
    USART_TypeDef *target = (USARTx != NULL) ? USARTx : (USART_TypeDef*)enable_usart;

    /* Warten, bis Empfangsdaten bereitstehen (RXNE == 1) */
    while (!(target->SR & USART_SR_RXNE)) {}

    /* Das Lesen des DR-Registers setzt das RXNE-Flag automatisch zurück */
    return (char)(target->DR & 0x00FFU);
}

/**
 * @brief Erstellt die DMA-Konfigurationsstruktur für den empfangsseitigen USART-Modus (RX DMA)
 */
DMA_Config_t create_config_dma_usart_rx(USART_TypeDef *USARTx, uint8_t *buffer, uint16_t length) {
    USART_TypeDef *target = (USARTx != NULL) ? USARTx : (USART_TypeDef*)enable_usart;

    DMA_Config_t dma_config = {
        .periph_base_addr = (uint32_t)&(target->DR),
        .mem_base_addr    = (uint32_t)buffer,
        .data_length      = length,
        .direction        = DRIVER_DMA_DIR_PERIPH_TO_MEM,
        .periph_data_size = DRIVER_DMA_DATA_SIZE_8BIT,
        .mem_data_size    = DRIVER_DMA_DATA_SIZE_8BIT,
        .periph_inc_enable= false,
        .mem_inc_enable   = true,
        .priority         = DRIVER_DMA_PRIORITY_VERY_HIGH,
        .mode             = DRIVER_DMA_MODE_CIRCULAR
    };

    return dma_config;
}

/**
 * @brief Empfängt Daten über USART mittels DMA im zirkulären Modus (RX DMA)
 * RM0008: "Bit 6 DMAR: DMA enable receiver. 1: DMA mode is enabled for reception."
 * RM0008 Table 78: USART1_RX -> DMA1_Channel5, USART2_RX -> DMA1_Channel6, USART3_RX -> DMA1_Channel3
 */
bool dma_usart_receive_string(USART_TypeDef *USARTx, uint8_t *buffer, uint16_t length) {
    USART_TypeDef *target = (USARTx != NULL) ? USARTx : (USART_TypeDef*)enable_usart;

    if (buffer == NULL || length == 0U) {
        return false;
    }

    DMA_Config_t dma_config = create_config_dma_usart_rx(target, buffer, length);
    driver_dma_clock_enable(DMA1);

    DMA_Channel_TypeDef *dma_channel = NULL;

    /* DMA-Kanal basierend auf der USART-Instanz zuordnen (RM0008 Tabelle 78) */
    if (target == USART1) {
        dma_channel = DMA1_Channel5;
    } else if (target == USART2) {
        dma_channel = DMA1_Channel6;
    } else if (target == USART3) {
        dma_channel = DMA1_Channel3;
    } else {
        return false;
    }

    /* 1. DMA-Kanal mit der Konfigurationsstruktur initialisieren */
    driver_dma_init(dma_channel, &dma_config);

    /* 2. DMAR-Bit (DMA Enable Receiver) im USART_CR3 aktivieren */
    target->CR3 |= USART_CR3_DMAR;

    /* 3. DMA-Kanal für den kontinuierlichen Empfang im Hintergrund starten */
    driver_dma_start(dma_channel);

    return true;
}

bool dma_usart_receive_it(USART_TypeDef *USARTx, uint8_t *buffer, uint16_t length,
                          Driver_USART_DmaRxCallback_t callback,
                          uint32_t preempt_priority, uint32_t sub_priority) {
    USART_TypeDef *target = (USARTx != NULL) ? USARTx : (USART_TypeDef*)enable_usart;
    if (target == NULL || buffer == NULL || length == 0U || callback == NULL) return false;

    DMA_Channel_TypeDef *channel;
    uint8_t channel_num;
    IRQn_Type irq;
    Driver_USART_DmaRxState_t *state;
    if (target == USART1) {
        channel = DMA1_Channel5; channel_num = 5U; irq = DMA1_Channel5_IRQn;
        state = &usart1_dma_rx_state;
    } else if (target == USART2) {
        channel = DMA1_Channel6; channel_num = 6U; irq = DMA1_Channel6_IRQn;
        state = &usart2_dma_rx_state;
    } else if (target == USART3) {
        channel = DMA1_Channel3; channel_num = 3U; irq = DMA1_Channel3_IRQn;
        state = &usart3_dma_rx_state;
    } else return false;

    /* Fixed-length one-shot transfer: callback fires after exactly length bytes. */
    DMA_Config_t config = create_config_dma_usart_rx(target, buffer, length);
    config.mode = DRIVER_DMA_MODE_NORMAL;
    driver_dma_clock_enable(DMA1);
    driver_dma_stop(channel);
    driver_dma_clear_flags(DMA1, channel_num);
    driver_dma_init(channel, &config);
    state->usart = target;
    state->buffer = buffer;
    state->length = length;
    state->callback = callback;

    /* TCIE enables the DMA transfer-complete interrupt; NVIC routes it to the ISR. */
    channel->CCR |= DMA_CCR_TCIE;
    driver_interrupt_nvic_config(irq, preempt_priority, sub_priority, true);
    /* DMA owns DR during this transfer; avoid a competing byte-wise RXNE ISR. */
    target->CR1 &= ~USART_CR1_RXNEIE;
    target->CR3 |= USART_CR3_DMAR;
    driver_dma_start(channel);
    return true;
}
