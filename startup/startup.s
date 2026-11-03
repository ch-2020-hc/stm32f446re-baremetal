.syntax unified
.cpu cortex-m4
.fpu fpv4-sp-d16
.thumb

/*
 * Cortex-M vector table for the STM32F446.
 * Word 0 is the initial main stack pointer. Word 1 is Reset_Handler.
 * Each later entry is one address at a fixed slot: byte offset (16 + IRQn) * 4.
 * Reserved slots stay 0. Every other name is a weak alias of Default_Handler,
 * so a C function with the same name (EXTI15_10_IRQHandler, TIM2_IRQHandler, ...)
 * replaces that slot at link time.
 * IRQ order follows RM0390 Table 38.
 */
.section .vectors, "a", %progbits
.global _vectors
.type _vectors, %object
_vectors:
    .word _estack                      /* initial MSP */
    .word Reset_Handler                /* reset */

    /* System Exception Handlers */
    .word NMI_Handler
    .word HardFault_Handler
    .word MemManage_Handler
    .word BusFault_Handler
    .word UsageFault_Handler
    .word 0
    .word 0
    .word 0
    .word 0
    .word SVC_Handler
    .word DebugMon_Handler
    .word 0
    .word PendSV_Handler
    .word SysTick_Handler

    /* External Interrupts (Interrupt Requests in the RM0390) */
    .word WWDG_IRQHandler              /* IRQ 0 */
    .word PVD_IRQHandler               /* IRQ 1 */
    .word TAMP_STAMP_IRQHandler        /* IRQ 2 */
    .word RTC_WKUP_IRQHandler          /* IRQ 3 */
    .word FLASH_IRQHandler             /* IRQ 4 */
    .word RCC_IRQHandler               /* IRQ 5 */
    .word EXTI0_IRQHandler             /* IRQ 6 */
    .word EXTI1_IRQHandler             /* IRQ 7 */
    .word EXTI2_IRQHandler             /* IRQ 8 */
    .word EXTI3_IRQHandler             /* IRQ 9 */
    .word EXTI4_IRQHandler             /* IRQ 10 */
    .word DMA1_Stream0_IRQHandler      /* IRQ 11 */
    .word DMA1_Stream1_IRQHandler      /* IRQ 12 */
    .word DMA1_Stream2_IRQHandler      /* IRQ 13 */
    .word DMA1_Stream3_IRQHandler      /* IRQ 14 */
    .word DMA1_Stream4_IRQHandler      /* IRQ 15 */
    .word DMA1_Stream5_IRQHandler      /* IRQ 16 */
    .word DMA1_Stream6_IRQHandler      /* IRQ 17 */
    .word ADC_IRQHandler               /* IRQ 18 */
    .word CAN1_TX_IRQHandler           /* IRQ 19 */
    .word CAN1_RX0_IRQHandler          /* IRQ 20 */
    .word CAN1_RX1_IRQHandler          /* IRQ 21 */
    .word CAN1_SCE_IRQHandler          /* IRQ 22 */
    .word EXTI9_5_IRQHandler           /* IRQ 23 */
    .word TIM1_BRK_TIM9_IRQHandler     /* IRQ 24 */
    .word TIM1_UP_TIM10_IRQHandler     /* IRQ 25 */
    .word TIM1_TRG_COM_TIM11_IRQHandler /* IRQ 26 */
    .word TIM1_CC_IRQHandler           /* IRQ 27 */
    .word TIM2_IRQHandler              /* IRQ 28 */
    .word TIM3_IRQHandler              /* IRQ 29 */
    .word TIM4_IRQHandler              /* IRQ 30 */
    .word I2C1_EV_IRQHandler           /* IRQ 31 */
    .word I2C1_ER_IRQHandler           /* IRQ 32 */
    .word I2C2_EV_IRQHandler           /* IRQ 33 */
    .word I2C2_ER_IRQHandler           /* IRQ 34 */
    .word SPI1_IRQHandler              /* IRQ 35 */
    .word SPI2_IRQHandler              /* IRQ 36 */
    .word USART1_IRQHandler            /* IRQ 37 */
    .word USART2_IRQHandler            /* IRQ 38 */
    .word USART3_IRQHandler            /* IRQ 39 */
    .word EXTI15_10_IRQHandler         /* IRQ 40 */
    .word RTC_Alarm_IRQHandler         /* IRQ 41 */
    .word OTG_FS_WKUP_IRQHandler       /* IRQ 42 */
    .word TIM8_BRK_TIM12_IRQHandler    /* IRQ 43 */
    .word TIM8_UP_TIM13_IRQHandler     /* IRQ 44 */
    .word TIM8_TRG_COM_TIM14_IRQHandler /* IRQ 45 */
    .word TIM8_CC_IRQHandler           /* IRQ 46 */
    .word DMA1_Stream7_IRQHandler      /* IRQ 47 */
    .word FMC_IRQHandler               /* IRQ 48 */
    .word SDIO_IRQHandler              /* IRQ 49 */
    .word TIM5_IRQHandler              /* IRQ 50 */
    .word SPI3_IRQHandler              /* IRQ 51 */
    .word UART4_IRQHandler             /* IRQ 52 */
    .word UART5_IRQHandler             /* IRQ 53 */
    .word TIM6_DAC_IRQHandler          /* IRQ 54 */
    .word TIM7_IRQHandler              /* IRQ 55 */
    .word DMA2_Stream0_IRQHandler      /* IRQ 56 */
    .word DMA2_Stream1_IRQHandler      /* IRQ 57 */
    .word DMA2_Stream2_IRQHandler      /* IRQ 58 */
    .word DMA2_Stream3_IRQHandler      /* IRQ 59 */
    .word DMA2_Stream4_IRQHandler      /* IRQ 60 */
    .word 0                            /* IRQ 61 reserved */
    .word 0                            /* IRQ 62 reserved */
    .word CAN2_TX_IRQHandler           /* IRQ 63 */
    .word CAN2_RX0_IRQHandler          /* IRQ 64 */
    .word CAN2_RX1_IRQHandler          /* IRQ 65 */
    .word CAN2_SCE_IRQHandler          /* IRQ 66 */
    .word OTG_FS_IRQHandler            /* IRQ 67 */
    .word DMA2_Stream5_IRQHandler      /* IRQ 68 */
    .word DMA2_Stream6_IRQHandler      /* IRQ 69 */
    .word DMA2_Stream7_IRQHandler      /* IRQ 70 */
    .word USART6_IRQHandler            /* IRQ 71 */
    .word I2C3_EV_IRQHandler           /* IRQ 72 */
    .word I2C3_ER_IRQHandler           /* IRQ 73 */
    .word OTG_HS_EP1_OUT_IRQHandler    /* IRQ 74 */
    .word OTG_HS_EP1_IN_IRQHandler     /* IRQ 75 */
    .word OTG_HS_WKUP_IRQHandler       /* IRQ 76 */
    .word OTG_HS_IRQHandler            /* IRQ 77 */
    .word DCMI_IRQHandler              /* IRQ 78 */
    .word 0                            /* IRQ 79 reserved */
    .word 0                            /* IRQ 80 reserved */
    .word FPU_IRQHandler               /* IRQ 81 */
    .word 0                            /* IRQ 82 reserved */
    .word 0                            /* IRQ 83 reserved */
    .word SPI4_IRQHandler              /* IRQ 84 */
    .word 0                            /* IRQ 85 reserved */
    .word 0                            /* IRQ 86 reserved */
    .word SAI1_IRQHandler              /* IRQ 87 */
    .word 0                            /* IRQ 88 reserved */
    .word 0                            /* IRQ 89 reserved */
    .word 0                            /* IRQ 90 reserved */
    .word SAI2_IRQHandler              /* IRQ 91 */
    .word QUADSPI_IRQHandler           /* IRQ 92 */
    .word CEC_IRQHandler               /* IRQ 93 */
    .word SPDIF_RX_IRQHandler          /* IRQ 94 */
    .word FMPI2C1_EV_IRQHandler        /* IRQ 95 */
    .word FMPI2C1_ER_IRQHandler        /* IRQ 96 */
