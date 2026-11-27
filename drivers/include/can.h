#pragma once

#include "stm32f446re.h"

typedef struct
{
    volatile uint32_t FR1; /* Filter Register 1: Mask Mode */
    volatile uint32_t FR2; /* Filter Register 2: Identifier Mode */
} CAN_Filter_TypeDef;

typedef struct
{
    volatile uint32_t MCR;    /* 0x000 */
    volatile uint32_t MSR;    /* 0x004 */
    volatile uint32_t TSR;    /* 0x008 */
    volatile uint32_t RF0R;   /* 0x00C */
    volatile uint32_t RF1R;   /* 0x010 */
    volatile uint32_t IER;    /* 0x014 */
    volatile uint32_t ESR;    /* 0x018 */
    volatile uint32_t BTR;    /* 0x01C */
    uint32_t RESERVED0[88];   /* 0x020 - 0x17F */

    volatile uint32_t TI0R;   /* 0x180 */
    volatile uint32_t TDT0R;  /* 0x184 */
    volatile uint32_t TDL0R;  /* 0x188 */
    volatile uint32_t TDH0R;  /* 0x18C */
    volatile uint32_t TI1R;   /* 0x190 */
    volatile uint32_t TDT1R;  /* 0x194 */
    volatile uint32_t TDL1R;  /* 0x198 */
    volatile uint32_t TDH1R;  /* 0x19C */
    volatile uint32_t TI2R;   /* 0x1A0 */
    volatile uint32_t TDT2R;  /* 0x1A4 */
    volatile uint32_t TDL2R;  /* 0x1A8 */
    volatile uint32_t TDH2R;  /* 0x1AC */

    volatile uint32_t RI0R;   /* 0x1B0 */
    volatile uint32_t RDT0R;  /* 0x1B4 */
    volatile uint32_t RDL0R;  /* 0x1B8 */
    volatile uint32_t RDH0R;  /* 0x1BC */
    volatile uint32_t RI1R;   /* 0x1C0 */
    volatile uint32_t RDT1R;  /* 0x1C4 */
    volatile uint32_t RDL1R;  /* 0x1C8 */
    volatile uint32_t RDH1R;  /* 0x1CC */
    uint32_t RESERVED1[12];   /* 0x1D0 - 0x1FF */

    volatile uint32_t FMR;    /* 0x200 */
    volatile uint32_t FM1R;   /* 0x204 */
    uint32_t RESERVED2;       /* 0x208 */
    volatile uint32_t FS1R;   /* 0x20C */
    uint32_t RESERVED3;       /* 0x210 */
    volatile uint32_t FFA1R;  /* 0x214 */
    uint32_t RESERVED4;       /* 0x218 */
    volatile uint32_t FA1R;   /* 0x21C */
    uint32_t RESERVED5[8];    /* 0x220 - 0x23F */
    CAN_Filter_TypeDef Filter[28]; /* 0x240 - 0x31C, F0R1/F0R2 through F27R1/F27R2 */
} CAN_TypeDef;

#define CAN1 ((CAN_TypeDef*)0x40006400)

void can_init(void);