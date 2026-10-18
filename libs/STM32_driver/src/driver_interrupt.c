#include "driver_interrupt.h"

/* ========================================================================= */
/* Implementierung der bare-metal Interrupt-Treiberfunktionen (NVIC)        */
/* ========================================================================= */

static NVIC_PriorityGroup_t current_prio_group = DRIVER_NVIC_PRIO_GROUP_2;

/**
 * @brief Initialisiert das NVIC-Modul und setzt die globale Prioritätsgruppe (Standard: Group 2)
 */
void driver_interrupt_init(void) {
    driver_interrupt_set_priority_group(DRIVER_NVIC_PRIO_GROUP_2);
}

/**
 * @brief Setzt die Prioritätsgruppe des NVIC im SCB->AIRCR Register
 */
void driver_interrupt_set_priority_group(NVIC_PriorityGroup_t priority_group) {
    current_prio_group = priority_group;
    NVIC_SetPriorityGrouping((uint32_t)priority_group);
}

/**
 * @brief Setzt die Priorität eines spezifischen Interrupt Vectors
 */
void driver_interrupt_set_priority(IRQn_Type irq_n, uint32_t preempt_priority, uint32_t sub_priority) {
    if ((int32_t)irq_n >= 0) {
        uint32_t priority_value = NVIC_EncodePriority((uint32_t)current_prio_group, preempt_priority, sub_priority);
        NVIC_SetPriority(irq_n, priority_value);
    }
}

/**
 * @brief Aktiviert einen spezifischen Interrupt Vector im NVIC
 */
void driver_interrupt_enable(IRQn_Type irq_n) {
    if ((int32_t)irq_n >= 0) {
        NVIC_EnableIRQ(irq_n);
    }
}

/**
 * @brief Deaktiviert einen spezifischen Interrupt Vector im NVIC
 */
void driver_interrupt_disable(IRQn_Type irq_n) {
    if ((int32_t)irq_n >= 0) {
        NVIC_DisableIRQ(irq_n);
    }
}

/**
 * @brief Löscht ein anhängiges Interrupt-Flag im NVIC
 */
void driver_interrupt_clear_pending(IRQn_Type irq_n) {
    if ((int32_t)irq_n >= 0) {
        NVIC_ClearPendingIRQ(irq_n);
    }
}

/**
 * @brief Konfiguriert und aktiviert/deaktiviert einen NVIC Interrupt Vector
 */
void driver_interrupt_nvic_config(IRQn_Type irq_n, uint32_t preempt_priority, uint32_t sub_priority, bool enable) {
    if ((int32_t)irq_n < 0) {
        return;
    }

    if (enable) {
        driver_interrupt_set_priority(irq_n, preempt_priority, sub_priority);
        driver_interrupt_clear_pending(irq_n);
        driver_interrupt_enable(irq_n);
    } else {
        driver_interrupt_disable(irq_n);
    }
}

/* ========================================================================= */
/* Schwache Standard-Implementierungen (Weak ISR Handlers) ALLER Peripherien  */
/* ========================================================================= */

/* System & Control Interrupt Vectors */
__attribute__((weak)) void WWDG_IRQHandler(void) {}
__attribute__((weak)) void PVD_IRQHandler(void) {}
__attribute__((weak)) void TAMPER_IRQHandler(void) {}
__attribute__((weak)) void RTC_IRQHandler(void) {}
__attribute__((weak)) void FLASH_IRQHandler(void) {}
__attribute__((weak)) void RCC_IRQHandler(void) {}
__attribute__((weak)) void RTC_Alarm_IRQHandler(void) {}

/* USART & UART Interrupt Vectors */
/* USART1..3 handlers are implemented by driver_usart.c for RXNE callbacks. */
__attribute__((weak)) void USART1_IRQHandler(void) {}
__attribute__((weak)) void USART2_IRQHandler(void) {}
__attribute__((weak)) void USART3_IRQHandler(void) {}
__attribute__((weak)) void UART4_IRQHandler(void) {}
__attribute__((weak)) void UART5_IRQHandler(void) {}

