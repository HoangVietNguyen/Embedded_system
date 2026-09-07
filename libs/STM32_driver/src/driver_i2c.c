#include "driver_i2c.h"
#include "driver_gpio.h"

/* ========================================================================= */
/* Implementierung der I2C-Treiberfunktionen (Interne Logik & Register)      */
/* ========================================================================= */

/* ========================================================================= */
/* 1. Makros für Geschwindigkeits- und Duty-Cycle-Konfiguration (v1 vs v2)  */
/* ========================================================================= */

#if defined(STM32G0) || defined(STM32G030xx) || defined(STM32F0) || defined(STM32F7) || defined(STM32G4)
/* --- I2C v2 Architektur (STM32G0/F0/F7/G4 - Verwendet TIMINGR-Register) --- */
#define I2C_SET_SPEED(i2c, speed) \
do { \
    if ((speed) == I2C_SPEED_FAST_400KHz) { \
        (i2c)->TIMINGR = 0x00300F38U; /* Fast mode 400kHz @ 64MHz HCLK */ \
    } else { \
        (i2c)->TIMINGR = 0x10707DBCU; /* Standard mode 100kHz @ 64MHz HCLK */ \
    } \
} while(0U)

#define I2C_DUTY_CYCLE(i2c, speed) do {} while(0U)

#else
/* --- Standard I2C v1 Architektur (STM32F1/F4/F2 - Verwendet CCR-Register) --- */
#define I2C_SET_SPEED(i2c, speed) \
do { \
    (i2c)->CR2 &= ~I2C_CR2_FREQ; \
    (i2c)->CR2 |= 36U; /* APB1 Clock Frequency = 36 MHz */ \
    if ((speed) == I2C_SPEED_FAST_400KHz) { \
        (i2c)->CCR = I2C_CCR_FS | 30U; /* Fast Mode 400kHz: CCR = 36MHz / (3 * 400kHz) = 30 */ \
        (i2c)->TRISE = 12U;             /* Maximum Rise Time in Fast Mode */ \
    } else { \
        (i2c)->CCR = 180U;             /* Standard Mode 100kHz: CCR = 36MHz / (2 * 100kHz) = 180 */ \
        (i2c)->TRISE = 37U;            /* Maximum Rise Time in Standard Mode (36 + 1) */ \
    } \
} while(0U)

#define I2C_DUTY_CYCLE(i2c, speed) do {} while(0U)
#endif

/* ========================================================================= */
/* 2. RCC-Taktaktivierungsmakros für plattformübergreifende Mikrocontroller  */
/* ========================================================================= */

#if defined(STM32F407xx) || defined(STM32F411xE) || defined(STM32F401xC) || defined(STM32F4)
/* --- STM32F4-Serie (APB1-Bus) --- */
#define I2C_ENABLE_MODUL(i2c) \
do { \
    if ((i2c) == I2C1) { RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; } \
    else if ((i2c) == I2C2) { RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; } \
    #ifdef RCC_APB1ENR_I2C3EN \
    else if ((i2c) == I2C3) { RCC->APB1ENR |= RCC_APB1ENR_I2C3EN; } \
    #endif \
} while(0U)

#elif defined(STM32G030xx) || defined(STM32G071xx) || defined(STM32G0)
/* --- STM32G0-Serie (Neuer APBENR1-Bus) --- */
#define I2C_ENABLE_MODUL(i2c) \
do { \
    if ((i2c) == I2C1) { RCC->APBENR1 |= RCC_APBENR1_I2C1EN; } \
    else if ((i2c) == I2C2) { RCC->APBENR1 |= RCC_APBENR1_I2C2EN; } \
} while(0U)

#else
/* --- Standard STM32F1-Serie (STM32F103xB, STM32F103xE...) --- */
#define I2C_ENABLE_MODUL(i2c) \
do { \
    if ((i2c) == I2C1) { RCC->APB1ENR |= RCC_APB1ENR_I2C1EN; } \
    else if ((i2c) == I2C2) { RCC->APB1ENR |= RCC_APB1ENR_I2C2EN; } \
} while(0U)
#endif

/* Makro zur Konfiguration der eigenen Slave-Adresse (OAR1 & OAR2) */
#define SET_OWNER_ADDRESS(mode, i2c, own_addr1, own_addr2) \
do { \
    if ((mode) == I2C_SINGLE_ADDRESS) { \
        (i2c)->OAR2 &= ~I2C_OAR2_ENDUAL; \
        (i2c)->OAR1 = (1U << 14) | (((own_addr1) & 0x7F) << 1); \
    } else if ((mode) == I2C_DUAL_ADDRESS) { \
        (i2c)->OAR1 = (1U << 14) | (((own_addr1) & 0x7F) << 1); \
        (i2c)->OAR2 = I2C_OAR2_ENDUAL | (((own_addr2) & 0x7F) << 1); \
    } \
} while(0U)

/* ========================================================================= */
/* 3. Allgemeine I2C-Initialisierungsfunktion (Plattformübergreifend)       */
/* ========================================================================= */

