/*
 * hmp110.c
 *
 *  Created on: Oct 16, 2023
 *      Author: LeHoaiGiang
 */
#include <hmp110.h>
hmp110_t hmp110;
float HMP110_get_temperature()
{
	ADS1115_Read_A0(&hmp110.f_Volt_T);
	hmp110.f_temperature =  mapFloat(hmp110.f_Volt_T, 0, 5, -40, 80);
	return hmp110.f_temperature;
}

float HMP110_get_humidity()
{

	ADS1115_Read_A0(&hmp110.f_Volt_H);
	hmp110.f_humidity =  mapFloat(hmp110.f_Volt_H, 0, 5, 0 , 100);
	return hmp110.f_humidity;
}


float mapFloat(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
	return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow;
}

void HMP110_control_humidity()
{
	if(hmp110.f_humidity >= hmp110.u16tb_humidity_HIGH_limit_value)
	{
		//HAL_GPIO_WritePin(SOLENOID_OPEN_NITO_PORT, SOLENOID_OPEN_NITO_PIN, SET);
	}
	else
	{
		if(hmp110.f_humidity < hmp110.u16tb_humidity_LOW_limit_value)
		{
			//HAL_GPIO_WritePin(SOLENOID_OPEN_NITO_PORT, SOLENOID_OPEN_NITO_PIN, RESET);
		}
	}
}

void HMP110_SetlimiHumidity(uint16_t u16_limit_value)
{

	hmp110.u16tb_humidity_HIGH_limit_value = u16_limit_value + OFFSET_LIMIT_VALUE;
	hmp110.u16tb_humidity_LOW_limit_value = u16_limit_value;
}
