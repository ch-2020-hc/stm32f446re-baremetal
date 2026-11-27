#include "gpio.h"

/*
 * Stage 0. PA5 is a GPIO output for LD2, and PC13 is the button input.
 * Call rcc_init() first so GPIOA and GPIOC are clocked.
 */
void gpio_init(void)
{
    /* MODER[11:10] = 01: PA5 general-purpose output. */
    GPIOA->MODER &= ~(3U << 10);
    GPIOA->MODER |= (1U << 10);

    /* MODER[27:26] = 00: PC13 input.
     * On the Nucleo-F446RE, B1 ties PC13 to ground while it is held.
     * Released reads 1. Pressed reads 0.
     * PUPDR[27:26] = 01: pull-up.
     */
    GPIOC->MODER &= ~(3U << 26);
    GPIOC->PUPDR &= ~(3U << 26);
    GPIOC->PUPDR |= (1U << 26);
}

/*
 * Project 1. PA5 is TIM2 channel 1, alternate function 1, which drives LD2.
 * This replaces the GPIO-output setting from gpio_init(). Do not call both
 * on PA5: MODER output mode does not connect the timer to the pin.
 */
void gpio_tim2_ch1_pa5(void)
{
    /* MODER[11:10] = 10: alternate function. 11 would be analog mode. */
    GPIOA->MODER &= ~(3U << 10);
    GPIOA->MODER |= (2U << 10);

    /* AFRL[23:20] = 0001: AF1, TIM2_CH1. */
    GPIOA->AFRL &= ~(15U << 20);
    GPIOA->AFRL |= (1U << 20);
}

/*
 * Project 2. PA5, PA6, and PA7 are SPI1_SCK, SPI1_MISO, and SPI1_MOSI, alternate function 5.
 * PA5 is also LD2, so the LED follows SCK during a transfer.
 */
void gpio_spi1_pa5_pa6_pa7(void)
{
    /* MODER[11:10] = 10: PA5 alternate function. */
    GPIOA->MODER &= ~(3U << 10);
    GPIOA->MODER |= (2U << 10);
    /* AFRL[23:20] = 0101: AF5, SPI1_SCK. */
    GPIOA->AFRL &= ~(15U << 20);
    GPIOA->AFRL |= (5U << 20);
    /* OTYPER bit 5 = 0: push-pull. */
    GPIOA->OTYPER &= ~(1U << 5);
    /* OSPEEDR[11:10] = 11: very high speed. 10 would be high speed. */
    GPIOA->OSPEEDR |= (3U << 10);

    /* MODER[13:12] = 10: PA6 alternate function. */
    GPIOA->MODER &= ~(3U << 12);
    GPIOA->MODER |= (2U << 12);
    /* AFRL[27:24] = 0101: AF5, SPI1_MISO. */
    GPIOA->AFRL &= ~(15U << 24);
    GPIOA->AFRL |= (5U << 24);
    /* PUPDR[13:12] = 01: pull-up. 11 is reserved, not a pull. */
    GPIOA->PUPDR &= ~(3U << 12);
    GPIOA->PUPDR |= (1U << 12);

    /* MODER[15:14] = 10: PA7 alternate function. */
    GPIOA->MODER &= ~(3U << 14);
    GPIOA->MODER |= (2U << 14);
    /* AFRL[31:28] = 0101: AF5, SPI1_MOSI. */
    GPIOA->AFRL &= ~(15U << 28);
    GPIOA->AFRL |= (5U << 28);
    /* OTYPER bit 7 = 0: push-pull. */
    GPIOA->OTYPER &= ~(1U << 7);
    /* OSPEEDR[15:14] = 11: very high speed. */
    GPIOA->OSPEEDR |= (3U << 14);
}

/*
 * Project 2. PB6 is the active-low chip select, as a GPIO output.
 * SPI1 hardware NSS is PA4 or PA15, so this pin stays under software control.
 */
void gpio_spi1_pb6(void)
{
    /*
     * BS6 is BSRR bit 6. Writing 1 drives PB6 high before MODER enables the
     * output, so CS does not glitch low. BSRR avoids a read-modify-write of ODR.
     */
    GPIOB->BSRR = (1U << 6);

    /* MODER[13:12] = 01: PB6 general-purpose output. */
    GPIOB->MODER &= ~(3U << 12);
    GPIOB->MODER |= (1U << 12);
    /* OTYPER bit 6 = 0: push-pull. */
    GPIOB->OTYPER &= ~(1U << 6);
    /* OSPEEDR[13:12] = 11: very high speed. */
    GPIOB->OSPEEDR |= (3U << 12);
}

/*
 * Project 3. PB8 and PB9 are CAN1_RX and CAN1_TX, alternate function 9.
 */
void gpio_can1_pb8_pb9(void)
{
    GPIOB->MODER &= ~(15U << 16);
    GPIOB->MODER |= (2U << 16) | (2U << 18);

    /* PB8 is CAN1_RX. */
    /* AFRH[3:0] = 1001: AF9, CAN1_RX. */
    GPIOB->AFRH &= ~(15U << 0);
    GPIOB->AFRH |= (9U << 0);
    /* OTYPER bit 8 = 0: open-drain. */
    GPIOB->OTYPER |= (1U << 8);
    /* OSPEEDR[17:16] = 11: very high speed. */
    GPIOB->OSPEEDR |= (3U << 16);
    /* PUPDR[17:16] = 01: pull-up, make Rx recessive when not connected. */
    GPIOB->PUPDR &= ~(3U << 16);
    GPIOB->PUPDR |= (1U << 16);

    /* PB9 is CAN1_TX. */
    /* AFRH[7:4] = 1001: AF9, CAN1_TX. */
    GPIOB->AFRH &= ~(15U << 4);
    GPIOB->AFRH |= (9U << 4);
    /* OTYPER bit 9 = 0: push-pull. */
    GPIOB->OTYPER &= ~(1U << 9);
    /* OSPEEDR[19:18] = 11: very high speed. */
    GPIOB->OSPEEDR |= (3U << 18);
}
