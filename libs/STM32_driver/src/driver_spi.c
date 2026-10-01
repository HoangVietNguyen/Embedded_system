#include "driver_spi.h"
#include "driver_gpio.h"

/* Makro zur internen Taktaktivierung des entsprechenden SPI */
// RM008 pdf page 275/1136, SPI1-APB1, SPI2-APB2
// SPI1: APB2 Bus -> RCC_APB2ENR Register -> Bit 12 enable, page 146/1136
// SPI2,3: APB1 Bus -> RCC_APB1ENR Register -> Bit 14 (SPI2), Bit 15 (SPI3) enable, page 116/1136
#define DRIVER_SPI_ENABLE(PORT) \
do { \
    if ((PORT) == SPI1) { \
        RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;\
        driver_gpio_clock_enable(GPIOA); \
        driver_gpio_init_pin(GPIOA, 4 , GPIO_MODE_OUTPUT_PP_50MHZ); \
        driver_gpio_init_pin(GPIOA, 5 , GPIO_MODE_AF_PP_50MHZ); \
        driver_gpio_init_pin(GPIOA, 7 , GPIO_MODE_AF_PP_50MHZ); \
        driver_gpio_init_pin(GPIOA, 6 , GPIO_MODE_INPUT_PULLUP); \
        SPI1->CR1 &= ~SPI_CR1_SPE; \
    } else if ((PORT) == SPI2) { \
        RCC->APB1ENR |= RCC_APB1ENR_SPI2EN;\
        driver_gpio_clock_enable(GPIOB); \
        driver_gpio_init_pin(GPIOB, 12 , GPIO_MODE_OUTPUT_PP_50MHZ); \
        driver_gpio_init_pin(GPIOB, 13 , GPIO_MODE_AF_PP_50MHZ); \
        driver_gpio_init_pin(GPIOB, 15 , GPIO_MODE_AF_PP_50MHZ); \
        driver_gpio_init_pin(GPIOB, 14 , GPIO_MODE_INPUT_PULLUP); \
        SPI2->CR1 &= ~SPI_CR1_SPE; \
    } \
} while(0)

void driver_spi_init(SPI_TypeDef *SPIx, const SPI_Config_t *config) {
    if (SPIx == NULL || config == NULL) {
        return;
    }

    RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;

    DRIVER_SPI_ENABLE(SPIx);

    if (config->mode == SPI_MODE_MASTER) {
        SPIx->CR1 |= SPI_CR1_MSTR;
    } else {
        SPIx->CR1 &= ~SPI_CR1_MSTR;
    }

    /* Software Slave Management aktivieren (SSM = 1, SSI = 1) zur Vermeidung von MODF-Fehlern */
    SPIx->CR1 &= ~(3UL << SPI_CR1_SSI_Pos);
    SPIx->CR1 |= SPI_CR1_SSM;
    SPIx->CR1 |= SPI_CR1_SSI;

    /* Bitfeld BR[2:0] loeschen und neue Baudrate setzen */
    SPIx->CR1 &= ~(7U << SPI_CR1_BR_Pos);
    SPIx->CR1 |= (config->baudrate << SPI_CR1_BR_Pos);

    /* Clock Polarity (CPOL) */
    if (config->cpol == DRIVER_SPI_CPOL_HIGH) {
        SPIx->CR1 |= SPI_CR1_CPOL;
    }

    /* Clock Phase (CPHA) */
    if (config->cpha == DRIVER_SPI_CPHA_2EDGE) {
        SPIx->CR1 |= SPI_CR1_CPHA;
    }

    if (config->data_frame == DRIVER_SPI_DFF_16BIT) {
        SPIx->CR1 |= SPI_CR1_DFF;
    }

    if (config->first_bit == DRIVER_SPI_FIRSTBIT_LSB) {
        SPIx->CR1 |= SPI_CR1_LSBFIRST;
    }

    SPIx->CR1 |= SPI_CR1_SPE;
}

uint8_t driver_spi_transmit_receive_byte(SPI_TypeDef *SPIx, uint8_t tx_data) {
    if (SPIx == NULL) {
        return 0U;
    }

    // Warten auf TXE-Bit auf 1 einzuschalten -> Tx buffer (Sendebuffer) ist leer.
    // -> Chờ cờ TXE (Bit 1 trong SPIx->SR) bật lên 1 -> Bộ đệm truyền đã sẵn sàng nhận dữ liệu mới
    while (!(SPIx->SR & SPI_SR_TXE));

    // Die gesenden Daten tx_data werden im DR-Register geschrieben
    // Nach dem die Daten im DR-Register geschrieben wurden, startet die Hardware automatisch 8 Takimpulse auf der SCK-Leitung
    // -> Ghi dữ liệu tx_data vào thanh ghi DR -> Kích hoạt phần cứng phát 8 xung clock SCK và truyền dữ liệu qua MOSI
    *(volatile uint8_t *)&SPIx->DR = tx_data;

    // Warten auf RXNE-Bit auf 1 einzuschalten -> Rx buffer (Sendebuffer) hat die Daten zurückerhaltet.
    // -> Chờ cờ RXNE (Bit 0 trong SPIx->SR) bật lên 1 -> 8 xung clock hoàn tất và bộ đệm nhận đã nhận xong dữ liệu từ MISO
    while (!(SPIx->SR & SPI_SR_RXNE));

    // -> Đọc và trả về byte dữ liệu nhận được từ thanh ghi DR (Thao tác đọc này sẽ tự động xóa cờ RXNE)
    return (uint8_t)(*(volatile uint8_t *)&SPIx->DR);
}

