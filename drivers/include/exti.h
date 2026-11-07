#ifndef EXTI_H
#define EXTI_H

#include <stdint.h>

typedef struct {
    volatile uint32_t IMR; // Interrupt mask register (offset 0x00)
    volatile uint32_t EMR; // Event mask register (offset 0x04)
    volatile uint32_t RTSR; // Rising trigger selection register (offset 0x08)
    volatile uint32_t FTSR; // Falling trigger selection register (offset 0x0C)
    volatile uint32_t SWIER; // Software interrupt event register (offset 0x10)
    volatile uint32_t PR; // Pending register (offset 0x14)
} EXTI_TypeDef;

#define EXTI ((EXTI_TypeDef *) 0x40013C00)

void exti_init(void);

#endif