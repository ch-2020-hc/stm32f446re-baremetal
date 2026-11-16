#include <stdint.h>
#include "rcc.h"
#include "gpio.h"
#include "tim2_5.h"
#include "stm32f446re.h"

/*
 * TIM2 update interrupt. The counter rolls over once per 1 ms period.
 * UIF (Update Interrupt Flag) stays 1 until software writes 0. Clearing it
 * is what lets this function return; otherwise the CPU re-enters immediately.
 * One permille step per period is a 2 s bright/dim cycle.
 */
void TIM2_IRQHandler(void)
{
    static int16_t duty = 0;
    static int8_t step = 1;

    TIM2->SR &= ~(1U << 0);

    duty += step;
    if (duty >= 1000) {
        duty = 1000;
        step = -1;
    } else if (duty <= 0) {
        duty = 0;
        step = 1;
    }
    tim2_pwm_set_duty_cycle(1, (uint32_t)duty);
}

int main(void)
{
    rcc_init();
    gpio_tim2_ch1_pa5();
    tim2_pwm_init(0, 15999); /* PSC = 0, ARR = 15999: 16 MHz / 16000 = 1 kHz */

    /* UG inside tim2_pwm_init sets UIF. Drop that flag, then arm the interrupt. */
    TIM2->SR &= ~(1U << 0);
    TIM2->DIER |= (1U << 0); /* UIE (Update Interrupt Enable) */
    NVIC_EnableIRQ(TIM2_IRQn);

    while (1) {
        /* TIM2 drives PA5. main stays here so it does not return to Reset_Handler. */
    }
}
