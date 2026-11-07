# STM32F446RE Bare-Metal

Register-level drivers in C for the STM32F446RE (ARM Cortex-M4 with FPU). Assembly is limited to startup and the vector table. Peripheral code writes the registers in [RM0390](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf). CMSIS-Core is used for the NVIC and the system control registers. ST HAL and LL are not used.

The board is a Nucleo-F446RE: 512 KB flash at `0x08000000`, 128 KB SRAM at `0x20000000`, onboard ST-Link.

The clock stays on the internal 16 MHz HSI until a later PWM measurement matches the timer formula. After reset the APB prescalers are 1, so TIM2 is also 16 MHz. An APB prescaler other than 1 doubles the timer clock, which is the usual reason a measured PWM frequency is twice the calculation.

## Status

Stage 0 is in the tree: the linker script, a full vector table, `.data` / `.bss` / `.rodata`, GPIO, and EXTI. PWM, SPI, and CAN are the labs that follow. Publish in that order and tag `v0.1-exti`, `v0.2-pwm`, `v0.3-dma`, `v0.4-spi`, `v0.5-can`.


| Step                       | App                                  | Extra hardware                                | What it shows                     |
| -------------------------- | ------------------------------------ | --------------------------------------------- | --------------------------------- |
| Stage 0, GPIO + EXTI       | `apps/00_blink_exti`                 | None                                          | Linker, vector table, `volatile`  |
| Project 1, PWM + ISR + DMA | `apps/01_pwm_led`, `apps/02_pwm_dma` | None for the LED; a 5 V servo is optional     | Timers, NVIC, DMA                 |
| Project 2, SPI × 3         | `apps/03_spi_sensor`                 | One SPI part (ADXL345, ICM-42688-P, or W25Q)  | Register struct, bus mode, DMA    |
| Project 3, CAN             | `apps/04_can_pwm_node`               | TJA1050 or SN65HVD230, 120 Ω, USB-CAN adapter | Bit timing, one filter, mailboxes |


Project 1 comes first because it runs on the Nucleo alone, and SPI and CAN reuse RCC, NVIC, and DMA. A logic analyzer matters more than the order of the later labs: a wrong `PSC` or SPI mode is invisible on the LED.

## Stage 0 — `apps/00_blink_exti`

Goal: after reset, C globals are valid, and the user button toggles LD2 through an interrupt. The CPU does not poll the pin.

Deliberately out of scope: timers, DMA, SPI, CAN, and any clock other than the 16 MHz HSI.

Pins:


| Signal | Pin             | Setup                                             |
| ------ | --------------- | ------------------------------------------------- |
| LD2    | PA5             | GPIO output                                       |
| B1     | PC13            | Input, pull-up. Released reads 1, pressed reads 0 |
| EXTI   | Line 13, port C | Falling edge, `EXTI15_10` enabled in the NVIC     |


`EXTI15_10_IRQHandler` clears the pending bit and sets `g_exti_flag`. `main` toggles PA5 when that flag is set. The flag is `volatile` because the handler and `main` share it.

Before GPIO init, `main` checks three file-scope objects and stores the result in `g_result`:


| Bit | Check                                             |
| --- | ------------------------------------------------- |
| 0   | `.data` initializers survived the copy from flash |
| 1   | `.bss` is zero                                    |
| 2   | a `const` object lives in flash (`>= 0x08000000`) |


`g_result == 7` means startup did all three. On the board, each press of B1 toggles LD2.

## Build

Tested with:

- Arm GNU Toolchain 15.3.Rel2 (`arm-none-eabi-gcc` 15.3.1)
- CMake 3.20 or newer (4.4.4 used here)
- Ninja

Flags are Cortex-M4, Thumb, FPv4-SP-D16, hard float, `-O0 -g3`, and `-nostdlib`. The linker script is `ld/stm32f446re.ld`.

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

Outputs are `build/firmware.elf` and `build/firmware.bin`. `arm-none-eabi-size` runs after the link.

## Flash

Use the onboard ST-Link. The image starts at `0x08000000`.

[stlink](https://github.com/stlink-org/stlink):

```sh
st-flash --reset write build/firmware.bin 0x08000000
```



## Layout

```text
stm32f446re-baremetal/
  README.md
  CMakeLists.txt
  cmake/arm-none-eabi.cmake
  ld/stm32f446re.ld              # 512 KB flash, 128 KB RAM, .data/.bss/.rodata
  startup/startup.s              # vector table (RM0390 Table 38) and Reset_Handler
  cmsis/                         # CMSIS-Core: NVIC, SCB, core_cm4.h
  drivers/
    include/   gpio.h  rcc.h  exti.h  syscfg.h
    src/       gpio.c  rcc.c  exti.c
  apps/
    00_blink_exti/main.c         # present
    01_pwm_led/                  # Project 1
    02_pwm_dma/
    03_spi_sensor/               # Project 2
    04_can_pwm_node/             # Project 3
```

Each app is a `main.c` plus the drivers it links. A driver header is the API. A driver source is the registers, with a comment pointing at the RM0390 section. Planned headers are `tim_pwm.h`, `dma.h`, `spi.h`, and `can.h`. Host tests for the PWM divider, CAN ID pack/unpack, and `BTR` belong in `tests/` so they run without a board.

`Reset_Handler` sets the stack to `_estack` (`0x20020000`), copies `.data` from flash, zeroes `.bss`, then calls `main`. Vector entries are weak aliases of `Default_Handler`. A C function with the same name (`EXTI15_10_IRQHandler`, `TIM2_IRQHandler`, …) replaces that slot at link time. The hardware stacks `R0–R3`, `R12`, `LR`, `PC`, and `xPSR` on entry. The NVIC itself stays in CMSIS-Core (`NVIC_EnableIRQ`).

## Formulas the later labs must match

PWM on PA5, which is TIM2 channel 1, alternate function 1. LD2 dimming is only a sanity check. The period is read with a logic analyzer on the Morpho or Arduino header.

```text
f_pwm = TIMCLK / ((PSC + 1) * (ARR + 1))
duty  = CCR / (ARR + 1)
```

At the 16 MHz HSI, with APB prescalers left at 1:

```text
16 MHz / ((0 + 1) * (15999 + 1)) = 1.000 kHz
```

The measured number goes in this file when Project 1 is on the bench, in the form `calculated 1.000 kHz, measured 1.000 kHz`. The same rule applies to the SPI baud divider and to CAN time quanta: calculate first, then show the capture.

The timer driver surface for that lab:

```c
void tim2_pwm_init(uint32_t tim_clk_hz, uint32_t pwm_hz);
void tim2_pwm_set_duty_permille(unsigned channel, unsigned duty);
```

`tim_pwm.c` is the only file that writes `PSC`, `ARR`, `CCMR`, and `CCER`.

CAN at 500 kbit/s does not divide cleanly from a raw 16 MHz HSI. That lab moves the clock tree once, writes down `SYSCLK`, `PCLK1`, and the time quanta, then programs `BTR`. Flash wait states stay at 0 while `SYSCLK` is 16 MHz. 180 MHz needs voltage scale 1, over-drive, and 5 wait states in `FLASH_ACR`, and only after the SPI mode is already proven.

## References

- STM32F446 reference manual, RM0390 (vector table Table 38, GPIO, EXTI, SYSCFG, TIM, DMA, SPI, bxCAN, RCC)
- CMSIS-Core for `NVIC_EnableIRQ` and the system control registers
- Register structs in this repo for GPIO, EXTI, and SYSCFG. TIM, DMA, SPI, and bxCAN follow the same pattern.

