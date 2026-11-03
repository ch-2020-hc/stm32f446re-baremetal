#include "stdint.h"
#ifndef __GPIO_H
#define __GPIO_H


typedef struct {
    volatile uint32_t MODER;   /**< GPIO port mode register (input/output), Address offset: 0x00 */
    volatile uint32_t OTYPER;  /**< GPIO port output type register (push-pull/open-drain), Address offset: 0x04 */
    volatile uint32_t OSPEEDR; /**< GPIO port output speed register (low/medium/high), Address offset: 0x08 */
    volatile uint32_t PUPDR;   /**< GPIO port pull-up/pull-down register (no/pull-up/pull-down), Address offset: 0x0C */
    volatile uint32_t IDR;     /**< GPIO port input data register, Address offset: 0x10 */
    volatile uint32_t ODR;     /**< GPIO port output data register, Address offset: 0x14 */
    volatile uint32_t BSRR;    /**< GPIO port bit set/reset register, Address offset: 0x18 */
    volatile uint32_t LCKR;    /**< GPIO port configuration lock register, Address offset: 0x1C */
    volatile uint32_t AFRL;    /**< GPIO alternate function low register (AF0/AF1/AF2/AF3), Address offset: 0x20 */
    volatile uint32_t AFRH;    /**< GPIO alternate function high register (AF0/AF1/AF2/AF3), Address offset: 0x24 */
} GPIO_TypeDef;

/*Peripheral base addresses*/

#define GPIOA_BASE 0x40020000
#define GPIOC_BASE 0x40020800

#define GPIOA ((GPIO_TypeDef *)GPIOA_BASE)
#define GPIOC ((GPIO_TypeDef *)GPIOC_BASE)

#define RCC_AHB1ENR *(volatile uint32_t *)(0x40023800 + 0x30)

#endif