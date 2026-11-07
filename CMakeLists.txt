cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-none-eabi.cmake)

project(STM32F446RE_BAREMETAL C ASM)


# Compiler Flags
set(CPU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

add_compile_options(${CPU_FLAGS} -Wall -O0 -g3)
add_link_options(${CPU_FLAGS} -T${CMAKE_CURRENT_SOURCE_DIR}/ld/stm32f446re.ld -nostdlib)

# Source Code
add_executable(firmware.elf
    startup/startup.s
    drivers/src/rcc.c
    drivers/src/gpio.c
    drivers/src/exti.c
    apps/00_blink_exti/main.c

)

target_include_directories(firmware.elf PRIVATE drivers/include cmsis)

# Generate .bin file and size info
add_custom_command(TARGET firmware.elf POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary firmware.elf firmware.bin
    COMMAND ${CMAKE_SIZE} firmware.elf
)