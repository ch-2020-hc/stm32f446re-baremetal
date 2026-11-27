#include "stm32f446re.h"
#include "gpio.h"
#include "can.h"


int main(void)
{
    
    rcc_can1_init();
    gpio_can1_pb8_pb9();
    can_init();


    while (1)
    {
        
    }
    return 0;
}