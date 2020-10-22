#include "arduino.h"
#include "main.h"
#include "stm32f0xx_hal.h"
#include "hal_extras.h"

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

        relay_state = !relay_state;
        // Here we hope both relays are located on the same port, true so far
        HAL_GPIO_WritePin(RELAY0_GPIO_Port, relay_pin, (GPIO_PinState)relay_state);
    }
    else
    {
    	GPIO_Set_Trigger(input_pin, FALLING_EDGE);
    	delay = debounce;
    }
}

ChannelHandler channel0(RELAY0_Pin, INPUT0_Pin);
ChannelHandler channel1(RELAY1_Pin, INPUT1_Pin);

void loop(void)
{
	HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);
    HAL_Delay(500);
    HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
    HAL_Delay(500);
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