void driver_spi_transmit(SPI_TypeDef *SPIx, const uint8_t *data, uint16_t size) {
    if (SPIx == NULL || data == NULL || size == 0U) {
        return;
    }

    for (uint16_t i = 0U; i < size; i++) {
        /* Gửi byte data[i], bỏ qua giá trị trả về */
        (void)driver_spi_transmit_receive_byte(SPIx, data[i]);
    }

    /* Chờ cho đến khi SPI-Bus rảnh hoàn toàn (BSY = 0) */
    while (SPIx->SR & SPI_SR_BSY);
}

void driver_spi_receive(SPI_TypeDef *SPIx, uint8_t *data, uint16_t size) {
    if (SPIx == NULL || data == NULL || size == 0U) {
        return;
    }

    for (uint16_t i = 0U; i < size; i++) {
        /* Gửi Dummy Byte (0xFF) để kích hoạt Clock SCK và nhận byte dữ liệu về */
        data[i] = driver_spi_transmit_receive_byte(SPIx, 0xFFU);
    }

    /* Chờ cho đến khi SPI-Bus rảnh hoàn toàn (BSY = 0) */
    while (SPIx->SR & SPI_SR_BSY);
}

void driver_spi_cs_low(GPIO_TypeDef *port, uint8_t pin) {
    driver_gpio_write_pin(port, pin, DRIVER_GPIO_PIN_RESET);
}

void driver_spi_cs_high(GPIO_TypeDef *port, uint8_t pin) {
    driver_gpio_write_pin(port, pin, DRIVER_GPIO_PIN_SET);
}

uint16_t driver_spi_transmit_receive_16bit(SPI_TypeDef *SPIx, uint16_t tx_data) {
    if (SPIx == NULL) {
        return 0U;
    }

    // Warten auf TXE-Bit auf 1 einzuschalten -> Tx buffer (Sendebuffer) ist leer.
    // -> Chờ cờ TXE (Bit 1 trong SPIx->SR) bật lên 1 -> Bộ đệm truyền đã sẵn sàng nhận dữ liệu mới
    while (!(SPIx->SR & SPI_SR_TXE));

    // Die gesendeten 16-Bit Daten tx_data werden im DR-Register geschrieben (Half-Word Zugriffsmodus)
    // -> Ghi dữ liệu 16-bit tx_data vào thanh ghi DR (Ép kiểu truy cập 16-bit) -> Kích hoạt phần cứng phát 16 xung clock SCK
    *(volatile uint16_t *)&SPIx->DR = tx_data;

    // Warten auf RXNE-Bit auf 1 einzuschalten -> Rx buffer hat 16-Bit Daten empfangen.
    // -> Chờ cờ RXNE (Bit 0 trong SPIx->SR) bật lên 1 -> 16 xung clock hoàn tất và bộ đệm nhận đã nhận xong 16-bit từ MISO
    while (!(SPIx->SR & SPI_SR_RXNE));

    // -> Đọc và trả về 16-bit dữ liệu nhận được từ thanh ghi DR (Thao tác đọc 16-bit tự động xóa cờ RXNE)
    return *(volatile uint16_t *)&SPIx->DR;
}

void driver_spi_transmit_16bit(SPI_TypeDef *SPIx, const uint16_t *data, uint16_t size) {
    if (SPIx == NULL || data == NULL || size == 0U) {
        return;
    }

    for (uint16_t i = 0U; i < size; i++) {
        /* Sendet 16-Bit Word data[i], Rückgabewert ignorieren */
        /* -> Gửi từ 16-bit data[i], bỏ qua giá trị trả về */
        (void)driver_spi_transmit_receive_16bit(SPIx, data[i]);
    }

    /* Chờ cho đến khi SPI-Bus rảnh hoàn toàn (BSY = 0) */
    while (SPIx->SR & SPI_SR_BSY);
}

void driver_spi_receive_16bit(SPI_TypeDef *SPIx, uint16_t *data, uint16_t size) {
    if (SPIx == NULL || data == NULL || size == 0U) {
        return;
    }

    for (uint16_t i = 0U; i < size; i++) {
        /* Sendet 16-Bit Dummy-Word (0xFFFF) um SCK zu generieren und 16-Bit Empfangsdaten zu lesen */
        /* -> Gửi 16-bit Dummy Word (0xFFFF) để kích hoạt Clock SCK và nhận 16-bit dữ liệu về */
        data[i] = driver_spi_transmit_receive_16bit(SPIx, 0xFFFFU);
    }

    /* Chờ cho đến khi SPI-Bus rảnh hoàn toàn (BSY = 0) */
    while (SPIx->SR & SPI_SR_BSY);
}
