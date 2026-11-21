#include "spi.h"
#include "rcc.h"
#include "gpio.h"

static void cs_low(void)
{
    /* BR6 is BSRR bit 22. Writing 1 drives PB6 low. */
    GPIOB->BSRR = (1U << (6 + 16));
}

static void cs_high(void)
{
    /*
     * BSY is SR bit 7. BSY = 1 means a frame is still shifting on SCK.
     * BSY = 0 means the shift register is empty and the Tx buffer is empty,
     * so the last bit has left SCK. Raise CS only after that.
     * spi_exchange_byte() has already read DR, which clears RXNE.
     */
    while ((SPI1->SR & (1U << 7)) != 0U) {
    }
    GPIOB->BSRR = (1U << 6); /* BS6 is BSRR bit 6. Writing 1 drives PB6 high. */
}

void spi_init(void)
{
    rcc_spi1_init(); /* SPI1, GPIOA, and GPIOB clocks */
    gpio_spi1_pa5_pa6_pa7(); /* SCK, MISO, MOSI */
    gpio_spi1_pb6(); /* Active-low CS */

    SPI1->CR1 &= ~(7U << 3);
    SPI1->CR1 |= (3U << 3);      /* BR = 011: 16 MHz / 16 = 1 MHz */
    /* Matching ADXL345's SPI mode 3 (CPOL = 1, CPHA = 1) */
    SPI1->CR1 |= (1U << 1);      /* CPOL = 1 */
    SPI1->CR1 |= (1U << 0);      /* CPHA = 1 */
    SPI1->CR1 &= ~(1U << 7);     /* LSBFIRST = 0: MSB first */

    /*
     * PB6 is GPIO, not SPI1_NSS 
     * (PA4 or PA15: SPI peripheral's hardware NSS pin ).
     * Set SSM = 1 to use software NSS.
     * SSM = 1 takes NSS from SSI. SSI = 1 holds that internal NSS high,
     * which keeps MODF clear in master mode.
     */
    SPI1->CR1 |= (1U << 9);      /* SSM, CR1 bit 9 */
    SPI1->CR1 |= (1U << 8);      /* SSI, CR1 bit 8 */
    
    SPI1->CR1 |= (1U << 2);      /* MSTR = 1 */
    SPI1->CR1 &= ~(1U << 11);    /* DFF = 0: 8-bit */
    SPI1->CR1 |= (1U << 6);      /* SPE = 1 last. Mode bits above are written while SPE is 0. */
}

uint8_t spi_exchange_byte(uint8_t tx)
{
    /*
     * TXE is SR bit 1. Wait until it is 1.
     * TXE = 1 means the transmit buffer is empty, so the next write to DR is accepted.
     * The previous byte has moved to the shift register. It is not still in DR.
     */
    while ((SPI1->SR & (1U << 1)) == 0U) {
        /* TXE = 0: the transmit buffer still holds the previous byte. */
    }

    /* Write the byte to the DR */
    SPI1->DR = tx; 

    /* Wait for RXNE to be set (filled), meaning the shifted-in byte is in DR. */
    while ((SPI1->SR & (1U << 0)) == 0U) {
        /* RXNE = 0 (Rx buffer empty): 
        the shifted-in byte is not in DR yet */
    }
    /* The cast to uint8_t keeps the low 8 bits, 
    which matches DFF = 0 (8-bit frames) set in spi_init. */
    return (uint8_t)SPI1->DR; /* this read DR also clears RXNE flag */
}

void spi_exchange(const uint8_t *tx, volatile uint8_t *rx, uint16_t len)
{
    uint16_t i;

    cs_low();
    for (i = 0U; i < len; i++) {
        uint8_t byte = spi_exchange_byte(tx[i]);
        if (rx != 0) {
            rx[i] = byte;
        }
    }
    cs_high();
}