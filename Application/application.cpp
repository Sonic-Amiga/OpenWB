#include "main.h"
#include "stm32f0xx_hal.h"
#include "hal_extras.h"
#include "ModbusRTU.h"

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
	using ModbusRTUSlave::ModbusRTUSlave;

protected:
	uint32_t onWriteCoil(uint16_t reg, bool value) override;
	uint32_t onReadCoil(uint16_t reg) override;
	uint32_t onReadDiscrete(uint16_t reg) override;
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

static WBMR modbus(&huart1, 145);

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
}
