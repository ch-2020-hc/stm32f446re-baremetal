#include "rcc.h"
#include "gpio.h"
#include "spi.h"


volatile uint8_t g_rx[2];
volatile uint8_t g_whoami;

int main(void)
{
    /* send message to dummy ADXL345 */
    static const uint8_t tx[2] = { 0x80U, 0x00U };
    spi_init();
    spi_exchange(tx, g_rx, 2U);
    g_whoami = g_rx[1];

    while (1)
    {
        /* loop stays here until the program is reset */
    }
}

