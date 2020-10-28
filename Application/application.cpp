#include "main.h"
#include "stm32f0xx_hal.h"
#include "hal_extras.h"
#include "ModbusRTU.h"

static const char model[7]      = "WBMR2";
static const char version[16]   = "0.1";
static const char signature[12] = "OpenWB";

static uint32_t uptime = 0;

class ChannelHandler
{
public:
  ChannelHandler(uint16_t relay, uint16_t input) : relay_pin(relay), input_pin(input) {}

  void onInterrupt();
  void onTick()
  {
	  if (delay <= uwTickFreq)
		  delay = 0;
	  else
		  delay -= uwTickFreq;
  }

  void setRelayState(bool state)
  {
	  relay_state = state;
      // Here we hope both relays are located on the same port, true so far
      HAL_GPIO_WritePin(RELAY0_GPIO_Port, relay_pin, (GPIO_PinState)state);
  }

  uint32_t setDebounce(uint16_t value)
  {
	  if (value > 100)
		  return ModbusRTUSlave::Result::IllegalDataValue;

	  debounce = value;
	  return ModbusRTUSlave::Result::OK;
  }

  bool     relay_state = false;
  bool     input_state = false;
  uint16_t debounce    = 100;

private:
  const uint16_t relay_pin;
  const uint16_t input_pin;
  uint32_t delay = 0;
};

void ChannelHandler::onInterrupt()
{
	input_state = !input_state;
    if (input_state)
    {
        GPIO_Set_Trigger(input_pin, RISING_EDGE);

        if (delay)
            return;

        setRelayState(!relay_state);
    }
    else
    {
    	GPIO_Set_Trigger(input_pin, FALLING_EDGE);
    	delay = debounce;
    }
}

static ChannelHandler channel0(RELAY0_Pin, INPUT0_Pin);
static ChannelHandler channel1(RELAY1_Pin, INPUT1_Pin);

class WBMR : public ModbusRTUSlave
{
public:
	WBMR(UART_HandleTypeDef *uart) : ModbusRTUSlave(uart, 145) {}

	uint32_t setSlaveID(uint8_t id)
	{
		if (id < 1 || id > 247)
			return Result::IllegalDataValue;

		// It's OK to change m_SlaveID during the transaction.
		// The response will be correctly sent from an old address
		m_SlaveID = id;
		return Result::OK;
	}

protected:
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
};

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
	default:
		return Result::IllegalDataAddress;
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
	BE32_REG(reg, 104, uptime);
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

uint32_t WBMR::onWriteHolding(uint16_t reg, uint16_t value)
{
	switch (reg)
	{
	case 20:
		return channel0.setDebounce(value);
	case 21:
		return channel1.setDebounce(value);
	case 128:
		return setSlaveID(value);
	default:
		return Result::IllegalDataAddress;
	}

	return Result::OK;
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

static uint16_t second_counter = 0;

void HAL_IncTick(void)
{
	// Do not forget to drive HAL's own time counter
    uwTick += uwTickFreq;

    channel0.onTick();
    channel1.onTick();

    second_counter += uwTickFreq;
    if (second_counter >= 1000)
    {
    	uptime++;
    	second_counter = 0;
    }
}
