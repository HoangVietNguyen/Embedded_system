#ifndef DRIVER_INTERRUPT_H
#define DRIVER_INTERRUPT_H

#include "stm32f1xx.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================= */
/* Definitionen & Strukturen für den NVIC Interrupt-Treiber                 */
/* ========================================================================= */

/**
 * @brief NVIC Prioritätsgruppen-Konfiguration (Priority Grouping)
 * STM32F103 nutzt 4 Bits für die Prioritätsstufe im NVIC
 */
typedef enum {
    DRIVER_NVIC_PRIO_GROUP_0 = 0x07U, /*!< 0 Bits für Preemption, 4 Bits für Subpriority */
    DRIVER_NVIC_PRIO_GROUP_1 = 0x06U, /*!< 1 Bit für Preemption, 3 Bits für Subpriority */
    DRIVER_NVIC_PRIO_GROUP_2 = 0x05U, /*!< 2 Bits für Preemption, 2 Bits für Subpriority (Standard) */
    DRIVER_NVIC_PRIO_GROUP_3 = 0x04U, /*!< 3 Bits für Preemption, 1 Bit für Subpriority */
    DRIVER_NVIC_PRIO_GROUP_4 = 0x03U  /*!< 4 Bits für Preemption, 0 Bits für Subpriority */
} NVIC_PriorityGroup_t;

/* ========================================================================= */
/* Funktionsprototypen der NVIC Interrupt-Schnittstelle                     */
/* ========================================================================= */

/**
 * @brief Initialisiert das NVIC-Modul und setzt die globale Prioritätsgruppe (Standard: Group 2)
 */
void driver_interrupt_init(void);

/**
 * @brief Setzt die Prioritätsgruppe des NVIC (Priority Grouping)
 * @param priority_group Auswahl der Gruppe (DRIVER_NVIC_PRIO_GROUP_0 bis 4)
 */
void driver_interrupt_set_priority_group(NVIC_PriorityGroup_t priority_group);

/**
 * @brief Konfiguriert und aktiviert/deaktiviert einen NVIC Interrupt Vector
 * @param irq_n Kênh ngắt (IRQn_Type z.B. USART1_IRQn, DMA1_Channel4_IRQn)
 * @param preempt_priority Preemption-Priorität (0 bis 15 je nach Group)
 * @param sub_priority Sub-Priorität (0 bis 15 je nach Group)
 * @param enable true zum Aktivieren, false zum Deaktivieren
 */
void driver_interrupt_nvic_config(IRQn_Type irq_n, uint32_t preempt_priority, uint32_t sub_priority, bool enable);

/**
 * @brief Aktiviert einen spezifischen Interrupt Vector im NVIC (NVIC_EnableIRQ)
 * @param irq_n Kênh ngắt (IRQn_Type)
 */
void driver_interrupt_enable(IRQn_Type irq_n);

/**
 * @brief Deaktiviert einen spezifischen Interrupt Vector im NVIC (NVIC_DisableIRQ)
 * @param irq_n Kênh ngắt (IRQn_Type)
 */
void driver_interrupt_disable(IRQn_Type irq_n);

/**
 * @brief Setzt die Priorität eines spezifischen Interrupt Vectors
 * @param irq_n Kênh ngắt (IRQn_Type)
 * @param preempt_priority Preemption-Priorität
 * @param sub_priority Sub-Priorität
 */
void driver_interrupt_set_priority(IRQn_Type irq_n, uint32_t preempt_priority, uint32_t sub_priority);

/**
 * @brief Löscht ein anhängiges Interrupt-Flag im NVIC (NVIC_ClearPendingIRQ)
 * @param irq_n Kênh ngắt (IRQn_Type)
 */
void driver_interrupt_clear_pending(IRQn_Type irq_n);

#ifdef __cplusplus
}
#endif

#endif // DRIVER_INTERRUPT_H
