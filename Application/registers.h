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

/* Discrete input registers */
#define REG_BUTTON_0 0
#define REG_BUTTON_1 1

/* Data input registers */
#define REG_UPTIME    104
#define REG_MODEL     200
#define REG_VERSION   250
#define REG_SIGNATURE 290

/* Holding registers */
#define REG_DEBOUNCE_0 20
#define REG_DEBOUNCE_1 21
#define REG_BAUD_RATE  110
#define REG_PARITY     111
#define REG_STOP_BITS  112
#define REG_SLAVE_ADDR 128

#endif /* REGISTERS_H_ */
