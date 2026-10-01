cmake_minimum_required(VERSION 3.22)
project(STM32_driver C)

add_library(STM32_driver STATIC
    src/driver_gpio.c
    src/driver_i2c.c
    src/driver_iwdg.c
    src/driver_adc.c
    src/driver_tim.c
    src/driver_dma.c
    src/driver_usart.c
    src/driver_interrupt.c
    src/driver_exti.c
    src/driver_spi.c
)

target_include_directories(STM32_driver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include
)

target_compile_options(STM32_driver PUBLIC -g -O0 -ffunction-sections -fdata-sections)
target_compile_definitions(STM32_driver PUBLIC STM32F103xE)
