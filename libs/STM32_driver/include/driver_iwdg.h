#ifndef DRIVER_IWDG_H
#define DRIVER_IWDG_H

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Definitionen für Enums, Strukturen & Makros des IWDG-Treibers            */
/* ========================================================================= */

/**
 * @brief Schlüsseleingabe-Werte für das IWDG-Schlüsselregister (IWDG_KR)
 */
#define IWDG_KEY_ENABLE_WRITE   0x5555U  /*!< Schreiben in PR & RLR freischalten */
#define IWDG_KEY_REFRESH        0xAAAAU  /*!< Zähler neu laden (Hund füttern) */
#define IWDG_KEY_START          0xCCCCU  /*!< Unabhängigen Watchdog starten */

/**
 * @brief Enums für IWDG-Frequenzteiler (Prescaler-Werte PR[2:0])
 */
typedef enum {
    IWDG_PRESCALER_4   = 0U,  /*!< Taktteiler /4 */
    IWDG_PRESCALER_8   = 1U,  /*!< Taktteiler /8 */
    IWDG_PRESCALER_16  = 2U,  /*!< Taktteiler /16 */
    IWDG_PRESCALER_32  = 3U,  /*!< Taktteiler /32 */
    IWDG_PRESCALER_64  = 4U,  /*!< Taktteiler /64 */
    IWDG_PRESCALER_128 = 5U,  /*!< Taktteiler /128 */
    IWDG_PRESCALER_256 = 6U   /*!< Taktteiler /256 */
} IWDG_Prescaler_t;

/* ========================================================================= */
/* Funktionsprototypen der IWDG-Treiberschnittstelle (Öffentliche API)      */
/* ========================================================================= */

/**
 * @brief Initialisiert und startet den Unabhängigen Watchdog (IWDG)
 * @param prescaler Frequenzteiler (IWDG_PRESCALER_4 bis IWDG_PRESCALER_256)
 * @param reload Zähler-Nachladewert (12-Bit: 0 bis 4095 / 0x0FFF)
 */
void driver_iwdg_init(IWDG_Prescaler_t prescaler, uint16_t reload);

/**
 * @brief Lädt den IWDG-Zähler neu (Watchdog auffrischen / "Hund füttern")
 */
void driver_iwdg_refresh(void);

/**
 * @brief Überprüft, ob der letzte Systemstart durch einen IWDG-Reset verursacht wurde
 * @return true if Reset durch IWDG ausgelöst wurde, false sonst
 */
bool driver_iwdg_is_reset_caused_by_watchdog(void);

/**
 * @brief Löscht die Flaggen des System-Reset-Statusregisters (RCC_CSR)
 */
void driver_iwdg_clear_reset_flags(void);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_IWDG_H
