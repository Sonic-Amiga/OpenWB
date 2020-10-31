#include "main.h"
#include "stm32f0xx_hal.h"
#include "hal_extras.h"
#include "ModbusRTU.h"

static const char model[7]      = "WBMR2";
static const char version[16]   = "0.1";
static const char signature[12] = "OpenWB";

class CountdownTimer
{
public:
	CountdownTimer(uint16_t start_value = 0) : time_count(start_value)
    {}

	void onTick()
	{
	    if (time_count <= uwTickFreq)
	    {
	    	time_count = 0;
			onTimeout();
	    }
		else
		{
			time_count -= uwTickFreq;
		}
	}

	void startTimer(uint32_t timeout)
	{
		time_count = timeout;
	}

	bool isTimedOut() const
	{
		return time_count == 0;
	}

protected:
	virtual void onTimeout() {}

private:
    uint16_t time_count;
};

class UptimeCounter : public CountdownTimer
{
public:
	UptimeCounter() : CountdownTimer(1000), seconds(0) {}

	uint32_t seconds;

protected:
	void onTimeout() override
	{
		seconds++;
		startTimer(1000);
	}
};

class LEDTimer : public CountdownTimer
{
public:
	// LED pin is inverted in hardware
	void Blink()
	{
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
		startTimer(100); // 0.1 sec
	}

protected:
	void onTimeout() override
	{
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
	}
};

class ChannelHandler : public CountdownTimer
{
public:
  ChannelHandler(uint16_t relay, uint16_t input) : relay_pin(relay), input_pin(input) {}

  void onInterrupt();

  void setRelayState(bool state)
  {
	  relay_state = state;
      // Here we hope both relays are located on the same port, true so far
      HAL_GPIO_WritePin(RELAY0_GPIO_Port, relay_pin, (GPIO_PinState)state);
  }

  bool     relay_state = false;
  bool     input_state = false;
  uint16_t debounce    = 100;

private:
  const uint16_t relay_pin;
  const uint16_t input_pin;
};

void ChannelHandler::onInterrupt()
{
	input_state = !input_state;
    if (input_state)
    {
        GPIO_Set_Trigger(input_pin, RISING_EDGE);

        if (!isTimedOut())
            return;

        setRelayState(!relay_state);
    }
    else
    {
    	GPIO_Set_Trigger(input_pin, FALLING_EDGE);
    	startTimer(debounce);
    }
}

static ChannelHandler channel0(RELAY0_Pin, INPUT0_Pin);
static ChannelHandler channel1(RELAY1_Pin, INPUT1_Pin);
static UptimeCounter uptime;
static LEDTimer led;

class WBMR : public ModbusRTUSlave
{
public:
	WBMR(UART_HandleTypeDef *uart) : ModbusRTUSlave(uart, 145) {}

	uint32_t receiveFrame();

protected:
	void onFrameReceived() override
	{
		led.Blink();
	}

	uint32_t validateCoil(uint16_t reg) override;
	uint32_t onWriteCoil(uint16_t reg, bool value) override;
	uint32_t onReadCoil(uint16_t reg) override;
	uint32_t onReadDiscrete(uint16_t reg) override;
	uint32_t onReadInput(uint16_t reg) override;
	uint32_t onReadHolding(uint16_t reg) override;
	uint32_t validateHolding(uint16_t reg, uint16_t value) override;
	uint32_t onWriteHolding(uint16_t reg, uint16_t value) override;

private:
	uint16_t baud_rate = 96;
	uint16_t parity    = 0;
	uint16_t stop_bits = 2;

	bool cfg_changed = false;
};

uint32_t WBMR::validateCoil(uint16_t reg)
{
	switch (reg)
	{
	case 0:
	case 1:
        break;
	default:
		return Result::IllegalDataAddress;
	}

	return Result::OK;
}

uint32_t WBMR::onWriteCoil(uint16_t reg, bool value)
{
	switch (reg)
	{
	case 0:
        channel0.setRelayState(value);
        break;
	case 1:
        channel1.setRelayState(value);
        break;
	}

	return Result::OK;
}

