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

#include <inttypes.h>

#define RISING_EDGE  0x00100000U
#define FALLING_EDGE 0x00200000U

void GPIO_Set_Trigger(uint16_t pin, uint32_t mode);

#ifdef __cplusplus
}
#endif


#endif /* HAL_EXTRAS_H_ */
