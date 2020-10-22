#include "hal_extras.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"

void GPIO_Set_Trigger(uint16_t pin, uint32_t mode)
{
    uint32_t temp;

    temp = EXTI->RTSR;
    if (mode & RISING_EDGE)
        temp |= pin;
    else
    	temp &= ~(pin);
    EXTI->RTSR = temp;

    temp = EXTI->FTSR;
    if (mode & FALLING_EDGE)
        temp |= pin;
    else
    	temp &= ~(pin);
    EXTI->FTSR = temp;
}
