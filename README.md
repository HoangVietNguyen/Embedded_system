# STM32F103 Bare-Metal Register-Treiberbibliothek & Embedded-Projekt

Projekt zur Entwicklung einer Bare-Metal-Register-Treiberbibliothek (direkter Zugriff auf Hardware-Register) für die Mikrocontroller-Familie **STM32F103** (ARM Cortex-M3). Die Bibliothek ist nach dem **C11-Standard** entwickelt, hält sich strikt an das **ST RM0008 Reference Manual**, optimiert die Systemleistung, minimiert den Speicherbedarf (Flash/RAM) und kommt vollständig ohne HAL-Bibliotheken aus.

---

## 1. Hauptmerkmale (Key Features)

* **Reine Bare-Metal-Implementierung**: Direkter Zugriff auf Hardware-Register (`RCC`, `DMA`, `ADC`, `TIM`, `GPIO`, `I2C`) für maximale Ausführungsgeschwindigkeit.
* **Automatische DMA-Übertragung (0% CPU-Last)**: Zirkulärer DMA-Modus (Circular Mode) zum kontinuierlichen Einlesen von ADC1-Werten und Generieren von PWM-Signalen ohne CPU-Intervention.
* **Hohe Portabilität**: Nahtlose Unterstützung für STM32F1 Medium-Density (`STM32F103xB`) und High-Density (`STM32F103xE`) Mikrokontroller.
* **Speichersicherheit & W1C-Konformität**: Korrekte Handhabung des Write-1-to-Clear (W1C)-Mechanismus im `IFCR`-Register sowie Beachtung der C-Operatorpriorität.

---

## 2. Architektur der Bare-Metal-Treiber (`libs/STM32_driver`)

