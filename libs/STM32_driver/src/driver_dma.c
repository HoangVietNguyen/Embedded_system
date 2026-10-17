#include "driver_dma.h"

/**
 * @brief Aktiviert den Peripherietakt für den DMA-Controller (DMA1 oder DMA2)
 * @param channel Zeiger auf den DMA-Kanal (z.B. DMA1_Channel1)
 */

#define DMA_CHANNEL(PORT) \
    do { \
        if((PORT) == DMA1) { RCC->AHBENR |= RCC_AHBENR_DMA1EN; } \
        else if((PORT) == DMA2) { RCC->AHBENR |= RCC_AHBENR_DMA2EN; } \
    } while (0)

void driver_dma_clock_enable(DMA_TypeDef *channel) {
    DMA_CHANNEL(channel);
}

/**
 * @brief Initialisiert einen bestimmten DMA-Kanal
 * @param channel Zeiger auf den DMA-Kanal (z.B. DMA1_Channel1)
 * @param config Zeiger auf die Konfigurationsstruktur (DMA_Config_t)
 */
void driver_dma_init(DMA_Channel_TypeDef *channel, const DMA_Config_t *config) {
    if (channel == NULL || config == NULL) {
        return;
    }
    // Der DMA-Kanal muss deaktiviert werden, bevor Werte in die Register CNDTR, CPAR und CMAR geschrieben werden
    // Page 287/1136 DMA_CCRx
    channel->CCR &= ~(DMA_CCR_EN | DMA_CCR_DIR | DMA_CCR_MEM2MEM |
                      DMA_CCR_PL | DMA_CCR_MSIZE | DMA_CCR_PSIZE |
                      DMA_CCR_MINC | DMA_CCR_PINC | DMA_CCR_CIRC |
                      DMA_CCR_HTIE | DMA_CCR_TCIE | DMA_CCR_TEIE);
    channel->CPAR = config->periph_base_addr;
    channel->CMAR = config->mem_base_addr;
    channel->CNDTR = config->data_length;

    // Konfiguration der Bits im Steuerregister CCR
    // Bit 14, MEM2MEM
    if (config->direction == DRIVER_DMA_DIR_MEM_TO_PERIPH) {
        channel->CCR |= DMA_CCR_DIR;       /* DIR = 1 */
    } else if (config->direction == DRIVER_DMA_DIR_MEM_TO_MEM) {
        channel->CCR |= DMA_CCR_MEM2MEM;   /* MEM2MEM = 1 */
    }

    // Bit 13 und Bit 12 (Priorität / Priority)
    channel->CCR &= ~(3U << 12);
    channel->CCR |= (config->priority << 12);

    // Bit 11 und Bit 10 (RAM-Datengröße / Memory Data Size)
    channel->CCR &= ~(3U << 10);
    channel->CCR |= (config->mem_data_size << 10);

    // Bit 9 und Bit 8 (Peripherie-Datengröße / Peripheral Data Size)
    channel->CCR &= ~(3U << 8);
    channel->CCR |= (config->periph_data_size << 8);

    // Bit 7, 6, 5 (MINC, PINC, CIRC)
    if (config->mem_inc_enable) {
        channel->CCR |= DMA_CCR_MINC;
    }
    if (config->periph_inc_enable) {
        channel->CCR |= DMA_CCR_PINC;
    }
    if (config->mode == DRIVER_DMA_MODE_CIRCULAR) {
        channel->CCR |= DMA_CCR_CIRC;
    }
}

/**
 * @brief Startet die Datenübertragung auf dem gewählten DMA-Kanal
 * @param channel Zeiger auf den DMA-Kanal (z.B. DMA1_Channel1)
 */
void driver_dma_start(DMA_Channel_TypeDef *channel) {
    if (channel != NULL) {
        channel->CCR |= DMA_CCR_EN;
    }
}

/**
 * @brief Stoppt die Datenübertragung auf dem gewählten DMA-Kanal
 * @param channel Zeiger auf den DMA-Kanal (z.B. DMA1_Channel1)
 */
void driver_dma_stop(DMA_Channel_TypeDef *channel) {
    if (channel != NULL) {
        channel->CCR &= ~DMA_CCR_EN;
    }
}

/**
 * @brief Prüft, ob die Datenübertragung auf dem gewählten Kanal abgeschlossen ist
 * @param type_def Zeiger auf den DMA-Controller (DMA1 oder DMA2)
 * @param channel_num Kanalnummer (1 bis 7)
 * @return true if Transfer Complete, false otherwise
 */
bool driver_dma_is_transfer_complete(DMA_TypeDef *type_def, uint8_t channel_num) {
    if (type_def == NULL || channel_num < 1 || channel_num > 7) {
        return false;
    }
    uint32_t shift_bit = (channel_num - 1) * 4 + 1;
    return ((type_def->ISR & (1U << shift_bit)) != 0);
}

/**
 * @brief Löscht die Interrupt-Flags des gewählten DMA-Kanals im IFCR-Register
 * @param type_def Zeiger auf den DMA-Controller (DMA1 oder DMA2)
 * @param channel_num Kanalnummer (1 bis 7)
 * @return true if successful
 */
bool driver_dma_clear_flags(DMA_TypeDef *type_def, uint8_t channel_num) {
    if (type_def == NULL || channel_num < 1 || channel_num > 7) {
        return false;
    }

    /* W1C: Schreiben von 1 in das entsprechende Bit im IFCR zum Löschen des Flags */
    uint32_t shift_bit = (channel_num - 1U) * 4U;
    /* CGIFx clears GIF, TCIF, HTIF and TEIF for this channel (W1C). */
    type_def->IFCR = (1U << shift_bit);
    return true;
}
