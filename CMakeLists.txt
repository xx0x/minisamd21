cmake_minimum_required(VERSION 3.13)

# Setup cross-compilation
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR cortex-m0plus)

set(CMAKE_C_COMPILER /opt/homebrew/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER /opt/homebrew/bin/arm-none-eabi-g++)
set(CMAKE_ASM_COMPILER /opt/homebrew/bin/arm-none-eabi-gcc)

set(CMAKE_OBJCOPY /opt/homebrew/bin/arm-none-eabi-objcopy)
set(CMAKE_SIZE /opt/homebrew/bin/arm-none-eabi-size)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# Add linker flag for memory usage
string(APPEND CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")

project(blink C CXX ASM)

# Define compiler flags
set(MCU_FLAGS
  -mcpu=cortex-m0plus
  -mthumb
  -O0
  -g
  -Wall
  -Werror
  -ffunction-sections
  -fno-exceptions
  -fdata-sections
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Source files
add_executable(blink
    src/main.cpp
    src/samd21_startup.c
    src/Pin.cpp
    src/System.cpp
)

# Add the .elf extension to the output file
set_target_properties(blink PROPERTIES OUTPUT_NAME "blink.elf")

# Add CMSIS and Atmel CMSIS include directories
target_include_directories(blink PRIVATE
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/platform/CMSIS/5.4.0/CMSIS/Core/Include
    ${CMAKE_SOURCE_DIR}/platform/CMSIS-Atmel/1.2.2/CMSIS/Device/ATMEL/samd21/include
)

target_compile_definitions(blink PRIVATE __SAMD21E18A__)

# Linker script
set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/platform/linker_scripts/SAMD21E18A_w_bootloader.ld)

target_compile_options(blink PRIVATE ${MCU_FLAGS})
target_link_options(blink PRIVATE ${MCU_FLAGS} -Wl,--gc-sections -T${LINKER_SCRIPT})

# Generate .bin after build
add_custom_command(TARGET blink POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:blink> $<TARGET_FILE_DIR:blink>/blink.bin
)

# Generate .hex after build
add_custom_command(TARGET blink POST_BUILD
  COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:blink> $<TARGET_FILE_DIR:blink>/blink.hex
)