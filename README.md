# STM32F446RE Bare-Metal

Register-level drivers in C for the STM32F446RE (ARM Cortex-M4 with FPU). Assembly is limited to startup and the vector table. Peripheral code writes the registers in [RM0390](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf). CMSIS-Core is used for the NVIC and the system control registers. ST HAL and LL are not used.

The board is a Nucleo-F446RE: 512 KB flash at `0x08000000`, 128 KB SRAM at `0x20000000`, onboard ST-Link.

## Why this project

The point is to practice the path from reset to a working peripheral, and to keep that path usable in a later application. Each lab is a small program on the same startup code, toolchain, and driver style.

- **MCU architecture**, on a Cortex-M4: the flash and SRAM map, how an exception stacks `R0–R3`, `R12`, `LR`, `PC`, and `xPSR`, how the NVIC reaches a handler, and how AHB and APB clocks gate each peripheral.
- **ARM assembly**, for the part C does not provide: the vector table and `Reset_Handler` in `startup/startup.s`. That file sets the stack, copies `.data`, zeroes `.bss`, and branches to `main`.
- **The CMake toolchain.** `cmake/arm-none-eabi.cmake` drives `arm-none-eabi-gcc`, and `add_app` builds one image per lab.
- **OpenOCD on the onboard ST-Link**, to flash an ELF and to debug it. The same interface is what a later board will use.
- **Register-level drivers and interrupt handlers.** GPIO, EXTI, TIM2, and DMA are programmed from RM0390: clock enable, pin or request routing, then the handler or the DMA stream. An ISR shares data with `main` through a `volatile` flag.
- **A base for a later application.** Startup, the build, and these drivers stay in the tree. A future program links the same files and adds the peripheral it needs, instead of starting again from a blank linker script.

The clock stays on the internal 16 MHz HSI. `apps/01_pwm_led` and `apps/02_pwm_dma` are both programmed for 1 kHz on that clock. A logic-analyzer capture is still the check that the pin matches the formula. After reset the APB prescalers are 1, so TIM2 is also 16 MHz. An APB prescaler other than 1 doubles the timer clock, which is the usual reason a measured PWM frequency is twice the calculation.

## Why these steps

This repository is one shared driver layer, built in the order the later labs depend on. Each app is a `main.c` on top of drivers the next app keeps. The finished tree should show a vector table and linker script written here, peripheral drivers that only touch registers, a PWM waveform that matches the timer math while the CPU sits in a DMA loop, the same SPI transfer done three ways, and a CAN frame that changes that PWM.

**Stage 0** comes first. Every later app needs the vector table, a linker script that copies `.data` and zeroes `.bss`, and a `volatile` flag shared with an interrupt handler. The Nucleo button and LD2 prove that path before a timer or a bus is involved.

**Project 1** is the first feature lab. On this board LD2 is PA5 and also TIM2 channel 1, so the PWM, the update interrupt, and the DMA ramp run with no extra parts. RCC, the NVIC, and DMA from that lab are the pieces SPI and CAN use again. A 5 V hobby servo is the optional motor step: same driver, 50 Hz, a 1–2 ms pulse.

**Project 2** is SPI, after the timer DMA works. One transfer is done three ways — polling, interrupt, then DMA — so the CPU cost can be compared. That is the timer-DMA lesson applied to a bus. It needs one SPI part on the bench (an accelerometer or a NOR flash, so the lab stays about the bus).

**Project 3** is CAN, after a transceiver and a second node are on the bench. The Nucleo has CAN pins and no transceiver. One board alone transmits, receives no ACK, and goes bus-off, which looks like a register bug until the wire is known. The lab that ties the repository together is one frame from the PC that calls `tim2_pwm_set_duty_cycle()` and changes the Project 1 waveform.

## Status

Stage 0 and Project 1 through the DMA ramp are in the tree. `apps/01_pwm_led` steps the 1 kHz duty in the TIM2 update interrupt. `apps/02_pwm_dma` writes the same ramp from a RAM table through DMA1 while `main` sits in the loop. The servo, SPI, and CAN are still ahead. Publish in that order and tag `v0.1-exti`, `v0.2-pwm`, `v0.3-dma`, `v0.4-spi`, `v0.5-can`.

