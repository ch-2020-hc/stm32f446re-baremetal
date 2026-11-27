#include "spi.h"
#include "rcc.h"
#include "gpio.h"
#include "stm32f446re.h"

/* ISR writes it, main reads it. Global so main.c can use the extern in spi.h. */
volatile uint8_t g_spi_done;
/* static: only spi.c uses it, and it must outlive spi_exchange_interrupt.
 * volatile uint8_t: the ISR stores received bytes through this pointer.
 * *volatile: main stores the pointer, the ISR reads it. */
static volatile uint8_t *volatile g_rx_ptr;
/* static: private to spi.c. volatile: main uses it, the ISR reads it. */
static volatile uint8_t g_tx[2];
static volatile uint8_t g_len;
static volatile uint8_t g_tx_count;
static volatile uint8_t g_rx_count;



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
     * The caller has already read DR, which clears RXNE.
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

void spi_exchange_interrupt(const uint8_t *tx, volatile uint8_t *rx, uint16_t len)
{
    uint8_t i;
    for (i = 0U; i < len; i++)
    {
        g_tx[i] = tx[i];
    }
    /* g_rx_ptr keeps the caller's buffer. For this app that is g_rx_msg,
       so g_rx_ptr[g_rx_count] is g_rx_msg[g_rx_count]. */
    g_rx_ptr = rx; 
    g_len = (uint8_t)len;
    g_tx_count = 0U;
    g_rx_count = 0U;
    g_spi_done = 0U;

    SPI1->CR2 &= ~(1U << 7); /* TXEIE off */
    SPI1->CR2 &= ~(1U << 6); /* RXNEIE off */

    SPI1->CR2 |= (1U << 5); /* ERRIE: OVR, MODF, or CRCERR can request SPI1_IRQn */

    cs_low();
    /* CS is low before the first SCK. TXE is already 1, so the first write happens in the handler. */
    SPI1->CR2 |= (1U << 7); /* TXEIE on */
    SPI1->CR2 |= (1U << 6); /* RXNEIE on */
    
    NVIC_EnableIRQ(SPI1_IRQn);
    
    return;
}

void SPI1_IRQHandler(void)
{
    /* OVR: Overrun flag */
    if ((SPI1->SR & (1U << 6)) != 0U) {
        (void)SPI1->DR; /* first step: read the unread byte */
        (void)SPI1->SR; /* second step: this read clears OVR */
        SPI1->CR2 &= ~(1U << 5); /* ERRIE off */
        SPI1->CR2 &= ~(1U << 7); /* TXEIE off */
        SPI1->CR2 &= ~(1U << 6); /* RXNEIE off */
        g_spi_done = 1U;
        return;
    }

    /* RXNE is 1 and RXNEIE is set: one received byte is in DR. */
    while (((SPI1->SR & (1U << 0)) != 0U) && ((SPI1->CR2 & (1U << 6)) != 0U))
    {
        g_rx_ptr[g_rx_count] = (uint8_t)SPI1->DR;
        g_rx_count++;
        if (g_rx_count == g_len)
        {
            SPI1->CR2 &= ~(1U << 7); /* TXEIE off */
            SPI1->CR2 &= ~(1U << 6); /* RXNEIE off */
            SPI1->CR2 &= ~(1U << 5); /* ERRIE off */
            cs_high();
            g_spi_done = 1U;
        }
    }

    /* TXE is 1 and TXEIE is set: the transmit buffer can take one byte. */
    if (((SPI1->SR & (1U << 1)) != 0U) && ((SPI1->CR2 & (1U << 7)) != 0U))
    {
        SPI1->DR = g_tx[g_tx_count];
        g_tx_count++;
        if (g_tx_count == g_len)
        {
            /* Last byte is in DR. The shift on SCK is still running. */
            SPI1->CR2 &= ~(1U << 7); /* TXEIE off. TXE stays 1, so leaving this bit set enters the handler again. */
        }
    }
    return;
}