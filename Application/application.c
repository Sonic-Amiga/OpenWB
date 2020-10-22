#include "arduino.h"
#include "main.h"
#include "stm32f0xx_hal.h"

void loop(void)
{
	HAL_GPIO_WritePin(LED_GPIO_Port, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(LED_GPIO_Port, GPIO_PIN_4, GPIO_PIN_RESET);
    HAL_Delay(500);
}
