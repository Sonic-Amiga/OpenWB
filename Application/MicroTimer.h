/*
 * MicroTimer.h
 *
 *  Created on: 18 Nov 2020 г.
 *      Author: Sonic
 */

#ifndef MICROTIMER_H_
#define MICROTIMER_H_

#include "stm32f0xx_ll_tim.h"

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

#endif /* MICROTIMER_H_ */