| Step | App | Extra hardware | What it shows |
| --- | --- | --- | --- |
| Stage 0, GPIO + EXTI | `apps/00_blink_exti` | None | Linker, vector table, `volatile`. In the tree |
| Project 1, PWM + update ISR | `apps/01_pwm_led` | None | Timers, NVIC. In the tree |
| Project 1, DMA ramp | `apps/02_pwm_dma` | None | DMA. In the tree |
| Project 1, servo | same timer driver | Servo and 5 V | 50 Hz, 1–2 ms pulse. Optional, not built |
| Project 2, SPI × 3 | `apps/03_spi_sensor` | One SPI part (ADXL345, ICM-42688-P, or W25Q) | Register struct, bus mode, DMA |
| Project 3, CAN | `apps/04_can_pwm_node` | TJA1050 or SN65HVD230, 120 Ω, USB-CAN adapter | Bit timing, one filter, mailboxes |

A wrong `PSC` or SPI mode is invisible on the LED. The analyzer check matters for the 1 kHz image that is already built, and for every bus lab after it.

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

## Project 1 — `apps/01_pwm_led`

Goal: a 1 kHz PWM on LD2, with the duty moving in the TIM2 update interrupt while `main` spins. The CPU does not bit-bang the pin.

This image stops at the update interrupt. DMA, a 50 Hz servo, SPI, CAN, and any clock other than the 16 MHz HSI are out of scope here.

| Signal | Pin | Setup |
| --- | --- | --- |
| LD2 / TIM2_CH1 | PA5 | Alternate function 1, via `gpio_tim2_ch1_pa5()` |

`gpio_init()` leaves PA5 as a GPIO output for Stage 0. The PWM image calls `gpio_tim2_ch1_pa5()` instead. Output mode does not connect the timer to the pin.

```text
f_pwm = TIMCLK / ((PSC + 1) * (ARR + 1))
duty  = CCR / (ARR + 1)
```

`tim2_pwm_init(0, 15999)` programs that directly. With TIMCLK at 16 MHz:

```text
16 MHz / ((0 + 1) * (15999 + 1)) = 1.000 kHz
```

`tim2_pwm_set_duty_cycle` takes a duty in permille, 0 to 1000, and writes `CCR = duty * (ARR + 1) / 1000`. Channel 1 is the LD2 channel.

`tim2_pwm_init` leaves the counter stopped (`CEN` = 0). `main` enables the update interrupt and `TIM2_IRQn`, then sets `CEN`. `TIM2_IRQHandler` runs once per 1 ms period, clears the update flag, and steps the permille by 1. The ramp goes 0 → 1000 → 0, so one bright/dim cycle is 2 s. `main` then stays in the loop. LD2 brightening and dimming is only a sanity check. The period still needs a logic analyzer on PA5 (Morpho or Arduino header). Put the capture next to the formula when it exists: `calculated 1.000 kHz, measured …`.

```c
void tim2_pwm_init(uint32_t psc, uint32_t arr);
void tim2_pwm_set_duty_cycle(uint8_t channel, uint32_t duty_cycle);
```

The register struct is `drivers/include/tim2_5.h`. `drivers/src/tim_pwm.c` is the only file that writes `PSC`, `ARR`, `CCMR1`, and `CCER`. `rcc_init()` enables the TIM2 clock on APB1.

## Project 1 — `apps/02_pwm_dma`

Goal: the same 1 kHz PWM and the same 2 s bright/dim ramp, with DMA writing `CCR1` and `main` idle. This image does not enable `TIM2_IRQn`.

The pin, the formula, and `tim2_pwm_init(0, 15999)` are the ones in `apps/01_pwm_led`. A 5 V servo, SPI, CAN, and any clock other than the 16 MHz HSI are out of scope here.

`dma_tim2_init()` fills a 2000-word table in RAM and points DMA1 stream 1, channel 3, at it. Channel 3 is the TIM2 update request on that stream. Each update copies one `uint32_t` from the table to `TIM2->CCR1`. The memory address increments, the peripheral address stays on `CCR1`, both sides are 32-bit, and circular mode reloads the transfer count when the table ends.

The table stores compare counts, not permille. With `ARR` = 15999, one permille is `(ARR + 1) / 1000` = 16 counts. Entries 0–999 ramp from 16 to 16000. Entries 1000–1999 ramp from 15984 down to 0. One entry per 1 ms period is again a 2 s cycle.

`tim2_pwm_init` leaves `CEN` at 0. `main` then calls `dma_tim2_init`, sets the update DMA request (`UDE` in `TIM2->DIER`), and only then sets `CEN`. A TIM2 update that arrives before the stream is enabled is discarded.

```c
void dma_tim2_init(void);
```

The register struct is `drivers/include/dma.h`. `drivers/src/dma.c` is the only file that writes the DMA1 stream 1 registers. `rcc_init()` enables the DMA1 clock on AHB1. LD2 breathing is the sanity check. The 1 kHz period is still the analyzer measurement: `calculated 1.000 kHz, measured …`.

