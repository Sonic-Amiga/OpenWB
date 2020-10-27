#include "arduino.h"
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
	ExceptionCode onWriteCoil(uint16_t reg, bool value) override;
	ExceptionCode onReadCoil(uint16_t reg, bool& value) override;
	ExceptionCode onReadDiscrete(uint16_t reg, bool& value) override;
};

ModbusRTUSlave::ExceptionCode WBMR::onWriteCoil(uint16_t reg, bool value)
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
		return ExceptionCode::IllegalDataAddress;
	}

	return ExceptionCode::OK;
}

ModbusRTUSlave::ExceptionCode WBMR::onReadCoil(uint16_t reg, bool& value)
{
	switch (reg)
	{
	case 0:
        value = channel0.relay_state;
        break;
	case 1:
		value = channel1.relay_state;;
        break;
	default:
		return ExceptionCode::IllegalDataAddress;
	}

	return ExceptionCode::OK;
}

ModbusRTUSlave::ExceptionCode WBMR::onReadDiscrete(uint16_t reg, bool& value)
{
	switch (reg)
	{
	case 0:
        value = channel0.input_state;
        break;
	case 1:
		value = channel1.input_state;;
        break;
	default:
		return ExceptionCode::IllegalDataAddress;
	}

	return ExceptionCode::OK;
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
