/*
 * Based on ModbusRTU library by norgor: https://github.com/norgor/ModbusRTU.git
 */

#include <ModbusRTU.h>

#define BIT_SET(variable, bit, value) \
    if (value)                        \
        ((variable) |= (1<<(bit)));   \
    else                              \
        ((variable) &= ~(1<<(bit)))

#define BIT_CHECK(variable, bit) (variable & (1 << bit))

static inline uint16_t read_unaligned_le16(uint8_t* ptr)
{
	return ptr[0] | (((uint16_t)ptr[1]) << 8);
}

static inline void write_unaligned_le16(uint8_t* ptr, uint16_t value)
{
	ptr[0] = value;
	ptr[1] = value >> 8;
}

static inline uint16_t read_unaligned_be16(uint8_t* ptr)
{
	return (((uint16_t)ptr[0]) << 8) | ptr[1];
}

static inline void write_unaligned_be16(uint8_t* ptr, uint16_t value)
{
	ptr[0] = value >> 8;
	ptr[1] = value;
}

static inline uint16_t div_roundup(uint16_t a, uint16_t b)
{
	return (a + b - 1) / b;
}

uint16_t ModbusRTUSlave::crc16(const uint8_t *nData, uint16_t wLength)
{
	static const uint16_t wCRCTable[] = {
		0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
		0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
		0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
		0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
		0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
		0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
		0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
		0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
		0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
		0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
		0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
		0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
		0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
		0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
		0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
		0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
		0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
		0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
		0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
		0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
		0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
		0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
		0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
		0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
		0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
		0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
		0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
		0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
		0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
		0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
		0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
		0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040 };

	uint16_t wCRCWord = 0xFFFF;

	while (wLength--)
	{
		uint8_t nTemp = *nData++ ^ wCRCWord;

		wCRCWord >>= 8;
		wCRCWord ^= wCRCTable[nTemp];
	}

	return wCRCWord;
}

uint32_t ModbusRTUSlave::receiveFrame()
{
	// FIXME: Just some value for testing (500 ms).
	// The spec mandates this to be 1.5 chars or 750us when baudrate >= 19200
	uint32_t timeout = 500;
	uint16_t length;
	HAL_StatusTypeDef result;

	do
	{
		// Minimum modbus frame size:
		// 1 byte  - slave ID
		// 1 byte  - function code
		// 2 bytes - starting address of the register
		// 2 bytes - quantity of registers to read
		// 2 bytes - CRC
		length = 8;

		result = HAL_UART_Receive(m_uart, m_InputFrame, length, timeout);
		if (result != HAL_OK)
			continue;

		//Function length check
		if (m_InputFrame[1] >= FunctionCode::ReadCoils &&
			m_InputFrame[1] <= FunctionCode::WriteSingleRegister)
		{
			// parse
		}
		else if (m_InputFrame[1] == FunctionCode::WriteMultipleCoils ||
				m_InputFrame[1] == FunctionCode::WriteMultipleRegisters)
		{
			// 1 byte - subsequent data length in bytes
			uint16_t extra_len = m_InputFrame[6] + 1;

			result = HAL_UART_Receive(m_uart, &m_InputFrame[8], extra_len, timeout);

			if (result != HAL_OK)
				continue; // Restart from the beginning if timeout

			length += extra_len;
		}
		else
		{
			continue; // Some rubbish received, just drop
		}

		// Zero is broadcast address according to spec
		if (m_InputFrame[0] && m_InputFrame[0] != m_SlaveID)
			continue;

	} while (crc16(m_InputFrame, length - 2) != read_unaligned_le16(&m_InputFrame[length - 2]));

	onFrameReceived();

    return parseFrame(m_InputFrame, length);
}

void ModbusRTUSlave::sendFrame(uint8_t *pFrame, uint8_t frameLength)
{
	write_unaligned_le16(&pFrame[frameLength], crc16(pFrame, frameLength));
	send(pFrame, frameLength + 2);
}

