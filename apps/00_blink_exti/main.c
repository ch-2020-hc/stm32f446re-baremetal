#include <stdint.h>
#include "rcc.h"
#include "gpio.h"
#include "exti.h"
#include "syscfg.h"
#include "stm32f446re.h"

uint32_t g_data = 0x12345678u;
uint32_t g_data_tail = 0x9ABCDEF0u;
uint32_t g_bss[4];
const uint32_t g_ro = 0xA5A5A5A5u;

volatile uint32_t g_result; // result of the checkpoint

volatile uint32_t g_exti_flag = 0; // flag to indicate that the EXTI line 13 is pending



void EXTI15_10_IRQHandler(void)
{
    // Check if EXTI_Line13 is pending
    if (EXTI->PR & (1U << 13)) {
        // EXTI_Line13 is pending
        // Clear the pending bit
        EXTI->PR = (1U << 13);
        g_exti_flag = 1;
    }
}

int main(void)
{
    /* Checkpoint before GPIO: add three file-scope objects and watch them after reset. */

    uint32_t result = 0;

    if (g_data == 0x12345678u && g_data_tail == 0x9ABCDEF0u)
        result |= 1u;
    if (g_bss[0] == 0u && g_bss[1] == 0u && g_bss[2] == 0u && g_bss[3] == 0u)
        result |= 2u;
    if (g_ro == 0xA5A5A5A5u && (uint32_t)&g_ro >= 0x08000000u)
        result |= 4u;

    g_result = result;

    rcc_init();    // Enable GPIO and SYSCFG clocks
    gpio_init();   // Set PA5 (LED Green LD2) to output and PC13 (button B1) to input and pull-up
    exti_init();   // Map PC13 to EXTI_Line13 and set Falling trigger selection

    while (1) {
        /* First implementation of the button press detection
        // Check if button (pull-up) is pressed
        if ((GPIOC->IDR & (1U << 13)) == 0) {
            // Button is pressed
            GPIOA->BSRR |= (1U << 5); // bit 5: bit set
        } else {
            // Button is not pressed
            GPIOA->BSRR |= (1U << 21); // bit 21: bit reset
        }
        */
        // Second implementation of the button press detection using EXTI
        if (g_exti_flag == 1) {
            // Toggle LED on PA5
            GPIOA->ODR ^= (1U << 5); // bit 5: bit toggle
            g_exti_flag = 0;
        }
    }
}