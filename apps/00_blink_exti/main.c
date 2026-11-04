
/* Checkpoint, still before any GPIO: 
add three file-scope objects and watch them after reset. */

// g_data == 0x12345678, g_bss == 0, and g_ro sits in the 0x0800xxxx range. If g_data is 0, 
// the copy never ran or _sidata is wrong. If g_bss is garbage, the clear never ran.

#include <stdint.h>
#include "gpio.h"

uint32_t g_data = 0x12345678u;
uint32_t g_data_tail = 0x9ABCDEF0u;
uint32_t g_bss[4];
const uint32_t g_ro = 0xA5A5A5A5u;

volatile uint32_t g_result;

void gpio_init(void)
{
    // Enable GPIOA and GPIOC
    RCC_AHB1ENR |= (1U << 0) | (1U << 2); 
    // Force a read of RCC_AHB1ENR to wait for clock to be ready
    volatile uint32_t clock_ready = RCC_AHB1ENR;
    (void)clock_ready;

    // Set PA5 to output: bit 10&11 set to 1
    GPIOA->MODER &= ~(3U << 10);
    GPIOA->MODER |= (1U << 10); 

    // Set PC13 to input: bit 26&27 set to 0
    GPIOC->MODER &= ~(3U << 26); 
    // On the Nucleo-F446RE, B1 (the blue button) ties PC13 to ground 
    // while it is held. Released reads 1. Pressed reads 0.
    // Set PC13 to pull-up: bit 26&27 set to 1
    GPIOC->PUPDR &= ~(3U << 26);
    GPIOC->PUPDR |= (1U << 26); 
                             
}

int main(void)
{
    uint32_t result = 0;

    if (g_data == 0x12345678u && g_data_tail == 0x9ABCDEF0u)
        result |= 1u;
    if (g_bss[0] == 0u && g_bss[1] == 0u && g_bss[2] == 0u && g_bss[3] == 0u)
        result |= 2u;
    if (g_ro == 0xA5A5A5A5u && (uint32_t)&g_ro >= 0x08000000u)
        result |= 4u;

    g_result = result;

    gpio_init();

    while (1) {
        // Check if button (pull-up) is pressed
        if ((GPIOC->IDR & (1U << 13)) == 0) {
            // Button is pressed
            GPIOA->BSRR |= (1U << 5); // bit 5: bit set
        } else {
            // Button is not pressed
            GPIOA->BSRR |= (1U << 21); // bit 21: bit reset
        }
    }
}