uint32_t WBMR::onReadCoil(uint16_t reg)
{
	switch (reg)
	{
	case 0:
        return channel0.relay_state;
	case 1:
		return channel1.relay_state;;
	default:
		return Result::IllegalDataAddress;
	}
}

uint32_t WBMR::onReadDiscrete(uint16_t reg)
{
	switch (reg)
	{
	case 0:
        return channel0.input_state;
	case 1:
		return channel1.input_state;
	default:
		return Result::IllegalDataAddress;
	}
}

#define STRING_REG(reg, addr, str)                \
    if (reg >= addr && reg <= addr + sizeof(str)) \
        return str[reg - addr]

#define BE32_REG(reg, addr, value) \
	if (reg == addr)               \
        return value >> 16;        \
    if (reg == addr + 1)           \
    	return value & 0x00FF

uint32_t WBMR::onReadInput(uint16_t reg)
{
	BE32_REG(reg, 104, uptime.seconds);
	STRING_REG(reg, 200, model);
	STRING_REG(reg, 250, version);
	STRING_REG(reg, 290, signature);

	return Result::IllegalDataAddress;
}

uint32_t WBMR::onReadHolding(uint16_t reg)
{
	switch (reg)
	{
	case 20:
		return channel0.debounce;
	case 21:
		return channel1.debounce;
	case 110:
		return baud_rate;
	case 111:
		return parity;
	case 112:
		return stop_bits;
	case 128:
		return m_SlaveID;
	default:
		return Result::IllegalDataAddress;
	}
}

uint32_t WBMR::validateHolding(uint16_t reg, uint16_t value)
{
	switch (reg)
	{
	case 20:
	case 21:
		if (value > 100)
			return Result::IllegalDataValue;
		break;
	case 110:
		switch (value)
		{
		case 12:
		case 24:
		case 48:
		case 96:
		case 192:
		case 384:
		case 576:
		case 1152:
			break;
		default:
			return Result::IllegalDataValue;
		}
		break;
	case 111:
		if (value > 2)
			return Result::IllegalDataValue;
	    break;
	case 112:
		if (value < 1 || value > 2)
			return Result::IllegalDataValue;
		break;
	case 128:
		if (value < 1 || value > 247)
			return Result::IllegalDataValue;
		break;
	default:
		return Result::IllegalDataAddress;
	}

	return Result::OK;
}

uint32_t WBMR::onWriteHolding(uint16_t reg, uint16_t value)
{
	switch (reg)
	{
	case 20:
		channel0.debounce = value;
		break;
	case 21:
		channel1.debounce = value;
		break;
	case 110:
		baud_rate   = value;
		cfg_changed = true;
		break;
	case 111:
		parity      = value;
		cfg_changed = true;
		break;
	case 112:
		stop_bits   = value;
		cfg_changed = true;
		break;
	case 128:
		// It's OK to change m_SlaveID during the transaction.
		// The response will be correctly sent from an old address
		m_SlaveID = value;
		break;
	}

	return Result::OK;
}

static const uint32_t stop_bits_table[] =
{
	0,
	UART_STOPBITS_1,
	UART_STOPBITS_2
};

static const uint32_t parity_table[] =
{
	UART_PARITY_NONE,
	UART_PARITY_ODD,
	UART_PARITY_EVEN
};

uint32_t WBMR::receiveFrame()
{
	uint32_t result = ModbusRTUSlave::receiveFrame();

	if (cfg_changed)
	{
		m_uart->Init.BaudRate = baud_rate * 100;
		m_uart->Init.StopBits = stop_bits_table[stop_bits];
		m_uart->Init.Parity   = parity_table[parity];

		UART_SetConfig(m_uart);
		cfg_changed = false;
	}

	return result;
}

static WBMR modbus(&huart1);

void loop(void)
{
	modbus.receiveFrame();
}

void HAL_GPIO_EXTI_Callback(uint16_t pin)
{
	switch (pin)
	{
	case INPUT0_Pin:
		channel0.onInterrupt();
		break;
	case INPUT1_Pin:
		channel1.onInterrupt();
		break;
	}
}

void HAL_IncTick(void)
{
	// Do not forget to drive HAL's own time counter
    uwTick += uwTickFreq;

    channel0.onTick();
    channel1.onTick();
    uptime.onTick();
    led.onTick();
}
