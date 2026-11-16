#include "tim2_5.h"

/*
initialize the TIM2 PWM mode 1
psc: prescaler
arr: auto-reload register
example:
tim2_pwm_init(0, 15999);
psc = 0, arr = 15999
TIM2->PSC = 0, TIM2->ARR = 15999
*/
void tim2_pwm_init(uint32_t psc, uint32_t arr)
{
    // Enable TIM2 clock in rcc.c
    
    // CEN = 0, counter is stopped
    TIM2->CR1 &= ~(1U << 0); // bit 0: CEN (Counter Enable)

    // Set TIM2 Prescaler
    TIM2->PSC = psc; // prescaler

    // Set TIM2 Auto-Reload Register
    TIM2->ARR = arr; // auto-reload register

    // Set TIM 2 Capture/Compare Mode Register 1 for PWM Mode 1
    TIM2->CCMR1 |= (1U << 3); // bit 3: OC1PE (Output Compare 1 Preload Enable)
    TIM2->CCMR1 |= (6U << 4); // bit 6:4: OC1M: 110 (PWM Mode 1)

    // CC1E must be set after the OC1M is set
    TIM2->CCER |= (1U << 0); // bit 0: CC1E (Capture/Compare 1 Enable)

    // Set TIM2 Auto-reload Preload Enable, wait for the next update event
    // Since ARPE is 0 after reset, ARR is updated directly
    TIM2->CR1 |= (1U << 7); // bit 7: ARPE (Auto-reload Preload Enable)

    // Set TIM2 Update Generation => This is the time of a restart from a known cycle
    // to update the capture/compare register, we need to set the update generation bit 
    // active (shadow) PSC and ARR registers are updated
    // preload PSC to active PSC, preload ARR to active ARR, clears CNT to 0, sets UIF (in SR) to 1
    TIM2->EGR |= (1U << 0); // bit 0: UG (Update Generation) to update the capture/compare register
    
    TIM2->SR &= ~(1U << 0); /* UIF = 0, clear the update interrupt flag */

    // CEN = 1, starts counting from 0, TIM2 counter is enabled
    TIM2->CR1 |= (1U << 0); // bit 0: CEN (Counter Enable)
}


void tim2_pwm_set_duty_cycle(uint8_t channel, uint32_t duty_cycle)
{
    // Set TIM2 Capture/Compare Register for the selected channel
    switch (channel)
    {
        case 1:
            TIM2->CCR1 = duty_cycle*(TIM2->ARR+1)/1000;
            break;
        case 2:
            TIM2->CCR2 = duty_cycle*(TIM2->ARR+1)/1000;
            break;
        default:
            break;
    }
}