uint32_t ModbusRTUSlave::parseFrame(uint8_t *frame, uint16_t frameLength)
{
	uint8_t function = frame[1];
	uint16_t targetRegister, targetRegisterLength, data;
	uint8_t responseLength = 0;
	uint32_t result = 0;

	switch (function)
	{
	case ReadCoils:
        targetRegister = read_unaligned_be16(&frame[2]);
        targetRegisterLength = read_unaligned_be16(&frame[4]);
        responseLength = div_roundup(targetRegisterLength, 8);

        // Write frame header
        initResponse(function, responseLength);

        // Loop through requested registers
        for (uint16_t i = 0; i < targetRegisterLength; i++)
        {
        	result = onReadCoil(targetRegister + i);
        	if (result & Result::ErrorFlag)
        		break;

            // Write register value to response frame
            BIT_SET(m_OutputFrame[3 + (i / 8)], i % 8, result);
        }

        break;

	case WriteSingleCoil:
        targetRegister = read_unaligned_be16(&frame[2]);
        responseLength = responseFromFrame(frame);

        result = validateCoil(targetRegister);
        if (result == Result::OK)
            result = onWriteCoil(targetRegister, !!read_unaligned_be16(&frame[4]));

        break;

    case WriteMultipleCoils:
        targetRegister = read_unaligned_be16(&frame[2]);
        targetRegisterLength = read_unaligned_be16(&frame[4]);
        responseLength = responseFromFrame(frame);

        for (uint16_t i = 0; i < targetRegisterLength; i++)
        {
        	result = validateCoil(targetRegister + i);
            if (result != Result::OK)
            	break;
        }

        if (result == Result::OK)
        {
            for (uint16_t i = 0; i < targetRegisterLength; i++)
            {
                result = onWriteCoil(targetRegister + i, BIT_CHECK(frame[7 + (i / 8)], i % 8));
                if (result != Result::OK)
                	break;
            }
        }

        break;

    case ReadDiscreteInputs:
        targetRegister = read_unaligned_be16(&frame[2]);
        targetRegisterLength = read_unaligned_be16(&frame[4]);
        responseLength = div_roundup(targetRegisterLength, 8);

        initResponse(function, responseLength);

        // Loop through requested registers
        for (uint16_t i = 0; i < targetRegisterLength; i++)
        {
        	result = onReadDiscrete(targetRegister + i);
            if (result & Result::ErrorFlag)
            	break;

            // Write register value to response frame
            BIT_SET(m_OutputFrame[3 + (i / 8)], i % 8, result);
        }

        break;

    case ReadMultipleHoldingRegisters:
        targetRegister = read_unaligned_be16(&frame[2]);
        targetRegisterLength = read_unaligned_be16(&frame[4]);
        responseLength = targetRegisterLength * 2;

        initResponse(function, responseLength);

        // Loop through requested registers
        for (uint16_t i = 0; i < targetRegisterLength; i++)
        {
            result = onReadHolding(targetRegister + i);
            if (result & Result::ErrorFlag)
            	break;

            write_unaligned_be16(&m_OutputFrame[3 + (i * 2)], result);
        }

        break;

    case WriteSingleRegister:
        targetRegister = read_unaligned_be16(&frame[2]);
        responseLength = responseFromFrame(frame);

        data = read_unaligned_be16(&frame[4]);
        result = validateHolding(targetRegister, data);
        if (result == Result::OK)
            result = onWriteHolding(targetRegister, data);

        break;

    case WriteMultipleRegisters:
        targetRegister = read_unaligned_be16(&frame[2]);
        targetRegisterLength = read_unaligned_be16(&frame[4]);
        responseLength = responseFromFrame(frame);

        for (uint16_t i = 0; i < targetRegisterLength; i++)
        {
        	data = read_unaligned_be16(&frame[7 + (i * 2)]);
            result = validateHolding(targetRegister + i, data);
            if (result != Result::OK)
            	break;
        }

        if (result == Result::OK)
        {
            for (uint16_t i = 0; i < targetRegisterLength; i++)
            {
        	    data = read_unaligned_be16(&frame[7 + (i * 2)]);
                result = onWriteHolding(targetRegister + i, data);
                if (result & Result::ErrorFlag)
                	break;
            }
        }

        break;

    case ReadInputRegisters:
        targetRegister = read_unaligned_be16(&frame[2]);
        targetRegisterLength = read_unaligned_be16(&frame[4]);
        responseLength = targetRegisterLength * 2;

        initResponse(function, responseLength);

        // Loop through requested registers
        for (uint16_t i = 0; i < targetRegisterLength; i++)
        {
        	result = onReadInput(targetRegister + i);
            if (result & Result::ErrorFlag)
            	break;

            write_unaligned_be16(&m_OutputFrame[3 + (i * 2)], result);
        }

        break;
    }

    // Send response frame to master
    if (result & Result::ErrorFlag)
    {
    	initResponse(m_InputFrame[1] + FunctionCode::Exception, result & Result::ValueMask);
    	sendFrame(m_OutputFrame, 3);
    }
    else
    {
        sendFrame(m_OutputFrame, responseLength + 3);
    }

    return result;
}
