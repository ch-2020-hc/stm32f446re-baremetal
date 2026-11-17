#include "rcc.h"
#include "gpio.h"
#include "tim2_5.h"
#include "dma.h"

int main(void)
{
    rcc_init();
    gpio_tim2_ch1_pa5();

    /* CEN stays 0. UG inside init loads PSC and ARR and clears CNT. */
    tim2_pwm_init(0, 15999); /* PSC = 0, ARR = 15999: 16 MHz / 16000 = 1 kHz */

    /* EN = 1. Stream 1 waits for the TIM2 update request. */
    dma_tim2_init();

    /* UDE (Update DMA request enable) after EN. 
    An update pulse that arrives earlier is discarded. */
    TIM2->DIER |= (1U << 8); /* bit 8: UDE, TIM2_UP requests DMA1 stream 1 */

    /* First update copies duty_table[0] into CCR1. */
    TIM2->CR1 |= (1U << 0); /* bit 0: CEN = 1 */

    while (1) {
        /* TIM2 and DMA1 drive PA5. main stays here. */
    }
}
