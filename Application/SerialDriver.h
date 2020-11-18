/*
 * SerialDriver.h
 *
 *  Created on: 18 нояб. 2020 г.
 *      Author: Sonic
 */

#ifndef SERIALDRIVER_H_
#define SERIALDRIVER_H_

#include "stm32f0xx_ll_usart.h"

typedef USART_TypeDef UART_HandleTypeDef;

#ifdef __cplusplus
extern "C" {
#endif

void UART_Configure(USART_TypeDef* uart, uint32_t baud, uint16_t data_bits, uint16_t stop_bits);
void UART_Transmit(USART_TypeDef *huart, uint8_t *pData, uint16_t Size);
uint8_t UART_GetChar(USART_TypeDef *huart);

static inline void UART_Flush(USART_TypeDef* huart)
{
	LL_USART_RequestRxDataFlush(huart);
}



#ifdef __cplusplus
}
#endif


#endif /* SERIALDRIVER_H_ */
