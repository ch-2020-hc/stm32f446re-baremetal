#include "rcc.h"
#include "gpio.h"
#include "spi.h"
#include "stm32f446re.h"


volatile uint8_t g_rx_msg[2];
volatile uint8_t g_whoami;



int main(void)
{
    /* WHO_AM_I read: register 0x00 with the read bit, then one dummy byte. */
    static const uint8_t tx[2] = { 0x80U, 0x00U };
    spi_init();
    spi_exchange_interrupt(tx, g_rx_msg, 2U);
    while (g_spi_done == 0U)
    {
        /* SPI1_IRQHandler sets g_spi_done. Sleep until that interrupt. */
        __WFI();
    }
    g_whoami = g_rx_msg[1]; /* Byte shifted in while the dummy 0x00 was clocked out. */

    while (1)
    {
        /* loop stays here until the program is reset */
    }
    return 0U;
}