#include "driver_iwdg.h"

/* ========================================================================= */
/* Implementierung der IWDG-Treiberfunktionen (Interne Logik & Register)     */
/* ========================================================================= */

void driver_iwdg_init(IWDG_Prescaler_t prescaler, uint16_t reload)
{
    // IWDG_KR Register, 496/1136 - RM0008 Datasheet Manual
    //Writing the key value CCCCh starts the watchdog (except if the hardware watchdog option is selected - RM0008 Datasheet Manual
    IWDG->KR = IWDG_KEY_START;
    //Writing the key value 5555h to enable access to the IWDG_PR and IWDG_RLR registers - RM0008 Datasheet Manual
    IWDG->KR = IWDG_KEY_ENABLE_WRITE;

    //IWDG_PR Register,
    //000: divider /4
    // 001: divider /8
    // 010: divider /16
    // 011: divider /32
    // 100: divider /64
    // 101: divider /128
    // 110: divider /256
    // 111: divider /256
    IWDG->PR = (uint32_t)prescaler & 0x07U;
    // Bitmask 0x07U -> 00000111

    //Nachladewert (Reload Value 0..4095) im Register IWDG_RLR einstellen
    IWDG->RLR = (uint32_t)(reload & 0x0FFFU);

    //Zähler sofort auffrischen / laden (Bitschlüssel 0xAAAA schreiben)
    IWDG->KR = IWDG_KEY_REFRESH;
}

void driver_iwdg_refresh(void)
{
    /* Zähler auf den Wert von IWDG_RLR zurücksetzen ("Hund füttern") */
    IWDG->KR = IWDG_KEY_REFRESH;
}

bool driver_iwdg_is_reset_caused_by_watchdog(void)
{
    /* Überprüfen, ob das IWDGRSTF-Bit im RCC_CSR-Register gesetzt ist */
    return ((RCC->CSR & RCC_CSR_IWDGRSTF) != 0U);
}

void driver_iwdg_clear_reset_flags(void)
{
    /* Alle Reset-Flaggen im RCC_CSR-Register zurücksetzen (RMVF-Bit setzen) */
    RCC->CSR |= RCC_CSR_RMVF;
}
