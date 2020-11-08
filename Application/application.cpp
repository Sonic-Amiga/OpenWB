#include "main.h"
#include "stm32f0xx_hal.h"
#include "hal_extras.h"
#include "ModbusRTU.h"
#include "CountdownTimer.h"
#include "registers.h"

extern "C" { // This header is pure C
#include "eeprom.h"
}

static const char model[7]      = "WBMR2";
static const char version[16]   = "0.1";
static const char signature[12] = "OpenWB";

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
  uint16_t debounce    = 50;

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

	uint32_t validateHolding(uint16_t reg, uint16_t value) override;
	bool applyHolding(uint16_t reg, uint16_t value);

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
	case REG_RELAY_0:
	case REG_RELAY_1:
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
	case REG_RELAY_0:
        channel0.setRelayState(value);
        break;
	case REG_RELAY_1:
        channel1.setRelayState(value);
        break;
	}

	return Result::OK;
}

uint32_t WBMR::onReadCoil(uint16_t reg)
{
	switch (reg)
	{
	case REG_RELAY_0:
        return channel0.relay_state;
	case REG_RELAY_1:
		return channel1.relay_state;;
	default:
		return Result::IllegalDataAddress;
	}
}

uint32_t WBMR::onReadDiscrete(uint16_t reg)
{
	switch (reg)
	{
	case REG_BUTTON_0:
        return channel0.input_state;
	case REG_BUTTON_1:
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
        return (value) >> 16;      \
    if (reg == addr + 1)           \
    	return (value) & 0x00FF

uint32_t WBMR::onReadInput(uint16_t reg)
{
	BE32_REG(reg,   REG_UPTIME,    uptime.seconds);
	STRING_REG(reg, REG_MODEL,     model);
	STRING_REG(reg, REG_VERSION,   version);
	STRING_REG(reg, REG_SIGNATURE, signature);

	return Result::IllegalDataAddress;
}

uint32_t WBMR::onReadHolding(uint16_t reg)
{
	switch (reg)
	{
	case REG_DEBOUNCE_0:
		return channel0.debounce;
	case REG_DEBOUNCE_1:
		return channel1.debounce;
	case REG_BAUD_RATE:
		return baud_rate;
	case REG_PARITY:
		return parity;
	case REG_STOP_BITS:
		return stop_bits;
	case REG_SLAVE_ADDR:
		return m_SlaveID;
	default:
		return Result::IllegalDataAddress;
	}
}

uint32_t WBMR::validateHolding(uint16_t reg, uint16_t value)
{
	switch (reg)
	{
	case REG_DEBOUNCE_0:
	case REG_DEBOUNCE_1:
		if (value > 100)
			return Result::IllegalDataValue;
		break;
	case REG_BAUD_RATE:
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
	case REG_PARITY:
		if (value > 2)
			return Result::IllegalDataValue;
	    break;
	case REG_STOP_BITS:
		if (value < 1 || value > 2)
			return Result::IllegalDataValue;
		break;
	case REG_SLAVE_ADDR:
		if (value < 1 || value > 247)
			return Result::IllegalDataValue;
		break;
	default:
		return Result::IllegalDataAddress;
	}

	return Result::OK;
}

bool WBMR::applyHolding(uint16_t reg, uint16_t value)
{
	switch (reg)
	{
	case REG_DEBOUNCE_0:
		if (value == channel0.debounce)
			return false;
		channel0.debounce = value;
		break;
	case REG_DEBOUNCE_1:
		if (value == channel1.debounce)
			return false;
		channel1.debounce = value;
		break;
	case REG_BAUD_RATE:
		if (value == baud_rate)
			return false;
		baud_rate   = value;
	    cfg_changed = true;
		break;
	case REG_PARITY:
		if (value == parity)
			return false;
	    parity      = value;
	    cfg_changed = true;
		break;
	case REG_STOP_BITS:
		if (stop_bits == value)
			return false;
	    stop_bits   = value;
	    cfg_changed = true;
		break;
	case REG_SLAVE_ADDR:
		if (m_SlaveID != value)
			return false;
		// It's OK to change m_SlaveID during the transaction.
		// The response will be correctly sent from an old address
		m_SlaveID = value;
		break;
	}

	return true;
}

uint32_t WBMR::onWriteHolding(uint16_t reg, uint16_t value)
{
	bool changed = applyHolding(reg, value);

	if (changed) {
	    /* Temporarily disabled for debugging
		if (EE_WriteVariable(reg, value) != FLASH_COMPLETE)
	        return Result::DeviceFailure;
        */
	}

	return Result::OK;
}

static const uint32_t parity_table[] =
{
	UART_PARITY_NONE,
	UART_PARITY_ODD,
	UART_PARITY_EVEN
};

uint32_t WBMR::receiveFrame()
{
	if (cfg_changed)
	{
		m_uart->Init.BaudRate = baud_rate * 100;
		m_uart->Init.StopBits = (stop_bits == 2) ? UART_STOPBITS_2 : UART_STOPBITS_1;
		m_uart->Init.Parity   = parity_table[parity];

		UART_SetConfig(m_uart);
		cfg_changed = false;
	}

	return ModbusRTUSlave::receiveFrame();
}

static WBMR modbus(&huart1);

// We are also using ModBus holding register numbers as virtual addresses for EEPROM emulation
// This allows us to save up some space by completely reusing the validation code
const uint16_t VirtAddVarTab[NB_OF_VAR] = {
	REG_DEBOUNCE_0,
	REG_DEBOUNCE_1,
	REG_BAUD_RATE,
	REG_PARITY,
	REG_STOP_BITS,
	REG_SLAVE_ADDR
};

void setup(void)
{
	for (uint16_t i = 0; i < NB_OF_VAR; i++)
	{
		uint16_t reg = VirtAddVarTab[i];
		uint16_t data;

		if (EE_ReadVariable(reg, &data) == 0) {
            if (modbus.validateHolding(reg, data) == ModbusRTUSlave::Result::OK)
            	modbus.applyHolding(reg, data);
		}
	}
}

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
