#include "exti.h"
#include "syscfg.h"
#include "stm32f446re.h"


void exti_init(void)
{
    // SYSCFG clock must be enabled first by calling rcc_init() in main.c

    // Map PC13 to EXTI_Line13
    SYSCFG->EXTICR4 |= (2U << 4); // port C 0010, EXTI_Line13

    // Set Falling trigger selection for EXTI_Line13
    EXTI->FTSR |= (1U << 13); // bit 13: 1: Falling trigger selected

    // Enable EXTI_Line13
    EXTI->IMR |= (1U << 13); // bit 13: 1: IRQ not masked
    // Clear any pending interrupt
    EXTI->PR |= (1U << 13); // bit 13 set to 1: select triggered request occurred. Clear the pending bit.
    // Enable EXTI_Line13_IRQ
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}