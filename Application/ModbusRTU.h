/*
 * Based on ModbusRTU library by norgor: https://github.com/norgor/ModbusRTU.git
 */

#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H

#include "SerialDriver.h"

#define MODBUS_MAX_FRAME_LENGTH 256

class ModbusRTUSlave
{
private:
	enum ExceptionCode : uint8_t
	{
		Illegal_Function = 1,
		Illegal_DataAddress = 2,
		Illegal_DataValue = 3,
		Device_Failure = 4,
	};

public:
	enum Result : uint32_t
	{
		OK = 0,
		ValueMask = 0x0000FFFF,
		ErrorFlag = 0x80000000,
		IllegalFunction = ErrorFlag + ExceptionCode::Illegal_Function,
		IllegalDataAddress = ErrorFlag + ExceptionCode::Illegal_DataAddress,
		IllegalDataValue = ErrorFlag + ExceptionCode::Illegal_DataValue,
		DeviceFailure = ErrorFlag + ExceptionCode::Device_Failure
	};

	ModbusRTUSlave(UART_HandleTypeDef *uart, uint8_t slaveId)
        : m_uart(uart), m_SlaveID(slaveId), m_InputFrameLength(0), m_RxActive(true)
	{
	}

	void begin(uint32_t baud, uint16_t parity, uint16_t stop_bits);
	void update();
	void receiveByte(uint8_t data);

	void clearInputFrame()
	{
		m_RxActive = true;
		m_InputFrameLength = 0;
	}

protected:
	// Signal that a frame has been received. You can use this in order
	// to e. g. drive activity indicator.
	virtual void onFrameReceived() {};

	virtual uint32_t onReadCoil(uint16_t reg) {
		return Result::IllegalDataAddress;
	}

	virtual uint32_t validateCoil(uint16_t reg) {
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

	virtual uint32_t validateHolding(uint16_t reg, uint16_t value) {
		return Result::IllegalDataAddress;
	}

	virtual uint32_t onWriteHolding(uint16_t reg, uint16_t value) {
		return Result::IllegalDataAddress;
	}

	UART_HandleTypeDef *m_uart;
	uint8_t             m_SlaveID;

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

	uint8_t  m_InputFrame[MODBUS_MAX_FRAME_LENGTH];
	uint8_t  m_OutputFrame[MODBUS_MAX_FRAME_LENGTH];
	uint8_t  m_InputFrameLength;
	bool     m_RxActive;
	uint32_t m_RxTimeout;

	static uint16_t crc16(const uint8_t *nData, uint16_t wLength);

	void initResponse(uint8_t function, uint8_t dataLength)
	{
        m_OutputFrame[0] = m_SlaveID;
        m_OutputFrame[1] = function;
        m_OutputFrame[2] = dataLength;
	}

	uint8_t responseFromFrame(const uint8_t* frame)
	{
		initResponse(frame[1], frame[2]);
        m_OutputFrame[3] = frame[3];
        m_OutputFrame[4] = frame[4];
        m_OutputFrame[5] = frame[5];

        return 3;
	}

	//Parses input frame
	//
	//
	uint32_t parseFrame(uint8_t *frame, uint16_t frameLength);
};

#endif
