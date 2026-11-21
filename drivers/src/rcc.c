#include "rcc.h"


void rcc_init(void)
{
    // Enable GPIOA and GPIOC
    RCC_AHB1ENR |= (1U << 0) | (1U << 2); 
    // Force a read of RCC_AHB1ENR to wait for clock to be ready
    volatile uint32_t clock_ready = RCC_AHB1ENR;
    (void)clock_ready;

    // Enable SYSCFG clock
    RCC_APB2ENR |= (1U << 14); // bit 14: 1: SYSCFG clock enabled
    // Force a read of RCC_APB2ENR to wait for clock to be ready
    clock_ready = RCC_APB2ENR;
    (void)clock_ready;

    // Enable TIM2 clock
    RCC_APB1ENR |= (1U << 0); // bit 0: 1: TIM2 clock enabled

    // Enable DMA1 clock
    RCC_AHB1ENR |= (1U << 21); // bit 21: 1: DMA1 clock enabled
} 

void rcc_spi1_init(void)
{
    /* SPI1 is APB2. GPIOA is SCK/MISO/MOSI. GPIOB is CS. */
    RCC_APB2ENR |= (1U << 12); // bit 12: SPI1EN, SPI1 clock enable
    RCC_AHB1ENR |= (1U << 0) | (1U << 1); // bit 0 GPIOAEN, bit 1 GPIOBEN

    /* Read the enable bits back so the writes finish before GPIO or SPI1 is used. */
    volatile uint32_t clock_ready = RCC_APB2ENR;
    clock_ready = RCC_AHB1ENR;
    (void)clock_ready;
}