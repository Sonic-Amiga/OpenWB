/*
 * SerialDriver.h
 *
 *  Created on: 18 нояб. 2020 г.
 *      Author: Sonic
 */

#ifndef SERIALDRIVER_H_
#define SERIALDRIVER_H_

#include "stm32f0xx_hal.h"

#ifdef __cplusplus
extern "C" {
#endif

void UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size);
uint8_t UART_GetChar(UART_HandleTypeDef *huart);
void UART_StartReceive(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif


#endif /* SERIALDRIVER_H_ */
