#include "hal_extras.h"
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

HAL_StatusTypeDef ModBus_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint32_t initial_timeout, uint32_t Timeout)
{
    __HAL_LOCK(huart);

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->RxState = HAL_UART_STATE_BUSY_RX;

    /* Computation of UART mask to apply to RDR register */
    UART_MASK_COMPUTATION(huart);

    /* as long as data have to be received */
    while (Size)
    {
    	HAL_StatusTypeDef result = UART_WaitOnFlagUntilTimeout(huart, UART_FLAG_RXNE, RESET, HAL_GetTick(), initial_timeout);

    	if (result != HAL_OK)
    		return result;

    	*pData++ = (uint8_t)(huart->Instance->RDR & (uint8_t)huart->Mask);
        Size--;

        initial_timeout = Timeout;
    }

    /* At end of Rx process, restore huart->RxState to Ready */
    huart->RxState = HAL_UART_STATE_READY;
    __HAL_UNLOCK(huart);

    return HAL_OK;
}
