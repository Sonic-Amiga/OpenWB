/*
 * Based on ModbusRTU library by norgor: https://github.com/norgor/ModbusRTU.git
 */

#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include "stm32f0xx_hal.h"

#define MODBUS_MAX_FRAME_LENGTH 255

class ModbusRTUSlave
{
public:
	enum ExceptionCode : uint8_t
	{
		OK = 0,
		IllegalFunction = 1,
		IllegalDataAddress = 2,
		IllegalDataValue = 3,
	};

	ModbusRTUSlave(UART_HandleTypeDef *uart, uint8_t slaveId)
        : m_uart(uart), m_SlaveID(slaveId)
	{
	}

	bool receiveFrame();

protected:
	virtual ExceptionCode onReadCoil(uint16_t reg, bool& value) {
		return ExceptionCode::IllegalDataAddress;
	}

	virtual ExceptionCode onWriteCoil(uint16_t reg, bool value) {
		return ExceptionCode::IllegalDataAddress;
	}

	virtual ExceptionCode onReadDiscrete(uint16_t reg, bool &value) {
		return ExceptionCode::IllegalDataAddress;
	}

	virtual ExceptionCode onReadInput(uint16_t reg, uint16_t &value) {
		return ExceptionCode::IllegalDataAddress;
	}

	virtual ExceptionCode onReadHolding(uint16_t reg, uint16_t &value) {
		return ExceptionCode::IllegalDataAddress;
	}

	virtual ExceptionCode onWriteHolding(uint16_t reg, uint16_t value) {
		return ExceptionCode::IllegalDataAddress;
	}

private:
	enum FunctionCode : uint8_t
	{
		ReadCoils = 1,
		ReadDiscreteInputs = 2,
		ReadMultipleHoldingRegisters = 3,
		ReadInputRegisters = 4,
		WriteSingleCoil = 5,
		WriteSingleRegister = 6,
		WriteMultipleCoils = 15,
		WriteMultipleRegisters = 16,
		Exception = 128,
	};

	UART_HandleTypeDef *m_uart;
	uint8_t             m_SlaveID;
	uint8_t             m_InputFrame[MODBUS_MAX_FRAME_LENGTH];
	uint8_t             m_OutputFrame[MODBUS_MAX_FRAME_LENGTH];

	static uint16_t crc16(const uint8_t *nData, uint16_t wLength);

	//Checks if frame has been corrupted
	//
	//
	bool isFrameCorrupted(uint8_t *frame, uint16_t frameLength)
	{
		// Check if CRC16 is the same
		return (crc16(frame, frameLength - 2) != *(short*)&frame[frameLength - 2]);
	}

	// Sends exception to master
	// Uses last received frame as function code
	//
	void throwException(ExceptionCode exceptionCode);


	//Send response frame to master
	//This function appends a CRC16 to the end of the frame
	//
	void sendFrame(uint8_t *pFrame, uint8_t frameLength);

	//Parses input frame
	//
	//
	void parseFrame(uint8_t *frame, uint16_t frameLength);

	void send(uint8_t* frame, uint16_t length) {
	    HAL_UART_Transmit(m_uart, frame, 8, 0xFFFF);
	}
};

#endif
