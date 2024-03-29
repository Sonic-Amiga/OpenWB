/*
 * application.h
 *
 *  Created on: 10 мар. 2022 г.
 *      Author: Sonic
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#ifdef __cplusplus
extern "C" {
#endif

// Arduino-style entry points
void setup(void);
void loop(void);

// Device model string
// This should be defined somewhere in target-specific code
extern const char model_str[7];

#ifdef __cplusplus
}
#endif

#endif /* APPLICATION_H_ */
