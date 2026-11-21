# STM32F446RE Bare-Metal

Register-level drivers in C for the STM32F446RE (ARM Cortex-M4 with FPU). Assembly is limited to startup and the vector table. Peripheral code writes the registers in [RM0390](https://www.st.com/resource/en/reference_manual/rm0390-stm32f446xx-advanced-armbased-32bit-mcus-stmicroelectronics.pdf). CMSIS-Core is used for the NVIC and the system control registers. ST HAL and LL are not used.

The board is a Nucleo-F446RE: 512 KB flash at `0x08000000`, 128 KB SRAM at `0x20000000`, onboard ST-Link.

## Why this project

The point is to practice the path from reset to a working peripheral, and to keep that path usable in a later application. Each lab is a small program on the same startup code, toolchain, and driver style.

- **MCU architecture**, on a Cortex-M4: the flash and SRAM map, how an exception stacks `R0–R3`, `R12`, `LR`, `PC`, and `xPSR`, how the NVIC reaches a handler, and how AHB and APB clocks gate each peripheral.
- **ARM assembly**, for the part C does not provide: the vector table and `Reset_Handler` in `startup/startup.s`. That file sets the stack, copies `.data`, zeroes `.bss`, and branches to `main`.
- **The CMake toolchain.** `cmake/arm-none-eabi.cmake` drives `arm-none-eabi-gcc`, and `add_app` builds one image per lab.
- **OpenOCD on the onboard ST-Link**, to flash an ELF and to debug it. The same interface is what a later board will use.
- **Register-level drivers and interrupt handlers.** GPIO, EXTI, TIM2, DMA, and SPI are programmed from RM0390: clock enable, pin or request routing, then the handler, the DMA stream, or a polled status bit. An ISR shares data with `main` through a `volatile` flag.
- **A base for a later application.** Startup, the build, and these drivers stay in the tree. A future program links the same files and adds the peripheral it needs, instead of starting again from a blank linker script.

The clock stays on the internal 16 MHz HSI. `apps/01_pwm_led` and `apps/02_pwm_dma` are both programmed for 1 kHz on that clock. A logic analyzer has matched `apps/01_pwm_led`: the period is 1.000 kHz, and the update interrupt ramps the duty. The DMA image uses the same timer setup and has not had its own capture. `apps/03_spi_sensor` uses that same 16 MHz as the SPI1 kernel clock, and a logic analyzer has already matched its SCK to the formula below. After reset the APB prescalers are 1, so TIM2 and SPI1 are also 16 MHz. An APB prescaler other than 1 doubles the timer clock, which is the usual reason a measured PWM frequency is twice the calculation.

## Why these steps

This repository is one shared driver layer, built in the order the later labs depend on. Each app is a `main.c` on top of drivers the next app keeps. The finished tree should show a vector table and linker script written here, peripheral drivers that only touch registers, a PWM waveform that matches the timer math while the CPU sits in a DMA loop, the same SPI transfer done three ways, and a CAN frame that changes that PWM.

**Stage 0** comes first. Every later app needs the vector table, a linker script that copies `.data` and zeroes `.bss`, and a `volatile` flag shared with an interrupt handler. The Nucleo button and LD2 prove that path before a timer or a bus is involved.

**Project 1** is the first feature lab. On this board LD2 is PA5 and also TIM2 channel 1, so the PWM, the update interrupt, and the DMA ramp run with no extra parts. RCC, the NVIC, and DMA from that lab are the pieces SPI and CAN use again. A 5 V hobby servo is the optional motor step: same driver, 50 Hz, a 1–2 ms pulse.

**Project 2** is SPI, after the timer DMA works. One transfer is done three ways — polling, interrupt, then DMA — so the CPU cost can be compared. That is the timer-DMA lesson applied to a bus. It needs one SPI part on the bench (an accelerometer or a NOR flash, so the lab stays about the bus).

**Project 3** is CAN, after a transceiver and a second node are on the bench. The Nucleo has CAN pins and no transceiver. One board alone transmits, receives no ACK, and goes bus-off, which looks like a register bug until the wire is known. The lab that ties the repository together is one frame from the PC that calls `tim2_pwm_set_duty_cycle()` and changes the Project 1 waveform.

## Status

Stage 0, Project 1 through the DMA ramp, and the polling half of Project 2 are in the tree. `apps/01_pwm_led` steps the 1 kHz duty in the TIM2 update interrupt. `apps/02_pwm_dma` writes the same ramp from a RAM table through DMA1 while `main` sits in the loop. `apps/03_spi_sensor` reads an ADXL345 WHO_AM_I register on SPI1 by polling `TXE` and `RXNE`. The SPI interrupt and DMA styles, the servo, and CAN are still ahead. Publish in that order and tag `v0.1-exti`, `v0.2-pwm`, `v0.3-dma`, `v0.4-spi`, `v0.5-can`.


| Step                             | App                    | Extra hardware                                | What it shows                                 |
| -------------------------------- | ---------------------- | --------------------------------------------- | --------------------------------------------- |
| Stage 0, GPIO + EXTI             | `apps/00_blink_exti`   | None                                          | Linker, vector table, `volatile`. In the tree |
| Project 1, PWM + update ISR      | `apps/01_pwm_led`      | None                                          | Timers, NVIC. In the tree                     |
| Project 1, DMA ramp              | `apps/02_pwm_dma`      | None                                          | DMA. In the tree                              |
| Project 1, servo                 | same timer driver      | Servo and 5 V                                 | 50 Hz, 1–2 ms pulse. Optional, not built      |
| Project 2, SPI polling           | `apps/03_spi_sensor`   | ADXL345, or a stand-in on the same pins       | SPI1 master, mode 3, WHO_AM_I. In the tree    |
| Project 2, SPI interrupt and DMA | same bus               | same part                                     | TXE/RXNE state machine, then DMA. Not built   |
| Project 3, CAN                   | `apps/04_can_pwm_node` | TJA1050 or SN65HVD230, 120 Ω, USB-CAN adapter | Bit timing, one filter, mailboxes             |


A wrong `PSC` or SPI mode is invisible on the LED. `apps/01_pwm_led` has had the analyzer check: period measured 1.000 kHz, and the update interrupt moves the duty across a 2 s cycle. The DMA image has not. The SPI polling image has: SCK measured 1.000 MHz, mode 3, and the two-byte frame matched the program.

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


| Signal         | Pin | Setup                                           |
| -------------- | --- | ----------------------------------------------- |
| LD2 / TIM2_CH1 | PA5 | Alternate function 1, via `gpio_tim2_ch1_pa5()` |


`gpio_init()` leaves PA5 as a GPIO output for Stage 0. The PWM image calls `gpio_tim2_ch1_pa5()` instead. Output mode does not connect the timer to the pin.

```text
f_pwm = TIMCLK / ((PSC + 1) * (ARR + 1))
duty  = CCR / (ARR + 1)
```

`tim2_pwm_init(0, 15999)` programs that directly. With TIMCLK at 16 MHz:

```text
16 MHz / ((0 + 1) * (15999 + 1)) = 1.000 kHz, measured 1.000 kHz
```

A logic analyzer on PA5 confirmed that period. `tim2_pwm_set_duty_cycle` takes a duty in permille, 0 to 1000, and writes `CCR = duty * (ARR + 1) / 1000`. Channel 1 is the LD2 channel.

`tim2_pwm_init` leaves the counter stopped (`CEN` = 0). `main` enables the update interrupt and `TIM2_IRQn`, then sets `CEN`. `TIM2_IRQHandler` runs once per 1 ms period, clears the update flag, and steps the permille by 1. The ramp goes 0 → 1000 → 0, so one bright/dim cycle is 2 s. The same capture showed that duty moving while `main` stayed in the loop. LD2 brightening and dimming is only a sanity check.

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

## Project 2 — `apps/03_spi_sensor`

Goal: read the ADXL345 WHO_AM_I register with SPI1 as master, polling `TXE` and `RXNE`. `main` stays inside `spi_exchange` until both bytes have shifted.

This image is the polling style. The `TXE`/`RXNE` state machine and the DMA double buffer are still ahead. Timers, the servo, and CAN are out of scope here.


| Signal    | Pin | Setup                                                         |
| --------- | --- | ------------------------------------------------------------- |
| SPI1_SCK  | PA5 | Alternate function 5. PA5 is also LD2, so the LED follows SCK |
| SPI1_MISO | PA6 | Alternate function 5, pull-up                                 |
| SPI1_MOSI | PA7 | Alternate function 5                                          |
| CS        | PB6 | GPIO output, active low, idle high                            |


PB6 is software chip-select. SPI1 hardware NSS is PA4 or PA15, so this image sets `SSM` and `SSI` and drives PB6 from GPIO. `gpio_spi1_pa5_pa6_pa7()` replaces the TIM2 alternate function on PA5. This image does not call `gpio_tim2_ch1_pa5()`.

```text
f_sck = f_PCLK2 / 2^(BR + 1)
```

`spi_init` sets `BR` = 3. With PCLK2 at 16 MHz:

```text
16 MHz / 2^(3 + 1) = 1.000 MHz, measured 1.000 MHz
```

A logic analyzer on SCK and MOSI confirmed that capture. The frame is mode 3 (`CPOL` = 1, `CPHA` = 1), 8-bit, MSB first, and the two bytes on MOSI are `{0x80, 0x00}`. `SPE` is set last, after those bits. `spi_exchange` drops CS, shifts each byte, and raises CS only after `BSY` clears.

Those two bytes are the ADXL345 WHO_AM_I read: register 0x00 with the read bit set, then a dummy byte that clocks the ID out. `g_whoami` is `g_rx[1]`. A real ADXL345 returns `0xE5`.

```c
void spi_init(void);
uint8_t spi_exchange_byte(uint8_t tx);
void spi_exchange(const uint8_t *tx, volatile uint8_t *rx, uint16_t len);
```

The register struct is `drivers/include/spi.h`. `drivers/src/spi.c` is the only file that writes `CR1`, `SR`, and `DR`. `rcc_spi1_init()` enables SPI1 on APB2 and the GPIOA and GPIOB clocks. `SPI1_IRQn` is in the vector enum and is not enabled here.

## Build

Tested with:

- Arm GNU Toolchain 15.3.Rel2 (`arm-none-eabi-gcc` 15.3.1)
- CMake 3.20 or newer (4.4.4 used here)
- Ninja

Flags are Cortex-M4, Thumb, FPv4-SP-D16, hard float, `-O0 -g3`, and `-nostdlib`. The linker script is `ld/stm32f446re.ld`.

`CMakeLists.txt` builds one image per app through `add_app`. Every image links `startup/startup.s`, `drivers/src/rcc.c`, and `drivers/src/gpio.c`. Stage 0 adds `exti.c` and `apps/00_blink_exti/main.c`. The PWM image adds `tim_pwm.c` and `apps/01_pwm_led/main.c`. The DMA image adds `tim_pwm.c`, `dma.c`, and `apps/02_pwm_dma/main.c`. The SPI image adds `spi.c` and `apps/03_spi_sensor/main.c`. Each new app is another image. The earlier ones stay in the same build.

```sh
cmake -S . -B build -G Ninja
cmake --build build
```

`arm-none-eabi-size` runs after each link.


| App                        | ELF                       | BIN                       |
| -------------------------- | ------------------------- | ------------------------- |
| Stage 0, GPIO + EXTI       | `build/00_blink_exti.elf` | `build/00_blink_exti.bin` |
| Project 1, PWM + interrupt | `build/01_pwm_led.elf`    | `build/01_pwm_led.bin`    |
| Project 1, DMA ramp        | `build/02_pwm_dma.elf`    | `build/02_pwm_dma.bin`    |
| Project 2, SPI polling     | `build/03_spi_sensor.elf` | `build/03_spi_sensor.bin` |




## Flash

Use the onboard ST-Link. Each image starts at `0x08000000`, so flash one at a time.

OpenOCD programs the ELF, verifies it, resets the target, and exits:

```sh
openocd -f interface/stlink.cfg -f target/stm32f4x.cfg -c "program build/02_pwm_dma.elf verify reset exit"
```

Use `build/02_pwm_dma.elf` for the DMA ramp, `build/01_pwm_led.elf` for the interrupt ramp, or `build/00_blink_exti.elf` for Stage 0.

[stlink](https://github.com/stlink-org/stlink) writes the raw binary to the same address:

```sh
st-flash --reset write build/02_pwm_dma.bin 0x08000000
```

Use `build/02_pwm_dma.bin`, `build/01_pwm_led.bin`, or `build/00_blink_exti.bin` for the other images.

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
    include/   gpio.h  rcc.h  exti.h  syscfg.h  tim2_5.h  dma.h  spi.h
    src/       gpio.c  rcc.c  exti.c  tim_pwm.c  dma.c  spi.c
  apps/
    00_blink_exti/main.c         # Stage 0
    01_pwm_led/main.c            # Project 1, PWM + update ISR
    02_pwm_dma/main.c            # Project 1, DMA ramp
    03_spi_sensor/main.c         # Project 2, SPI polling
    04_can_pwm_node/             # Project 3
```

Each app is a `main.c` plus the drivers `add_app` links for it. A driver header is the API. A driver source is the registers. The planned header is `can.h`. Host tests for the PWM divider, CAN ID pack/unpack, and `BTR` belong in `tests/` so they run without a board.

`Reset_Handler` sets the stack to `_estack` (`0x20020000`), copies `.data` from flash, zeroes `.bss`, then calls `main`. Vector entries are weak aliases of `Default_Handler`. A C function with the same name (`EXTI15_10_IRQHandler`, `TIM2_IRQHandler`, …) replaces that slot at link time. The hardware stacks `R0–R3`, `R12`, `LR`, `PC`, and `xPSR` on entry. The NVIC itself stays in CMSIS-Core (`NVIC_EnableIRQ`).

## What is still ahead

The PWM formula and the update interrupt are in `apps/01_pwm_led`, and a logic analyzer matched both: 1.000 kHz, and the duty ramp over 2 s. The SPI polling transfer is in Project 2 above, and its capture matches the 1.000 MHz calculation. The DMA image in `apps/02_pwm_dma` uses the same timer setup and still needs its own capture. The same rule applies to the next labs: calculate first, then show the capture.

The other two SPI styles are still ahead on this same bus: a `TXE`/`RXNE` state machine that wakes `main` with a `volatile` done flag, then a DMA transfer into a double buffer. The optional servo is the same timer driver at 50 Hz, with a pulse of about 1–2 ms, on a 5 V supply that shares ground with the Nucleo. CAN follows the transceiver.

CAN at 500 kbit/s does not divide cleanly from a raw 16 MHz HSI. That lab moves the clock tree once, writes down `SYSCLK`, `PCLK1`, and the time quanta, then programs `BTR`. Flash wait states stay at 0 while `SYSCLK` is 16 MHz. 180 MHz needs voltage scale 1, over-drive, and 5 wait states in `FLASH_ACR`, and only after the SPI mode is already proven.

## References

- STM32F446 reference manual, RM0390 (vector table Table 38, GPIO, EXTI, SYSCFG, TIM, DMA, SPI, bxCAN, RCC)
- CMSIS-Core for `NVIC_EnableIRQ` and the system control registers
- Register structs in this repo for GPIO, EXTI, SYSCFG, TIM2/TIM5 (`tim2_5.h`), DMA1 (`dma.h`), and SPI1 (`spi.h`). bxCAN follows the same pattern.

