cmake_minimum_required(VERSION 3.22)
project(STM32_driver C)

add_library(STM32_driver STATIC IMPORTED GLOBAL)

set_target_properties(STM32_driver PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libstm32_arm_register_driver.a"
)

target_include_directories(STM32_driver INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include")
