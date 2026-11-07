#ifndef SYSCFG_H
#define SYSCFG_H

#include <stdint.h>

typedef struct {
    volatile uint32_t MEMRMP; // Memory remap register (offset 0x00)
    volatile uint32_t PMC; // Peripheral mode configuration register (offset 0x04)
    volatile uint32_t EXTICR1; // External interrupt configuration register 1 (offset 0x08)
    volatile uint32_t EXTICR2; // External interrupt configuration register 2 (offset 0x0C) 
    volatile uint32_t EXTICR3; // External interrupt configuration register 3 (offset 0x10)
    volatile uint32_t EXTICR4; // External interrupt configuration register 4 (offset 0x14)

} SYSCFG_TypeDef;

#define SYSCFG ((SYSCFG_TypeDef *) 0x40013800)

#endif
