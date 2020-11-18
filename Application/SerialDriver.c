/*
 * SerialDriver.c
 *
 *  Created on: 18 Nov 2020
 *      Author: Sonic
 *
 * A stripped down STM32 serial port driver, tailored for our needs:
 * - Polled mode Tx
 * - Interrupt mode Rx
 * - 8 data bits only
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

	UART_StopReceive(uart);
	LL_USART_Disable(uart);

    USART_InitStruct.BaudRate            = baud;
	USART_InitStruct.DataWidth           = (parity == 0) ? LL_USART_DATAWIDTH_8B : LL_USART_DATAWIDTH_9B;
	USART_InitStruct.StopBits            = (stop_bits == 2) ? LL_USART_STOPBITS_2 : LL_USART_STOPBITS_1;
	USART_InitStruct.Parity              = parity_table[parity];
	USART_InitStruct.TransferDirection   = LL_USART_DIRECTION_TX_RX;
	USART_InitStruct.HardwareFlowControl = LL_USART_HWCONTROL_NONE;
	USART_InitStruct.OverSampling        = LL_USART_OVERSAMPLING_16;

	LL_USART_Init(uart, &USART_InitStruct);
	LL_USART_Enable(uart);
	UART_StartReceive(uart);
}

void UART_Transmit(USART_TypeDef* uart, uint8_t *pData, uint16_t Size)
{
    while (Size)
    {
    	while(!LL_USART_IsActiveFlag_TXE(uart));
    	LL_USART_TransmitData8(uart, *pData++);
        Size--;
    }

    /* Wait for transfer completion */
    while(!LL_USART_IsActiveFlag_TC(uart));
}

void UART_StartReceive(USART_TypeDef *uart)
{
    /* Enable the UART Error Interrupt: (Frame error, noise error, overrun error) */
	LL_USART_EnableIT_ERROR(uart);

    /* Enable the UART Parity Error interrupt and Data Register Not Empty interrupt */
    SET_BIT(uart->CR1, USART_CR1_PEIE | USART_CR1_RXNEIE);
}

void UART_StopReceive(USART_TypeDef *uart)
{
    /* Disable the UART Parity Error Interrupt and RXNE interrupts */
    CLEAR_BIT(uart->CR1, (USART_CR1_RXNEIE | USART_CR1_PEIE));

    /* Disable the UART Error Interrupt: (Frame error, noise error, overrun error) */
    LL_USART_DisableIT_ERROR(uart);
}

void UART_IRQHandler(USART_TypeDef *huart)
{
    uint32_t isrflags   = READ_REG(huart->ISR);
    uint32_t cr1its     = READ_REG(huart->CR1);
    uint32_t cr3its     = READ_REG(huart->CR3);

    uint8_t  data_ready = (isrflags & USART_ISR_RXNE) && (cr1its & USART_CR1_RXNEIE);
    uint32_t errorflags = isrflags & (USART_ISR_PE | USART_ISR_FE | USART_ISR_ORE | USART_ISR_NE | USART_ISR_RTOF);

    /* If no error occurs */
    if (errorflags == 0U)
    {
        /* UART in mode Receiver ---------------------------------------------------*/
        if (data_ready)
		{
        	UART_RxISR(huart);
			return;
		}
	}

    /* If some errors occur */
    if (errorflags && ((cr3its & USART_CR3_EIE) || (cr1its & (USART_CR1_RXNEIE | USART_CR1_PEIE))))
    {
	    uint32_t errorcode = 0;

        /* UART parity error interrupt occurred -------------------------------------*/
        if ((isrflags & USART_ISR_PE) && (cr1its & USART_CR1_PEIE))
        {
            LL_USART_ClearFlag_PE(huart);
            errorcode |= USART_ISR_PE;
        }

        /* UART frame error interrupt occurred --------------------------------------*/
        if ((isrflags & USART_ISR_FE) && (cr3its & USART_CR3_EIE))
        {
    	    LL_USART_ClearFlag_FE(huart);
    	    errorcode |= USART_ISR_FE;
        }

        /* UART noise error interrupt occurred --------------------------------------*/
        if ((isrflags & USART_ISR_NE) && (cr3its & USART_CR3_EIE))
        {
    	    LL_USART_ClearFlag_NE(huart);
    	    errorcode |= USART_ISR_NE;
        }

        /* UART Over-Run interrupt occurred -----------------------------------------*/
        if (((isrflags & USART_ISR_ORE) && ((cr1its & USART_CR1_RXNEIE) || (cr3its & USART_CR3_EIE))))
        {
            LL_USART_ClearFlag_ORE(huart);
            errorcode |= USART_ISR_ORE;
        }

        /* UART Receiver Timeout interrupt occurred ---------------------------------*/
        if ((isrflags & USART_ISR_RTOF) && (cr1its & USART_CR1_RTOIE))
        {
    	    LL_USART_ClearFlag_RTO(huart);
    	    errorcode |= USART_ISR_RTOF;
        }

        /* Call UART Error Call back function if need be ----------------------------*/
        if (errorcode)
        {
            /* UART in mode Receiver --------------------------------------------------*/
            if (data_ready)
            {
                UART_RxISR(huart);
            }

            UART_ErrorCallback(huart);
            return;
        }
    } /* End if some error occurs */
#if defined(USART_CR1_UESM)

    /* UART wakeup from Stop mode interrupt occurred ---------------------------*/
    if ((isrflags & USART_ISR_WUF) && (cr3its & USART_CR3_WUFIE))
    {
    	LL_USART_ClearFlag_WKUP(huart);

        /* Call registered Wakeup Callback */
        huart->WakeupCallback(huart);
        return;
    }
#endif /* USART_CR1_UESM */
}
