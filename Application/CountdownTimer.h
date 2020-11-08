/*
 * CountdownTimer.h
 *
 *  Created on: 8 нояб. 2020 г.
 *      Author: Sonic
 */

#ifndef COUNTDOWNTIMER_H_
#define COUNTDOWNTIMER_H_

#include "stm32f0xx_hal.h"

class CountdownTimer
{
public:
	CountdownTimer(uint16_t start_value = 0);

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

    uint16_t time_count;
    CountdownTimer* next_timer;

    friend void HAL_IncTick(void);
};

#endif /* COUNTDOWNTIMER_H_ */
