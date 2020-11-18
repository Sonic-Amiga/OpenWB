/*
 * SerialDriver.c
 *
 *  Created on: 18 нояб. 2020 г.
 *      Author: Sonic
 *
 * A stripped down STM32 serial port driver. Great space saver compared to HAL.
 */

#include "SerialDriver.h"

void UART_Transmit(UART_HandleTypeDef *huart, uint8_t *pData, uint16_t Size)
{
    while (Size)
    {
    	while(!__HAL_UART_GET_FLAG(huart, UART_FLAG_TXE));
        huart->Instance->TDR = *pData++;
        Size--;
    }
}


uint8_t UART_GetChar(UART_HandleTypeDef *huart)
{
	while(!__HAL_UART_GET_FLAG(huart, UART_FLAG_RXNE));
	return huart->Instance->RDR & huart->Mask;
}

void UART_StartReceive(UART_HandleTypeDef *huart)
{
	UART_MASK_COMPUTATION(huart);
}
