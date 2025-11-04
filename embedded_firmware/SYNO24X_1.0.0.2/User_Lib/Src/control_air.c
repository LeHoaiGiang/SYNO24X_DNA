/*
 * control_air.c
 *
 *  Created on: Mar 14, 2024
 *      Author: LeHoaiGiang
 */
#include "control_air.h"

void controlAirOpenAriNitor()
{
	HAL_GPIO_WritePin(VALVE_NITOR_DRY_Port, VALVE_NITOR_DRY_Pin, SET);
}

void controlAirCloseAriNitor()
{
	HAL_GPIO_WritePin(VALVE_NITOR_DRY_Port, VALVE_NITOR_DRY_Pin, RESET);
}

void controlAirPushDownHighPressure()
{
	HAL_GPIO_WritePin(VALVE_PushDownHIGHPressure_Port, VALVE_PushDownHIGHPressure_Pin, SET);
}
void controlAirPushDownLowPressure()
{
	HAL_GPIO_WritePin(VALVE_PushDownLOWPressure_Port, VALVE_PushDownLOWPressure_Pin, SET);
}

void controlAirPushDownOFFALL()
{
	HAL_GPIO_WritePin(VALVE_PushDownHIGHPressure_Port, VALVE_PushDownHIGHPressure_Pin, RESET);
	HAL_GPIO_WritePin(VALVE_PushDownLOWPressure_Port, VALVE_PushDownLOWPressure_Pin, RESET);
}

void controlAirOffAll()
{
	HAL_GPIO_WritePin(VALVE_PushDownHIGHPressure_Port, VALVE_PushDownHIGHPressure_Pin, RESET);
	HAL_GPIO_WritePin(VALVE_PushDownLOWPressure_Port, VALVE_PushDownLOWPressure_Pin, RESET);
	HAL_GPIO_WritePin(VALVE_NITOR_DRY_Port, VALVE_NITOR_DRY_Pin, RESET);
}