| Treibermodul | Quelldateien | Hauptfunktion |
| :--- | :--- | :--- |
| **DMA-Treiber** | [driver_dma.h](file:///Users/nguyenhoangviet/Desktop/STM32Project/Embedded_system/libs/STM32_driver/include/driver_dma.h), [driver_dma.c](file:///Users/nguyenhoangviet/Desktop/STM32Project/Embedded_system/libs/STM32_driver/src/driver_dma.c) | Steuerung der DMA1/DMA2-Controller (`CCR`, `CPAR`, `CMAR`, `CNDTR`, `ISR`, `IFCR`). |
| **ADC-Treiber** | [driver_adc.h](file:///Users/nguyenhoangviet/Desktop/STM32Project/Embedded_system/libs/STM32_driver/include/driver_adc.h), [driver_adc.c](file:///Users/nguyenhoangviet/Desktop/STM32Project/Embedded_system/libs/STM32_driver/src/driver_adc.c) | Initialisierung des 12-Bit-ADC1, Selbstkalibrierung (Calibration), Polling & DMA-Modus. |
| **TIM-Treiber** | [driver_tim.h](file:///Users/nguyenhoangviet/Desktop/STM32Project/Embedded_system/libs/STM32_driver/include/driver_tim.h), [driver_tim.c](file:///Users/nguyenhoangviet/Desktop/STM32Project/Embedded_system/libs/STM32_driver/src/driver_tim.c) | Timer 1..4 Timebase, Delay us/ms, Update Interrupt, PWM Output & SG90 Servo-Steuerung. |
| **GPIO-Treiber**| [driver_gpio.h](file:///Users/nguyenhoangviet/Desktop/STM32Project/Embedded_system/libs/STM32_driver/include/driver_gpio.h), [driver_gpio.c](file:///Users/nguyenhoangviet/Desktop/STM32Project/Embedded_system/libs/STM32_driver/src/driver_gpio.c) | Konfiguration von GPIO-Pins (Input, Output, Alternate Function Push-Pull). |
| **I2C-Treiber**  | [driver_i2c.h](file:///Users/nguyenhoangviet/Desktop/STM32Project/Embedded_system/libs/STM32_driver/include/driver_i2c.h), [driver_i2c.c](file:///Users/nguyenhoangviet/Desktop/STM32Project/Embedded_system/libs/STM32_driver/src/driver_i2c.c) | Hardware-I2C-Seriellkommunikation für LCD 16x2 / 20x4 Displays. |

---

## 3. Detaillierte DMA-Controller-Architektur (`driver_dma`)

### 3.1 Signalflussdiagramm (DMA <-> Peripherie)

```text
+-------------------+      Hardware DMA Request      +--------------------+
|  Peripherie (ADC) | -----------------------------> |   DMA Controller   |
|   (Register DR)   |                                |  (DMA1_Channel1)   |
+-------------------+                                +--------------------+
          |                                                     |
          |       Automatische Datenübertragung via Bus         |
          +=====================================================+
                                    ||
                                    \/
                             +--------------+
                             |  RAM-Speicher|
                             | (adc_buffer) |
                             +--------------+
```

### 3.2 Hardware-Zuordnungstabelle der DMA1-Kanäle (STM32F103)

| Hardware-Peripherie | Fester DMA1-Kanal | Übertragungsrichtung (`direction`) | Ziel-/Quellregister |
| :--- | :--- | :--- | :--- |
| **ADC1** | **DMA1 Channel 1** | Peripherie $\rightarrow$ RAM (`DIR = 0`) | `ADC1->DR` |
| **SPI1_RX / SPI1_TX** | **DMA1 Channel 2 / 3** | Peripherie $\rightarrow$ RAM / RAM $\rightarrow$ Peripherie | `SPI1->DR` |
| **USART1_TX / USART1_RX** | **DMA1 Channel 4 / 5** | RAM $\rightarrow$ Peripherie / Peripherie $\rightarrow$ RAM | `USART1->DR` |
| **TIM4_CH1 / TIM4_UP** | **DMA1 Channel 1 / 7** | RAM $\rightarrow$ Peripherie | `TIM4->CCR1` |

### 3.3 Registerstruktur von `DMA_Channel_TypeDef`

1. **`CCR` (Channel Configuration Register)**: Kanalsteuerung (`EN`), Richtung (`DIR`/`MEM2MEM`), Datengröße (`PSIZE`/`MSIZE`), Adressinkrementierung (`PINC`/`MINC`), Priorität (`PL`), Zirkulärmodus (`CIRC`).
2. **`CPAR` (Peripheral Address Register)**: Basisadresse des Peripherieregisters (`(uint32_t)&(ADC1->DR)`).
3. **`CMAR` (Memory Address Register)**: Basisadresse des RAM-Puffers (`(uint32_t)&adc_value`).
4. **`CNDTR` (Number of Data Register)**: Anzahl der zu übertragenden Datenelemente.
5. **`ISR` / `IFCR` (Status & Flag Clear)**: 
   * `ISR`: Schreibgeschützt (Read-Only zur Prüfung des `TCIF`-Flags).
   * `IFCR`: **Write-1-to-Clear (W1C)** – Schreiben von `1` löscht das entsprechende Flag im `ISR`.

---

## 4. Anwendungsbeispiel: SG90 Servo-Steuerung über ADC1 + DMA + TIM4 PWM

Im Quellcode [main.c](file:///Users/nguyenhoangviet/Desktop/STM32Project/Embedded_system/Core/Src/main.c):
1. **ADC1** liest kontinuierlich den Potentiometerwert an Pin `PA0` (ADC_IN0).
2. **DMA1 Channel 1** überträgt die Wandlungsergebnisse automatisch im Hintergrund von `ADC1->DR` in die RAM-Variable `adc_value`.
3. **TIM4 Channel 1** generiert ein 50Hz PWM-Signal (20ms Periodendauer) an Pin `PB6`.
4. Die CPU liest lediglich die Variable `adc_value` aus und skaliert die Impulsbreite im Bereich `1.3ms .. 1.7ms` ohne Wartezeiten.

```c
#include "main.h"
#include "driver_gpio.h"
#include "driver_adc.h"
#include "driver_dma.h"
#include "driver_tim.h"

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    /* 1. GPIO-Takt und Pins initialisieren */
    driver_gpio_clock_enable(GPIOA);
    driver_gpio_clock_enable(GPIOB);
    driver_gpio_init_pin(GPIOA, 0, GPIO_MODE_INPUT_ANALOG);
    driver_gpio_init_pin(GPIOB, 6, GPIO_MODE_AF_PP_50MHZ);

    /* 2. ADC1 und TIM4 PWM 50Hz initialisieren */
    driver_adc_init(DRIVER_ADC_PRESCALER_DIV6);
    driver_adc_config_channel(ADC_CHANNEL_0, DRIVER_ADC_SAMPLETIME_239CYCLES5);
    driver_tim_pwm_init(TIM4, TIM_CHANNEL_1, 71, 19999);

    static volatile uint32_t adc_value = 0;

    /* 3. DMA1-Kanal 1 für kontinuierliche ADC1-Übertragung starten */
    read_adc_dma((uint32_t*)&adc_value);

    while (1)
    {
        /* ADC-Wert direkt aus der RAM-Variable auslesen */
        uint16_t adc_value_read = (uint16_t)adc_value;

        if (adc_value_read > 4085) {
            adc_value_read = 4085;
        }

        /* Skalierung auf Impulsdauer 1.3ms bis 1.7ms (1300us bis 1700us) */
        uint16_t pulse_us = 1300U + ((uint32_t)adc_value_read * (1700U - 1300U)) / 4085U;

        /* Impulsbreite im Register TIM4->CCR1 aktualisieren */
        driver_tim_pwm_set_compare(TIM4, TIM_CHANNEL_1, pulse_us);

        HAL_Delay(20);
    }
}
```

---

## 5. Build-Anleitung (Build Instructions)

### Voraussetzungen:
* **Toolchain**: `arm-none-eabi-gcc`
* **Build-System**: `CMake 3.22+` und `Ninja` oder `Make`

### Kompilierung über das Terminal:
```bash
# 1. Projekt mit CMake und Ninja konfigurieren
cmake -B build -G Ninja

# 2. Ausführbare Firmware-Datei (.elf) kompilieren
cmake --build build
```

---

## 6. Projektstruktur (Project Structure)

```text
Embedded_system/
├── CMakeLists.txt              # Haupt-CMake-Konfigurationsdatei
├── README.md                   # Technische Dokumentation (diese Datei)
├── Core/
│   ├── Inc/                    # System-Header-Dateien
│   └── Src/                    # main.c und Anwendungsquellcode
├── cmake/
│   └── stm32cubemx/            # CMSIS Device Header & Startup-Dateien
└── libs/
    └── STM32_driver/           # Bare-Metal Register-Treiberbibliothek
        ├── CMakeLists.txt      # CMake-Konfiguration für libSTM32_driver.a
        ├── include/            # Treiber-Header-Dateien (*.h)
        └── src/                # Treiber-Quellcode (*.c)
```
