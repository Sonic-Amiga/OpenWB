/*
 * SerialDriver.c
 *
 *  Created on: 18 нояб. 2020 г.
 *      Author: Sonic
 *
 * A stripped down STM32 serial port driver. Great space saver compared to HAL.
 */

#include "SerialDriver.h"

static const uint32_t parity_table[] =
{
	LL_USART_PARITY_NONE,
	LL_USART_PARITY_ODD,
	LL_USART_PARITY_EVEN
};

void UART_Configure(USART_TypeDef* uart, uint32_t baud, uint16_t parity, uint16_t stop_bits)
{
	LL_USART_InitTypeDef USART_InitStruct = {0};

    USART_InitStruct.BaudRate            = baud;
	USART_InitStruct.DataWidth           = (parity == 0) ? LL_USART_DATAWIDTH_8B : LL_USART_DATAWIDTH_9B;
	USART_InitStruct.StopBits            = (stop_bits == 2) ? LL_USART_STOPBITS_2 : LL_USART_STOPBITS_1;
	USART_InitStruct.Parity              = parity_table[parity];
	USART_InitStruct.TransferDirection   = LL_USART_DIRECTION_TX_RX;
	USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	USART_InitStruct.OverSampling        = LL_USART_OVERSAMPLING_16;

	LL_USART_Init(USART1, &USART_InitStruct);
}

void UART_Transmit(USART_TypeDef* uart, uint8_t *pData, uint16_t Size)
{
    while (Size)
    {
    	while(!LL_USART_IsActiveFlag_TXE(uart));
    	LL_USART_TransmitData8(uart, *pData++);
        Size--;
    }
}

uint8_t UART_GetChar(USART_TypeDef *uart)
{
	while(!LL_USART_IsActiveFlag_RXNE(uart));
	return LL_USART_ReceiveData8(uart);
}
