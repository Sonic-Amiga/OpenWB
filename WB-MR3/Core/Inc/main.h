/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_hal.h"
#include "stm32f0xx_ll_crs.h"
#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_exti.h"
#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_pwr.h"
#include "stm32f0xx_ll_dma.h"
#include "stm32f0xx_ll_tim.h"
#include "stm32f0xx_ll_usart.h"
#include "stm32f0xx_ll_gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define EEPROM_START_ADDRESS  ((uint32_t)0x08007800) /* EEPROM emulation start address:
                                                        last two kilobytes of Flash memory */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define INPUT0_Pin GPIO_PIN_5
#define INPUT0_GPIO_Port GPIOA
#define INPUT0_EXTI_IRQn EXTI4_15_IRQn
#define INPUT1_Pin GPIO_PIN_6
#define INPUT1_GPIO_Port GPIOA
#define INPUT1_EXTI_IRQn EXTI4_15_IRQn
#define INPUT2_Pin GPIO_PIN_7
#define INPUT2_GPIO_Port GPIOA
#define INPUT2_EXTI_IRQn EXTI4_15_IRQn
#define INPUT6_Pin GPIO_PIN_0
#define INPUT6_GPIO_Port GPIOB
#define INPUT6_EXTI_IRQn EXTI0_1_IRQn
#define RELAY0_Pin GPIO_PIN_10
#define RELAY0_GPIO_Port GPIOA
#define RELAY1_Pin GPIO_PIN_11
#define RELAY1_GPIO_Port GPIOA
#define RELAY2_Pin GPIO_PIN_12
#define RELAY2_GPIO_Port GPIOA
#define LED_Pin GPIO_PIN_7
#define LED_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
