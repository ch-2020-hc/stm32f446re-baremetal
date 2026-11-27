#pragma once

#include <stdint.h>

extern volatile uint8_t g_spi_done;

typedef struct
{
    volatile uint32_t CR1;     /**< SPI control register 1 (mode, baud, enable), Address offset: 0x00 */
    volatile uint32_t CR2;     /**< SPI control register 2 (interrupts, DMA, NSS output), Address offset: 0x04 */
    volatile uint32_t SR;      /**< SPI status register (RXNE, TXE, BSY), Address offset: 0x08 */
    volatile uint32_t DR;      /**< SPI data register. Write transmits, read receives, Address offset: 0x0C */
    volatile uint32_t CRCPR;   /**< SPI CRC polynomial register, Address offset: 0x10 */
    volatile uint32_t RXCRCR;  /**< SPI RX CRC register, Address offset: 0x14 */
    volatile uint32_t TXCRCR;  /**< SPI TX CRC register, Address offset: 0x18 */
    volatile uint32_t I2SCFGR; /**< I2S configuration register, Address offset: 0x1C */
    volatile uint32_t I2SPR;   /**< I2S prescaler register, Address offset: 0x20 */
} SPI_TypeDef;

/* SPI1 is on APB2. */
#define SPI1 ((SPI_TypeDef *)0x40013000)

void spi_init(void);
uint8_t spi_exchange_byte(uint8_t tx);
void spi_exchange(const uint8_t *tx, volatile uint8_t *rx, uint16_t len);
void spi_exchange_interrupt(const uint8_t *tx, volatile uint8_t *rx, uint16_t len);
