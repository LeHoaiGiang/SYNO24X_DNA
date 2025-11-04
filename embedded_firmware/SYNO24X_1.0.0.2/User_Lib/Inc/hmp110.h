/*
 * hmp110.h
 *
 *  Created on: Oct 16, 2023
 *      Author: LeHoaiGiang
 */

#ifndef INC_HMP110_H_
#define INC_HMP110_H_
#include "ADS1115.h"
#define OFFSET_LIMIT_VALUE		4
#define SOLENOID_OPEN_NITO_PORT
#define SOLENOID_OPEN_NITO_PIN
typedef struct
{
	float f_Volt_T;
	float f_Volt_H;
	float f_temperature;
	float f_humidity;
	uint16_t u16tb_humidity_HIGH_limit_value;
	uint16_t u16tb_humidity_LOW_limit_value;
}hmp110_t;

float HMP110_get_temperature();
float HMP110_get_humidity();
void HMP110_control_humidity();
void HMP110_SetlimiHumidity(uint16_t u16_limit_value);
float mapFloat(float value, float fromLow, float fromHigh, float toLow, float toHigh);

#endif /* INC_HMP110_H_ */