/* DMA Interrupt Vectors */
/* DMA1_Channel1_IRQHandler is overridden by driver_adc.c for ADC1 DMA change
 * notifications; the weak definition remains the fallback for other builds. */
__attribute__((weak)) void DMA1_Channel1_IRQHandler(void) {}
__attribute__((weak)) void DMA1_Channel2_IRQHandler(void) {}
__attribute__((weak)) void DMA1_Channel3_IRQHandler(void) {}
__attribute__((weak)) void DMA1_Channel4_IRQHandler(void) {}
__attribute__((weak)) void DMA1_Channel5_IRQHandler(void) {}
__attribute__((weak)) void DMA1_Channel6_IRQHandler(void) {}
__attribute__((weak)) void DMA1_Channel7_IRQHandler(void) {}
__attribute__((weak)) void DMA2_Channel1_IRQHandler(void) {}
__attribute__((weak)) void DMA2_Channel2_IRQHandler(void) {}
__attribute__((weak)) void DMA2_Channel3_IRQHandler(void) {}
__attribute__((weak)) void DMA2_Channel4_5_IRQHandler(void) {}

/* ADC & EXTI Interrupt Vectors */
__attribute__((weak)) void ADC1_2_IRQHandler(void) {}
__attribute__((weak)) void ADC3_IRQHandler(void) {}
__attribute__((weak)) void EXTI0_IRQHandler(void) {}
__attribute__((weak)) void EXTI1_IRQHandler(void) {}
__attribute__((weak)) void EXTI2_IRQHandler(void) {}
__attribute__((weak)) void EXTI3_IRQHandler(void) {}
__attribute__((weak)) void EXTI4_IRQHandler(void) {}
__attribute__((weak)) void EXTI9_5_IRQHandler(void) {}
__attribute__((weak)) void EXTI15_10_IRQHandler(void) {}

/* Timer Interrupt Vectors */
__attribute__((weak)) void TIM1_BRK_IRQHandler(void) {}
__attribute__((weak)) void TIM1_UP_IRQHandler(void) {}
__attribute__((weak)) void TIM1_TRG_COM_IRQHandler(void) {}
__attribute__((weak)) void TIM1_CC_IRQHandler(void) {}
__attribute__((weak)) void TIM2_IRQHandler(void) {}
__attribute__((weak)) void TIM3_IRQHandler(void) {}
__attribute__((weak)) void TIM4_IRQHandler(void) {}
__attribute__((weak)) void TIM5_IRQHandler(void) {}
__attribute__((weak)) void TIM6_IRQHandler(void) {}
__attribute__((weak)) void TIM7_IRQHandler(void) {}
__attribute__((weak)) void TIM8_BRK_IRQHandler(void) {}
__attribute__((weak)) void TIM8_UP_IRQHandler(void) {}
__attribute__((weak)) void TIM8_TRG_COM_IRQHandler(void) {}
__attribute__((weak)) void TIM8_CC_IRQHandler(void) {}

/* Communication (I2C, SPI, USB, CAN, SDIO, FSMC) Interrupt Vectors */
__attribute__((weak)) void I2C1_EV_IRQHandler(void) {}
__attribute__((weak)) void I2C1_ER_IRQHandler(void) {}
__attribute__((weak)) void I2C2_EV_IRQHandler(void) {}
__attribute__((weak)) void I2C2_ER_IRQHandler(void) {}
__attribute__((weak)) void SPI1_IRQHandler(void) {}
__attribute__((weak)) void SPI2_IRQHandler(void) {}
__attribute__((weak)) void SPI3_IRQHandler(void) {}
__attribute__((weak)) void USB_HP_CAN1_TX_IRQHandler(void) {}
__attribute__((weak)) void USB_LP_CAN1_RX0_IRQHandler(void) {}
__attribute__((weak)) void CAN1_RX1_IRQHandler(void) {}
__attribute__((weak)) void CAN1_SCE_IRQHandler(void) {}
__attribute__((weak)) void USBWakeUp_IRQHandler(void) {}
__attribute__((weak)) void SDIO_IRQHandler(void) {}
__attribute__((weak)) void FSMC_IRQHandler(void) {}
