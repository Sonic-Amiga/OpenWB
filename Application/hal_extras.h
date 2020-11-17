/*
 * hal_extras.h
 *
 *  Created on: 23 oct 2020
 *      Author: Sonic
 *
 * Some routines, missing from standard HAL
 */

#ifndef HAL_EXTRAS_H_
#define HAL_EXTRAS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_tim.h"

#define RISING_EDGE  0x0001U
#define FALLING_EDGE 0x0002U

void GPIO_Set_Trigger(uint16_t pin, uint16_t mode);

static inline void Micro_Timer_Stop(void)
{
	LL_TIM_DisableCounter(TIM3);
	LL_TIM_ClearFlag_UPDATE(TIM3); /* Clear any possible stale UIF. */
}

static inline void Micro_Timer_Start(uint32_t timeout)
{
    LL_TIM_SetCounter(TIM3, timeout);
    LL_TIM_EnableCounter(TIM3);
}

static inline uint8_t Micro_Timer_Expired(void)
{
	uint8_t timeout = LL_TIM_IsActiveFlag_UPDATE(TIM3);

	if (timeout)
	    LL_TIM_ClearFlag_UPDATE(TIM3);

	return timeout;
}

static inline uint8_t UART_HasCharacter(UART_HandleTypeDef* uart)
{
	return __HAL_UART_GET_FLAG(uart, UART_FLAG_RXNE);
}

static inline uint16_t UART_GetCharacter(UART_HandleTypeDef* huart)
{
	return huart->Instance->RDR & huart->Mask;
}

#ifdef __cplusplus
}
#endif


#endif /* HAL_EXTRAS_H_ */
