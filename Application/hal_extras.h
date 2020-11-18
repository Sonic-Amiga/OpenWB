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

#define RISING_EDGE  0x0001U
#define FALLING_EDGE 0x0002U

void GPIO_Set_Trigger(uint16_t pin, uint16_t mode);

#ifdef __cplusplus
}
#endif


#endif /* HAL_EXTRAS_H_ */
