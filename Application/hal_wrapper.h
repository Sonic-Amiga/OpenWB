/*
 * hal_wrapper.h
 *
 *  Created on: 31 oct 2020
 *      Author: Sonic

 * A minimalistic STM32 StdPeriph -> HAL wrapper for EEPROM emulator
 */

#ifndef HAL_WRAPPER_H_
#define HAL_WRAPPER_H_

#include "stm32f0xx_hal.h"

#define FLASH_Status HAL_StatusTypeDef
#define FLASH_COMPLETE HAL_OK

static inline FLASH_Status FLASH_ErasePage(uint32_t pageAddress)
{
	FLASH_EraseInitTypeDef args;
	uint32_t PageError;

	args.TypeErase = FLASH_TYPEERASE_PAGES;
	args.PageAddress = pageAddress;
	args.NbPages = 1;

	return HAL_FLASHEx_Erase(&args, &PageError);
}

static inline FLASH_Status FLASH_ProgramHalfWord(uint32_t address, uint16_t data)
{
	return HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, address, data);
}

#endif /* HAL_WRAPPER_H_ */
