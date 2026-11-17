#include "dma.h"
#include "tim2_5.h"

/*
 * One DMA write per 1 ms PWM period, 2000 entries, circular: a 2 s cycle.
 * Values are CCR counts, not permille. With ARR = 15999,
 * CCR = permille * (ARR + 1) / 1000 = permille * 16.
 * Same shape as the TIM2 update ISR: 1..1000, then 999..0.
 */
#define DUTY_TABLE_LEN 2000U
#define DUTY_CCR_PER_PERMILLE 16U

/* duty_table is in RAM .bss section. 
It is used to store the duty cycle values for the TIM2 PWM.
And later it is used by the DMA to copy the duty cycle values
from RAM to the TIM2 CCR1 register.
*/ 
static uint32_t duty_table[DUTY_TABLE_LEN];

static void duty_table_fill(void)
{
    uint32_t i;

    /* permille 1..1000 -> CCR 16, 32, ... 16000 */
    for (i = 0U; i < 1000U; i++) {
        duty_table[i] = (i + 1U) * DUTY_CCR_PER_PERMILLE;
    }
    /* permille 999..0 -> CCR 15984, 15968, ... 0 */
    for (i = 0U; i < 1000U; i++) {
        duty_table[1000U + i] = (999U - i) * DUTY_CCR_PER_PERMILLE;
    }
}

void dma_tim2_init(void)
{
    duty_table_fill();

    /* EN = 0 asks stream 1 to stop. The bit can stay 1 until the current beat ends. */
    DMA1->S1CR &= ~1U; // bit 0: EN = 0
    while ((DMA1->S1CR & 1U) != 0U)
    {
        /* EN still reads 1. S1CR, S1NDTR, S1PAR, and S1M0AR ignore writes until it reads 0. */
    }

    /*
     * LIFCR is write-1-to-clear, and only after EN reads 0.
     * Bits 6, 8, and 9 are the FIFO, direct-mode, and transfer errors.
     * Bits 10 and 11 are half-transfer and transfer-complete, not errors.
     */
    DMA1->LIFCR |= (1U << 6); // CFEIF1: clear FIFO error flag
    DMA1->LIFCR |= (1U << 8); // CDMEIF1: clear direct-mode error flag
    DMA1->LIFCR |= (1U << 9); // CTEIF1: clear transfer error flag
    DMA1->LIFCR |= (1U << 10); // CHTIF1: clear half-transfer flag
    DMA1->LIFCR |= (1U << 11); // CTCIF1: clear transfer-complete flag

    DMA1->S1CR |= (3U << 25); // CHSEL = 011: channel 3, the TIM2_UP request on stream 1
    DMA1->S1CR |= (1U << 6); // DIR bit 6 = 1
    DMA1->S1CR &= ~(1U << 7); // DIR bit 7 = 0, so DIR = 01: memory to peripheral
    DMA1->S1CR |= (1U << 8); // CIRC = 1: reload NDTR and M0AR when the table ends
    DMA1->S1CR &= ~(1U << 9); // PINC = 0: every beat writes CCR1, not CCR2, CCR3, ...
    DMA1->S1CR |= (1U << 10); // MINC = 1: advance one uint32_t entry after each beat
    DMA1->S1CR |= (2U << 11); // PSIZE = 10: peripheral side is a 32-bit write
    DMA1->S1CR |= (2U << 13); // MSIZE = 10: memory side is a 32-bit read. NDTR counts words
    DMA1->S1CR &= ~(3U << 21); // PBURST = 00: one beat per request. Required in direct mode
    DMA1->S1CR &= ~(3U << 23); // MBURST = 00: one beat per request
    DMA1->S1CR &= ~(1U << 5); // PFCTRL = 0: this DMA stream is the flow controller

    /* Destination address. TIM2_UP writes the next table word into CCR1. */
    DMA1->S1PAR = (uint32_t)&TIM2->CCR1;

    /* Source address of the RAM table. This is not a duty value. */
    DMA1->S1M0AR = (uint32_t)duty_table;
    /* Number of words in duty_table. CIRC reloads this when it reaches 0. */
    DMA1->S1NDTR = DUTY_TABLE_LEN;

    /* EN = 1 last, after PAR, M0AR, and NDTR. Requests are ignored before this. */
    DMA1->S1CR |= 1U; // bit 0: EN = 1
}
