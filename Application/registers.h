/*
 * registers.h
 *
 *  Created on: 8 nov 2020
 *      Author: Sonic
 *
 * WirenBoard registers definition
 */

#ifndef REGISTERS_H_
#define REGISTERS_H_

/* Coil registers */
#define REG_RELAY_0 0
#define REG_RELAY_1 1
#define REG_RELAY_2 2
#define REG_RELAY_3 3
#define REG_RELAY_4 4
#define REG_RELAY_5 5

/* Discrete input registers */
/* For historical reasons "common control" input, which is labeled "0",
 * is number 5 for us. Relays and their individual buttons are counted from 0.
 */
#define REG_BUTTON_0 0
#define REG_BUTTON_1 1
#define REG_BUTTON_2 2
#define REG_BUTTON_3 3
#define REG_BUTTON_4 4
#define REG_BUTTON_5 5
#define REG_BUTTON_6 6
#define REG_COUNT0   32
#define REG_COUNT1   33
#define REG_COUNT2   34
#define REG_COUNT3   35
#define REG_COUNT4   36
#define REG_COUNT5   37
#define REG_COUNT6   38

/* Data input registers */
#define REG_UPTIME    104
#define REG_MODEL     200
#define REG_COMMIT    220
#define REG_VERSION   250
#define REG_SIGNATURE 290

/* Holding registers */
#define REG_DEBOUNCE_0 20
#define REG_DEBOUNCE_1 21
#define REG_DEBOUNCE_2 22
#define REG_DEBOUNCE_4 23
#define REG_DEBOUNCE_5 23
#define REG_DEBOUNCE_6 23
#define REG_BAUD_RATE  110
#define REG_PARITY     111
#define REG_STOP_BITS  112
#define REG_REBOOT     120
#define REG_SLAVE_ADDR 128

#endif /* REGISTERS_H_ */