.size _vectors, . - _vectors

.section .text
.thumb_func
.global Reset_Handler
.type Reset_Handler, %function
Reset_Handler:
    ldr   sp, =_estack

    /* Copy .data initializers from their flash load address into RAM. */
    ldr   r0, =_sidata
    ldr   r1, =_sdata
    ldr   r2, =_edata
copy_data:
    cmp   r1, r2
    bcs   zero_bss
    ldr   r3, [r0], #4
    str   r3, [r1], #4
    b     copy_data

zero_bss:
    movs  r0, #0
    ldr   r1, =_sbss
    ldr   r2, =_ebss
clear_bss:
    cmp   r1, r2
    bcs   call_main
    str   r0, [r1], #4
    b     clear_bss

call_main:
    bl    main
hang:
    b     hang
.size Reset_Handler, . - Reset_Handler

.thumb_func
.type Default_Handler, %function
Default_Handler:
    b     Default_Handler
.size Default_Handler, . - Default_Handler

.macro weak_handler name
    .weak \name
    .thumb_set \name, Default_Handler
.endm

weak_handler NMI_Handler
weak_handler HardFault_Handler
weak_handler MemManage_Handler
weak_handler BusFault_Handler
weak_handler UsageFault_Handler
weak_handler SVC_Handler
weak_handler DebugMon_Handler
weak_handler PendSV_Handler
weak_handler SysTick_Handler

