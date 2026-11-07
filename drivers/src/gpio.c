#include "gpio.h"


void gpio_init(void)
{
    // GPIO clock must be enabled first by calling rcc_init() in main.c

    // Set PA5 to output: bit 10 set to 1, bit 11 set to 0
    GPIOA->MODER &= ~(3U << 10);
    GPIOA->MODER |= (1U << 10); 

    
    // Set PC13 to input: bit 26&27 set to 0
    GPIOC->MODER &= ~(3U << 26); 
    // On the Nucleo-F446RE, B1 (the blue button) ties PC13 to ground 
    // while it is held. Released reads 1. Pressed reads 0.
    // Set PC13 to pull-up: bit 26 set to 1, bit 27 set to 0
    GPIOC->PUPDR &= ~(3U << 26); // bit 26&27 set to 0
    GPIOC->PUPDR |= (1U << 26); // bit 26 set to 1
    
}