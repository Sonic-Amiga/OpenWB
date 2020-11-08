/*
 * CountdownTimer.cpp
 *
 *  Created on: 8 нояб. 2020 г.
 *      Author: Sonic
 */

#include "CountdownTimer.h"

static CountdownTimer* timers_list = nullptr;

CountdownTimer::CountdownTimer(uint16_t start_value) : time_count(start_value)
{
	next_timer = timers_list;
	timers_list = this;
}

void HAL_IncTick(void)
{
	// Do not forget to drive HAL's own time counter
    uwTick += uwTickFreq;

    for (CountdownTimer* t = timers_list; t; t = t->next_timer)
    	t->onTick();
}
