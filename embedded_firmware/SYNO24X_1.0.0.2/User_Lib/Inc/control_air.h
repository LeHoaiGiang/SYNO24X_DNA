/*
 * control_air.h
 *
 *  Created on: Mar 14, 2024
 *      Author: LeHoaiGiang
 */

#ifndef INC_CONTROL_AIR_H_
#define INC_CONTROL_AIR_H_
#include "main.h"
#include "struct.h"
/*
 *
 * 		{GPIOB, GPIO_PIN_14},//v2
		{GPIOB, GPIO_PIN_11},//v4
		{GPIOB, GPIO_PIN_10},//v6
 */
#define  VALVE_NITOR_DRY_Pin					GPIO_PIN_14
#define  VALVE_NITOR_DRY_Port					GPIOB
#define  VALVE_PushDownLOWPressure_Pin			GPIO_PIN_11
#define  VALVE_PushDownLOWPressure_Port			GPIOB
#define  VALVE_PushDownHIGHPressure_Pin			GPIO_PIN_10
#define  VALVE_PushDownHIGHPressure_Port		GPIOB
void controlAirPushDownDisable();
#endif /* INC_CONTROL_AIR_H_ */
