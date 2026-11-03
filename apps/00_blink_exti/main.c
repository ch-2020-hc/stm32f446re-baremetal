
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
    RCC_AHB1ENR |= 1U << 0 | 1U << 2; // Enable GPIOA and GPIOC


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

    while (1) {
    }
}