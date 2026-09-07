cmake_minimum_required(VERSION 3.22)
project(STM32_driver C)

add_library(STM32_driver STATIC
    src/driver_gpio.c
    src/driver_i2c.c
    src/driver_iwdg.c
)

target_include_directories(STM32_driver PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Device/ST/STM32F1xx/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../Drivers/CMSIS/Include
)

target_compile_options(STM32_driver PUBLIC -g -O0)
target_compile_definitions(STM32_driver PUBLIC STM32F103xB)
