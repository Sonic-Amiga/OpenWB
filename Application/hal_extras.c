#include "hal_extras.h"
#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"

void GPIO_Set_Trigger(uint16_t pin, uint16_t mode)
{
    uint32_t temp;

    temp = EXTI->RTSR & ~(pin);
    if (mode & RISING_EDGE)
        temp |= pin;
    EXTI->RTSR = temp;

    temp = EXTI->FTSR & ~(pin);
    if (mode & FALLING_EDGE)
        temp |= pin;
    EXTI->FTSR = temp;
}
