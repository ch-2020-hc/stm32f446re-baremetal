#ifndef TIM2_5_H
#define TIM2_5_H

#include <stdint.h>

typedef struct {
    volatile uint32_t CR1;   /**< TIM control register 1, Address offset: 0x00 */
    volatile uint32_t CR2;   /**< TIM control register 2, Address offset: 0x04 */
    volatile uint32_t SMCR;  /**< TIM slave mode control register, Address offset: 0x08 */
    volatile uint32_t DIER;  /**< TIM interrupt enable register, Address offset: 0x0C */
    volatile uint32_t SR;    /**< TIM status register, Address offset: 0x10 */
    volatile uint32_t EGR;   /**< TIM event generation register, Address offset: 0x14 */
    volatile uint32_t CCMR1;  /**< TIM capture/compare mode register 1, Address offset: 0x18 */
    volatile uint32_t CCMR2;  /**< TIM capture/compare mode register 2, Address offset: 0x1C */
    volatile uint32_t CCER;  /**< TIM capture/compare enable register, Address offset: 0x20 */
    volatile uint32_t CNT;   /**< TIM counter register, Address offset: 0x24 */
    volatile uint32_t PSC;   /**< TIM prescaler register, Address offset: 0x28 */
    volatile uint32_t ARR;   /**< TIM auto-reload register, Address offset: 0x2C */
    volatile uint32_t Reserved1;  /**< Reserved register, Address offset: 0x30 */
    volatile uint32_t CCR1;  /**< TIM capture/compare register 1, Address offset: 0x34 */
    volatile uint32_t CCR2;  /**< TIM capture/compare register 2, Address offset: 0x38 */
    volatile uint32_t CCR3;  /**< TIM capture/compare register 3, Address offset: 0x3C */
    volatile uint32_t CCR4;  /**< TIM capture/compare register 4, Address offset: 0x40 */
    volatile uint32_t Reserved2;  /**< Reserved register, Address offset: 0x44 */
    volatile uint32_t DCR;   /**< TIM DMA control register, Address offset: 0x48 */
    volatile uint32_t DMAR;  /**< TIM DMA address for full transfer, Address offset: 0x4C */
} TIM_TypeDef;

#define TIM2_BASE 0x40000000
#define TIM2 ((TIM_TypeDef *)TIM2_BASE)

#define TIM5_BASE 0x40000C00
#define TIM5 ((TIM_TypeDef *)TIM5_BASE)


void tim2_pwm_init(uint32_t psc, uint32_t arr);
void tim2_pwm_set_duty_cycle(uint8_t channel, uint32_t duty_cycle);

#endif // TIM2_5_H  