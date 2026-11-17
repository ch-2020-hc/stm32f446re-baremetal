#ifndef __DMA_H__
#define __DMA_H__

#include <stdint.h>


typedef struct {
    volatile uint32_t LISR;
    volatile uint32_t HISR;
    volatile uint32_t LIFCR;
    volatile uint32_t HIFCR;
    volatile uint32_t S0CR;
    volatile uint32_t S0NDTR;
    volatile uint32_t S0PAR;
    volatile uint32_t S0M0AR;
    volatile uint32_t S0M1AR;
    volatile uint32_t S0FCR;
    volatile uint32_t S1CR;
    volatile uint32_t S1NDTR;
    volatile uint32_t S1PAR;
    volatile uint32_t S1M0AR;
    volatile uint32_t S1M1AR;
    volatile uint32_t S1FCR;
} DMA_TypeDef;

#define DMA1 ((DMA_TypeDef *)0x40026000)

void dma_tim2_init(void);

#endif