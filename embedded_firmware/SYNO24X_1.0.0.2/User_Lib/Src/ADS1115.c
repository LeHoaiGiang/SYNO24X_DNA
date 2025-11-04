/*
 * ADS1115.c
 *
 *  Created on: Dec 22, 2022
 *      Author: LeHoaiGiang
 */

#include "ADS1115.h"
ADS1115_ADC_t ADS_var;
unsigned char ADSwrite[6];
int16_t reading;
float voltage[4];
HAL_StatusTypeDef ADS1115_Read_A0(float* Volt)
{
	ADSwrite[0] = 0x01;
	ADSwrite[1] = 0xC1; //11000001
	ADSwrite[2] = 0x83; //10000011 LSB
	if(	HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1, ADSwrite, 3, 100) == HAL_OK)
	{
		ADSwrite[0] = 0x00;
		if(	HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1 , ADSwrite, 1 ,100) == HAL_OK)
		{
			HAL_Delay(20);
			HAL_I2C_Master_Receive(&hi2c1, ADS1115_ADDRESS <<1, ADSwrite, 2, 100);
			reading = (ADSwrite[0] << 8 | ADSwrite[1] );
			if(reading < 0)
			{
				reading = 0;
				return HAL_ERROR;
			}
			*Volt = reading * 6.114 / 32768.0;
			return HAL_OK;
		}
	}
	return HAL_ERROR;
}
HAL_StatusTypeDef ADS1115_Read_A1(float* Volt)
{
	ADSwrite[0] = 0x01;
	ADSwrite[1] = 0xD1; //11000001
	ADSwrite[2] = 0x83; //10000011 LSB
	if(HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1, ADSwrite, 3, 100) == HAL_OK)
	{
		ADSwrite[0] = 0x00;
		if(HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1 , ADSwrite, 1 ,100) == HAL_OK)
		{
			HAL_Delay(20);
			HAL_I2C_Master_Receive(&hi2c1, ADS1115_ADDRESS <<1, ADSwrite, 2, 100);
			reading = (ADSwrite[0] << 8 | ADSwrite[1] );
			if(reading < 0)
			{
				reading = 0;
				return HAL_ERROR;
			}
			*Volt = reading * 6.114 / 32768.0;
			return HAL_OK;
		}
	}
	return HAL_ERROR;
}
float ADS1115_ReadADC_Single(I2C_HandleTypeDef* hi2c, uint8_t u8_CHANNEL_ADC)
{
	ADS_var.ADSwrite[0] = 0x01;
	switch(u8_CHANNEL_ADC)
	{
	case  0:
	{
		ADS_var.ADSwrite[1] = 0xC1; //11000001
		break;
	}

	case 1:
	{
		ADS_var.ADSwrite[1] = 0xD1; //11010001
		break;
	}

	case 2:
	{
		ADS_var.ADSwrite[1] = 0xE1;
		break;
	}

	case 3:
	{
		ADS_var.ADSwrite[1] = 0xF1;
		break;
	}
	default:
	{
		break;
	}
	}
	ADS_var.ADSwrite[0] = 0x01;
	//ADS_var.ADSwrite[1] = 0xC1; //11000001
	ADS_var.ADSwrite[2] = 0x83; //10000011 LSB
	HAL_I2C_Master_Transmit(hi2c, ADS1115_ADDRESS << 1, ADSwrite, 3, 100);
	ADSwrite[0] = 0x00;
	HAL_I2C_Master_Transmit(hi2c, ADS1115_ADDRESS << 1 ,  ADSwrite, 1 ,100);
	HAL_Delay(20);
	HAL_I2C_Master_Receive(hi2c, ADS1115_ADDRESS <<1,  ADSwrite, 2, 100);
	ADS_var.u16_ADC_Value = ADS_var.ADSwrite[0] <<8 | ADS_var.ADSwrite[1];
	if(ADS_var.u16_ADC_Value < 0)
	{
		ADS_var.u16_ADC_Value = 0;
	}
	ADS_var.f_Volt_ADC = ADS_var.u16_ADC_Value * 6.114 / 32768.0;
	return ADS_var.f_Volt_ADC;
}

void ADC_All()
{
	for(int i=0; i< 4; i++)
	{
		ADSwrite[0] = 0x01;

		switch(i)
		{
		case(0) :
								ADSwrite[1] = 0xC1; //11000001
		break;
		case(1):
								ADSwrite[1] = 0xD1; //11010001
		break;
		case(2):
								ADSwrite[1] = 0xE1;
		break;
		case(3):
								ADSwrite[1] = 0xF1;
		break;
		}

		ADSwrite[2] = 0x83; //10000011 LSB

		HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1, ADSwrite, 3, 100);
		ADSwrite[0] = 0x00;
		HAL_I2C_Master_Transmit(&hi2c1, ADS1115_ADDRESS << 1 , ADSwrite, 1 ,100);
		HAL_Delay(20);

		HAL_I2C_Master_Receive(&hi2c1, ADS1115_ADDRESS <<1, ADSwrite, 2, 100);
		reading = (ADSwrite[0] << 8 | ADSwrite[1] );
		if(reading < 0)
		{
			reading = 0;
		}
		voltage[i] = reading * 6.114 / 32768.0;
	}
}
