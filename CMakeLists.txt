cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Embedded_system)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME}
        Core/Src/tetris_game/tetris_factory/tetris_factory.h
        Core/Src/tetris_game/tetris_factory/tetris_factory.c
        Core/Src/tetris_game/ICMax7219_driver/ICMax7219_driver.c
        Core/Src/tetris_game/ICMax7219_driver/ICMax7219_driver.h
        Core/Src/peripheral/spi/spi.c
        Core/Src/peripheral/spi/spi.h
        Core/Src/peripheral/gpio/gpio.c
        Core/Src/peripheral/gpio/gpio.h
        Core/Src/peripheral/rcc/rcc.c
        Core/Src/peripheral/rcc/rcc.h
        Core/Src/peripheral/tim/tim.c
        Core/Src/peripheral/tim/tim.h
        Core/Src/tetris_game/generate_piece_driver/pieces.c
        Core/Src/tetris_game/generate_piece_driver/pieces.h
        Core/Src/peripheral/i2c/i2c/i2c.c
        Core/Src/peripheral/i2c/i2c/i2c.h
        Core/Src/peripheral/i2c/lcd_config/lcd_config.c
        Core/Src/peripheral/i2c/lcd_config/lcd_config.h
        Core/Src/sensor_manager/dht11_driver/dht11.c
        Core/Src/sensor_manager/dht11_driver/dht11.h
        Core/Src/sensor_manager/hc-sr04-driver/hc_sr04.c
        Core/Src/sensor_manager/hc-sr04-driver/hc_sr04.h
        Core/Src/peripheral/adc/adc.c
        Core/Src/peripheral/adc/adc.h
        Core/Src/peripheral/usart/usart.c
        Core/Src/peripheral/usart/usart.h
        Core/Src/lora_transmit/lora_send/lora_send.c
        Core/Src/lora_transmit/lora_send/lora_send.h
        Core/Src/lora_transmit/lora_receive/lora_receive.c
        Core/Src/lora_transmit/lora_receive/lora_receive.h
        Core/Src/tetris_game/matrix_stack/matrix_stack.h
        Core/Src/tetris_game/matrix_stack/matrix_stack.c
        Core/Src/interrupt/interrupt.c
        Core/Src/interrupt/interrupt.h
        Core/Src/lora_transmit/lora_config/lora_config.c
        Core/Src/lora_transmit/lora_config/lora_config.h
        Core/Src/peripheral/rtc/rtc.c
        Core/Src/peripheral/rtc/rtc.h
        Core/Src/main.c
)

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Core/Src
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
