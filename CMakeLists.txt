cmake_minimum_required(VERSION 3.20)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cmake/arm-none-eabi.cmake)

project(STM32F446RE_BAREMETAL C ASM)

# Compiler Flags
set(CPU_FLAGS -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard)

add_compile_options(${CPU_FLAGS} -Wall -O0 -g3)
add_link_options(${CPU_FLAGS} -T${CMAKE_CURRENT_SOURCE_DIR}/ld/stm32f446re.ld -nostdlib)

# One image per app. Stage 0 stays buildable after Project 1 is added.
set(COMMON_SOURCES
    startup/startup.s
    drivers/src/rcc.c
    drivers/src/gpio.c
)

function(add_app name)
    add_executable(${name}.elf ${COMMON_SOURCES} ${ARGN})
    target_include_directories(${name}.elf PRIVATE drivers/include cmsis)
    add_custom_command(TARGET ${name}.elf POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary ${name}.elf ${name}.bin
        COMMAND ${CMAKE_SIZE} ${name}.elf
        VERBATIM
    )
endfunction()

add_app(00_blink_exti
    drivers/src/exti.c
    apps/00_blink_exti/main.c
)

add_app(01_pwm_led
    drivers/src/tim_pwm.c
    apps/01_pwm_led/main.c
)