## Build

Tested with:

- Arm GNU Toolchain 15.3.Rel2 (`arm-none-eabi-gcc` 15.3.1)
- CMake 3.20 or newer (4.4.4 used here)
- Ninja

Flags are Cortex-M4, Thumb, FPv4-SP-D16, hard float, `-O0 -g3`, and `-nostdlib`. The linker script is `ld/stm32f446re.ld`.

`CMakeLists.txt` builds one image per app through `add_app`. Every image links `startup/startup.s`, `drivers/src/rcc.c`, and `drivers/src/gpio.c`. Stage 0 adds `exti.c` and `apps/00_blink_exti/main.c`. The PWM image adds `tim_pwm.c` and `apps/01_pwm_led/main.c`. The DMA image adds `tim_pwm.c`, `dma.c`, and `apps/02_pwm_dma/main.c`. Each new app is another image. The earlier ones stay in the same build.

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

`arm-none-eabi-size` runs after each link.

| App | ELF | BIN |
| --- | --- | --- |
| Stage 0, GPIO + EXTI | `build/00_blink_exti.elf` | `build/00_blink_exti.bin` |
| Project 1, PWM + interrupt | `build/01_pwm_led.elf` | `build/01_pwm_led.bin` |
| Project 1, DMA ramp | `build/02_pwm_dma.elf` | `build/02_pwm_dma.bin` |

## Flash

Use the onboard ST-Link. Each image starts at `0x08000000`, so flash one at a time.

OpenOCD programs the ELF, verifies it, resets the target, and exits:

```sh
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program build/02_pwm_dma.elf verify reset exit"
```

Use `build/01_pwm_led.elf` for the interrupt ramp, or `build/00_blink_exti.elf` for Stage 0.

[stlink](https://github.com/stlink-org/stlink) writes the raw binary to the same address:

```sh
st-flash --reset write build/02_pwm_dma.bin 0x08000000
```

Use `build/01_pwm_led.bin` or `build/00_blink_exti.bin` for the other two images.



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
    include/   gpio.h  rcc.h  exti.h  syscfg.h  tim2_5.h  dma.h
    src/       gpio.c  rcc.c  exti.c  tim_pwm.c  dma.c
  apps/
    00_blink_exti/main.c         # Stage 0
    01_pwm_led/main.c            # Project 1, PWM + update ISR
    02_pwm_dma/main.c            # Project 1, DMA ramp
    03_spi_sensor/               # Project 2
    04_can_pwm_node/             # Project 3
```

Each app is a `main.c` plus the drivers `add_app` links for it. A driver header is the API. A driver source is the registers. Planned headers are `spi.h` and `can.h`. Host tests for the PWM divider, CAN ID pack/unpack, and `BTR` belong in `tests/` so they run without a board.

`Reset_Handler` sets the stack to `_estack` (`0x20020000`), copies `.data` from flash, zeroes `.bss`, then calls `main`. Vector entries are weak aliases of `Default_Handler`. A C function with the same name (`EXTI15_10_IRQHandler`, `TIM2_IRQHandler`, …) replaces that slot at link time. The hardware stacks `R0–R3`, `R12`, `LR`, `PC`, and `xPSR` on entry. The NVIC itself stays in CMSIS-Core (`NVIC_EnableIRQ`).

## What is still ahead

The PWM formula, the update interrupt, and the DMA ramp are in Project 1 above. The analyzer number for that 1 kHz pin is still open. The same rule applies to the next labs: calculate first, then show the capture.

The optional servo is the same timer driver at 50 Hz, with a pulse of about 1–2 ms, on a 5 V supply that shares ground with the Nucleo. SPI is next after that: one transfer, three ways. CAN follows the transceiver.

CAN at 500 kbit/s does not divide cleanly from a raw 16 MHz HSI. That lab moves the clock tree once, writes down `SYSCLK`, `PCLK1`, and the time quanta, then programs `BTR`. Flash wait states stay at 0 while `SYSCLK` is 16 MHz. 180 MHz needs voltage scale 1, over-drive, and 5 wait states in `FLASH_ACR`, and only after the SPI mode is already proven.

## References

- STM32F446 reference manual, RM0390 (vector table Table 38, GPIO, EXTI, SYSCFG, TIM, DMA, SPI, bxCAN, RCC)
- CMSIS-Core for `NVIC_EnableIRQ` and the system control registers
- Register structs in this repo for GPIO, EXTI, SYSCFG, TIM2/TIM5 (`tim2_5.h`), and DMA1 (`dma.h`). SPI and bxCAN follow the same pattern.

