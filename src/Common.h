/*
 * Common.h
 *
 *  Created on: 26.11.2021
 *      Author: steffen
 */

#ifndef COMMON_H_
#define COMMON_H_

//#define DEBUG
#ifdef DEBUG
#define DEBUG_PRINT(x) Serial.print(x)
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINTF(...) Serial.printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#define DEBUG_PRINTF(...)
#endif


#endif /* COMMON_H_ */