void init_i2c_modul(I2C_TypeDef *i2c, I2C_Speed_t speed) {
    if (i2c == NULL) {
        return;
    }

    if (speed != I2C_SPEED_STANDARD_100KHz && speed != I2C_SPEED_FAST_400KHz) {
        speed = I2C_SPEED_STANDARD_100KHz;
    }

    driver_gpio_init_pin(GPIOB, 6, GPIO_MODE_AF_OD_50MHZ);
    driver_gpio_init_pin(GPIOB, 7, GPIO_MODE_AF_OD_50MHZ);

    /* 1. Automatische Taktaktivierung für die entsprechende MCU-Serie */
    I2C_ENABLE_MODUL(i2c);

    /* 2. PE-Bit deaktivieren für eine sichere Konfiguration der Register */
    i2c->CR1 &= ~I2C_CR1_PE;

    /* 3. Sicherstellen, dass Clock Stretching aktiviert ist (Clear Bit NOSTRETCH) */
    #ifdef I2C_CR1_NOSTRETCH
    i2c->CR1 &= ~I2C_CR1_NOSTRETCH;
    #endif

    /* 4. Konfiguration der Busgeschwindigkeit (CR2, CCR & TRISE Register) */
    I2C_SET_SPEED(i2c, speed);

    /* 5. PE-Bit wieder aktivieren, damit das I2C-Modul betriebsbereit ist */
    i2c->CR1 |= I2C_CR1_PE;
}

void init_gpio_i2c(uint8_t scl_gpio) {
    uint8_t sda_gpio;
    if (scl_gpio != 6 && scl_gpio != 8 && scl_gpio != 10) {
        return;
    }

    sda_gpio = scl_gpio + 1;

    #if defined(STM32F1) || defined(STM32F103xB)
    if (scl_gpio == 8) {
        RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
        AFIO->MAPR |= AFIO_MAPR_I2C1_REMAP;
    }
    #endif

    driver_gpio_init_pin(GPIOB, scl_gpio, GPIO_MODE_AF_OD_50MHZ);
    driver_gpio_init_pin(GPIOB, sda_gpio, GPIO_MODE_AF_OD_50MHZ);
}

// Konfiguration der Slave-Adresse (Nur verwendet, wenn die MCU als Slave agiert)
void set_owner_address(I2C_TypeDef *i2c, I2C_Address_Mode mode, uint8_t own_addr1, uint8_t own_addr2) {
    if (i2c == NULL || ((own_addr1 & 0x7F) == 0 && (own_addr2 & 0x7F) == 0)) {
        return;
    }

    i2c->CR1 &= ~I2C_CR1_PE;

    SET_OWNER_ADDRESS(mode, i2c, own_addr1, own_addr2);

    i2c->CR1 |= I2C_CR1_PE;
}

/* ========================================================================= */
/* 4. I2C-Master-Datenübertragungsfunktion (Registerebene)                  */
/* ========================================================================= */

I2C_Status_t driver_i2c_master_transmit(I2C_TypeDef *i2c, uint8_t dev_addr, const uint8_t *data, uint16_t len) {
    if (i2c == NULL || data == NULL || len == 0) {
        return I2C_STATUS_ERROR;
    }

    uint32_t timeout = 10000U;

    /* 1. Warten bis der I2C-Bus frei ist (Bit BUSY im SR2-Register ist 0) */
    while ((i2c->SR2 & I2C_SR2_BUSY) && --timeout);
    if (timeout == 0) return I2C_STATUS_TIMEOUT;

    /* 2. START-Bedingung erzeugen (Bit START im CR1-Register setzen) */
    i2c->CR1 |= I2C_CR1_START;

    /* Warten auf SB-Bit (Start Bit generiert) im SR1-Register */
    timeout = 10000U;
    while (!(i2c->SR1 & I2C_SR1_SB) && --timeout);
    if (timeout == 0) return I2C_STATUS_TIMEOUT;

    /* 3. Slave-Adresse + Schreibbit senden (7-Bit-Adresse nach links verschieben, LSB = 0) */
    i2c->DR = (dev_addr << 1) & ~0x01U;

    /* Warten auf ADDR-Bit (Adresse bestätigt) im SR1-Register */
    timeout = 10000U;
    while (!(i2c->SR1 & I2C_SR1_ADDR) && --timeout);
    if (timeout == 0) return I2C_STATUS_TIMEOUT;

    /* ADDR-Bit löschen durch Lesen von SR1 gefolgt von SR2 */
    (void)i2c->SR1;
    (void)i2c->SR2;

    /* 4. Schleife zur Übertragung der Datenbytes */
    for (uint16_t i = 0; i < len; i++) {
        /* Warten bis das Senderegister TXE (Transmit Empty) bereit ist */
        timeout = 10000U;
        while (!(i2c->SR1 & I2C_SR1_TXE) && --timeout);
        if (timeout == 0) return I2C_STATUS_TIMEOUT;

        /* Datenbyte in das DR-Register schreiben */
        i2c->DR = data[i];
    }

    /* 5. Warten bis die Übertragung des letzten Bytes abgeschlossen ist (BTF-Bit) */
    timeout = 10000U;
    while (!(i2c->SR1 & I2C_SR1_BTF) && --timeout);
    if (timeout == 0) return I2C_STATUS_TIMEOUT;

    /* 6. STOP-Bedingung erzeugen (Bit STOP im CR1-Register setzen) */
    i2c->CR1 |= I2C_CR1_STOP;

    return I2C_STATUS_OK;
}
