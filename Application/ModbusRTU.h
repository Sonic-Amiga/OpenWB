/*
 * Based on ModbusRTU library by norgor: https://github.com/norgor/ModbusRTU.git
 */

#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include "stm32f0xx_hal.h"

#define MODBUS_MAX_FRAME_LENGTH 255

class ModbusRTUSlave
{
private:
	enum ExceptionCode : uint8_t
	{
		Illegal_Function = 1,
		Illegal_DataAddress = 2,
		Illegal_DataValue = 3,
	};

public:
	enum Result : uint32_t
	{
		OK = 0,
		ValueMask = 0x0000FFFF,
		ErrorFlag = 0x80000000,
		IllegalFunction = ErrorFlag + ExceptionCode::Illegal_Function,
		IllegalDataAddress = ErrorFlag + ExceptionCode::Illegal_DataAddress,
		IllegalDataValue = ErrorFlag + ExceptionCode::Illegal_DataValue
	};

	ModbusRTUSlave(UART_HandleTypeDef *uart, uint8_t slaveId)
        : m_SlaveID(slaveId), m_uart(uart)
	{
	}

	uint32_t receiveFrame();

protected:
	virtual uint32_t onReadCoil(uint16_t reg) {
		return Result::IllegalDataAddress;
	}

	virtual uint32_t onWriteCoil(uint16_t reg, bool value) {
		return Result::IllegalDataAddress;
	}

	virtual uint32_t onReadDiscrete(uint16_t reg) {
		return Result::IllegalDataAddress;
	}

	virtual uint32_t onReadInput(uint16_t reg) {
		return Result::IllegalDataAddress;
	}

	virtual uint32_t onReadHolding(uint16_t reg) {
		return Result::IllegalDataAddress;
	}

	virtual uint32_t onWriteHolding(uint16_t reg, uint16_t value) {
		return Result::IllegalDataAddress;
	}

	uint8_t m_SlaveID;

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
	uint8_t             m_InputFrame[MODBUS_MAX_FRAME_LENGTH];
	uint8_t             m_OutputFrame[MODBUS_MAX_FRAME_LENGTH];

	static uint16_t crc16(const uint8_t *nData, uint16_t wLength);

	void initResponse(uint8_t function, uint8_t dataLength)
	{
        m_OutputFrame[0] = m_SlaveID;
        m_OutputFrame[1] = function;
        m_OutputFrame[2] = dataLength;
	}

	uint8_t responseFromFrame(const uint8_t* frame)
	{
        m_OutputFrame[0] = m_SlaveID;
        m_OutputFrame[1] = frame[1];
        m_OutputFrame[2] = frame[2];
        m_OutputFrame[3] = frame[3];
        m_OutputFrame[4] = frame[4];
        m_OutputFrame[5] = frame[5];

        return 3;
	}

	// Sends exception to master
	// Uses last received frame as function code
	//
	void throwException(uint8_t exceptionCode);

	//Send response frame to master
	//This function appends a CRC16 to the end of the frame
	//
	void sendFrame(uint8_t *pFrame, uint8_t frameLength);

	//Parses input frame
	//
	//
	uint32_t parseFrame(uint8_t *frame, uint16_t frameLength);

	void send(uint8_t* frame, uint16_t length) {
	    HAL_UART_Transmit(m_uart, frame, length, 0xFFFF);
	}
};

#endif
