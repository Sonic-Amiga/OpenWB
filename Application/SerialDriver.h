/*
 * SerialDriver.h
 *
 *  Created on: 18 Nov 2020
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
void UART_StartReceive(USART_TypeDef *uart);

static inline uint8_t UART_GetChar(USART_TypeDef *uart)
{
	return LL_USART_ReceiveData8(uart);
}

static inline void UART_Flush(USART_TypeDef* huart)
{
	LL_USART_RequestRxDataFlush(huart);
}

/* This is the IRQ handler, call it from within system's USART IRQ handler */
void UART_IRQHandler(USART_TypeDef *huart);

/* User-defined callbacks */
void UART_RxISR(USART_TypeDef *uart);
void UART_ErrorCallback(USART_TypeDef *uart);

#ifdef __cplusplus
}
#endif


#endif /* SERIALDRIVER_H_ */
