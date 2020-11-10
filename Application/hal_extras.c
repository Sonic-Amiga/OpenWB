#include "hal_extras.h"
#include "main.h"
#include "stm32f0xx_hal.h"
#include "stm32f0xx_hal_gpio.h"
#include "stm32f0xx_ll_tim.h"

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

static void UART_Stop(UART_HandleTypeDef *huart)
{
    /* Disable TXE, RXNE, PE and ERR (Frame error, noise error, overrun error) interrupts for the interrupt process */
     CLEAR_BIT(huart->Instance->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE | USART_CR1_TXEIE));
     CLEAR_BIT(huart->Instance->CR3, USART_CR3_EIE);

     huart->gState = HAL_UART_STATE_READY;
     huart->RxState = HAL_UART_STATE_READY;

     __HAL_UNLOCK(huart);
}

static HAL_StatusTypeDef WaitOnFlagUntilTimeout(UART_HandleTypeDef *huart, uint32_t Flag, FlagStatus Status,
												uint8_t timeout)
{
    /* Wait until flag is set */
    while ((__HAL_UART_GET_FLAG(huart, Flag) ? SET : RESET) == Status)
    {
        /* Check for the Timeout */
        if (timeout && LL_TIM_IsActiveFlag_UPDATE(TIM3))
        {
        	LL_TIM_ClearFlag_UPDATE(TIM3);
            UART_Stop(huart);
            return HAL_TIMEOUT;
        }

        if (READ_BIT(huart->Instance->CR1, USART_CR1_RE) != 0U)
        {
            if (__HAL_UART_GET_FLAG(huart, UART_FLAG_RTOF) == SET)
            {
                /* Clear Receiver Timeout flag*/
                __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_RTOF);

                UART_Stop(huart);
                huart->ErrorCode = HAL_UART_ERROR_RTO;
                return HAL_TIMEOUT;
            }
        }
    }

    return HAL_OK;
}

HAL_StatusTypeDef ModBus_Receive(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size, uint8_t initial_timeout, uint32_t Timeout)
{
	if (!initial_timeout) {
		/* We are starting to receive a new frame. Make sure the bomb isn't ticking
		 * and clear any possible stale UIF. */
		LL_TIM_DisableCounter(TIM3);
		LL_TIM_ClearFlag_UPDATE(TIM3);
	}

	__HAL_LOCK(huart);

    huart->ErrorCode = HAL_UART_ERROR_NONE;
    huart->RxState = HAL_UART_STATE_BUSY_RX;

    /* Computation of UART mask to apply to RDR register */
    UART_MASK_COMPUTATION(huart);

    /* as long as data have to be received */
    while (Size)
    {
    	HAL_StatusTypeDef result = WaitOnFlagUntilTimeout(huart, UART_FLAG_RXNE, RESET, initial_timeout);

    	if (result != HAL_OK)
    		return result;

    	*pData++ = (uint8_t)(huart->Instance->RDR & (uint8_t)huart->Mask);
        Size--;

        initial_timeout = 1;
        LL_TIM_SetCounter(TIM3, Timeout);
        LL_TIM_EnableCounter(TIM3);
    }

    /* At end of Rx process, restore huart->RxState to Ready */
    huart->RxState = HAL_UART_STATE_READY;
    __HAL_UNLOCK(huart);

    return HAL_OK;
}
