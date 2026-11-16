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