weak_handler WWDG_IRQHandler
weak_handler PVD_IRQHandler
weak_handler TAMP_STAMP_IRQHandler
weak_handler RTC_WKUP_IRQHandler
weak_handler FLASH_IRQHandler
weak_handler RCC_IRQHandler
weak_handler EXTI0_IRQHandler
weak_handler EXTI1_IRQHandler
weak_handler EXTI2_IRQHandler
weak_handler EXTI3_IRQHandler
weak_handler EXTI4_IRQHandler
weak_handler DMA1_Stream0_IRQHandler
weak_handler DMA1_Stream1_IRQHandler
weak_handler DMA1_Stream2_IRQHandler
weak_handler DMA1_Stream3_IRQHandler
weak_handler DMA1_Stream4_IRQHandler
weak_handler DMA1_Stream5_IRQHandler
weak_handler DMA1_Stream6_IRQHandler
weak_handler ADC_IRQHandler
weak_handler CAN1_TX_IRQHandler
weak_handler CAN1_RX0_IRQHandler
weak_handler CAN1_RX1_IRQHandler
weak_handler CAN1_SCE_IRQHandler
weak_handler EXTI9_5_IRQHandler
weak_handler TIM1_BRK_TIM9_IRQHandler
weak_handler TIM1_UP_TIM10_IRQHandler
weak_handler TIM1_TRG_COM_TIM11_IRQHandler
weak_handler TIM1_CC_IRQHandler
weak_handler TIM2_IRQHandler
weak_handler TIM3_IRQHandler
weak_handler TIM4_IRQHandler
weak_handler I2C1_EV_IRQHandler
weak_handler I2C1_ER_IRQHandler
weak_handler I2C2_EV_IRQHandler
weak_handler I2C2_ER_IRQHandler
weak_handler SPI1_IRQHandler
weak_handler SPI2_IRQHandler
weak_handler USART1_IRQHandler
weak_handler USART2_IRQHandler
weak_handler USART3_IRQHandler
weak_handler EXTI15_10_IRQHandler
weak_handler RTC_Alarm_IRQHandler
weak_handler OTG_FS_WKUP_IRQHandler
weak_handler TIM8_BRK_TIM12_IRQHandler
weak_handler TIM8_UP_TIM13_IRQHandler
weak_handler TIM8_TRG_COM_TIM14_IRQHandler
weak_handler TIM8_CC_IRQHandler
weak_handler DMA1_Stream7_IRQHandler
weak_handler FMC_IRQHandler
weak_handler SDIO_IRQHandler
weak_handler TIM5_IRQHandler
weak_handler SPI3_IRQHandler
weak_handler UART4_IRQHandler
weak_handler UART5_IRQHandler
weak_handler TIM6_DAC_IRQHandler
weak_handler TIM7_IRQHandler
weak_handler DMA2_Stream0_IRQHandler
weak_handler DMA2_Stream1_IRQHandler
weak_handler DMA2_Stream2_IRQHandler
weak_handler DMA2_Stream3_IRQHandler
weak_handler DMA2_Stream4_IRQHandler
weak_handler CAN2_TX_IRQHandler
weak_handler CAN2_RX0_IRQHandler
weak_handler CAN2_RX1_IRQHandler
weak_handler CAN2_SCE_IRQHandler
weak_handler OTG_FS_IRQHandler
weak_handler DMA2_Stream5_IRQHandler
weak_handler DMA2_Stream6_IRQHandler
weak_handler DMA2_Stream7_IRQHandler
weak_handler USART6_IRQHandler
weak_handler I2C3_EV_IRQHandler
weak_handler I2C3_ER_IRQHandler
weak_handler OTG_HS_EP1_OUT_IRQHandler
weak_handler OTG_HS_EP1_IN_IRQHandler
weak_handler OTG_HS_WKUP_IRQHandler
weak_handler OTG_HS_IRQHandler
weak_handler DCMI_IRQHandler
weak_handler FPU_IRQHandler
weak_handler SPI4_IRQHandler
weak_handler SAI1_IRQHandler
weak_handler SAI2_IRQHandler
weak_handler QUADSPI_IRQHandler
weak_handler CEC_IRQHandler
weak_handler SPDIF_RX_IRQHandler
weak_handler FMPI2C1_EV_IRQHandler
weak_handler FMPI2C1_ER_IRQHandler
