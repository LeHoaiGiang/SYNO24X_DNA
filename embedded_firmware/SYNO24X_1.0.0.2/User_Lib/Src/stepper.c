/*
 * stepper.c
 *
 *  Created on: Aug 22, 2022
 *      Author: LeHoaiGiang
 *      include stepper.h to using
 *		31-12-2024 loi dao chieu động cơ không thành công - nguyên nhân là do việc bật tắt enable liên tục có thể driver không thể đáp ứng được việc này
 */
#include "stepper.h"
#include "dwt_stm32_delay.h"
#include "stdlib.h"
//======================================================================
#ifdef VERSION_1
uint16_t STEPPER_SPEED_OPERATIONAL			= 50; // toc do binh thuong -- thoi gian delay tao xung tinh bang us 8
uint16_t STEPPER_SPEED_OPERATIONAL_Y		= 50;  // toc do binh thuong -- thoi gian delay tao xung tinh bang us 8
#endif
//======================================================================

#ifdef USING_BETA
uint16_t STEPPER_SPEED_OPERATIONAL			= 45; // toc do binh thuong -- thoi gian delay tao xung tinh bang us 8
uint16_t STEPPER_SPEED_OPERATIONAL_Y		= 65;  // toc do binh thuong -- thoi gian delay tao xung tinh bang us 8
#endif
//======================================================================
#ifdef USING_SYNO96
uint16_t STEPPER_SPEED_OPERATIONAL			= 16; // toc do binh thuong -- thoi gian delay tao xung tinh bang us 8
uint16_t STEPPER_SPEED_OPERATIONAL_Y		= 16;  // toc do binh thuong -- thoi gian delay tao xung tinh bang us 8
#endif

stepper_t Stepper_X;
stepper_t Stepper_Y;
stepper_t Stepper_Z1;
stepper_t Stepper_Z2;
volatile Cal_parameter_run_stepper_t Step_X_parameter;
volatile Cal_parameter_run_stepper_t Step_Y_parameter;
volatile Cal_parameter_run_stepper_t Step_Z1_parameter;
static uint32_t temp_X = 0;
static uint32_t temp_Y = 0;
static uint32_t u32_counter_us = 0;
uint8_t u8_Init_FW_Completed;
/*****************************************************************************************************************/

int16_t getPositionX()
{
	return Step_X_parameter.i16_Position_Old;
}
int16_t getPositionY()
{
	return Step_Y_parameter.i16_Position_Old;
}
int16_t getPositionZ1()
{
	return Step_Z1_parameter.i16_Position_Old;
}

void setPositionX(int16_t posit)
{
	Step_X_parameter.i16_Position_Old = posit;
}
void setPositionY(int16_t posit)
{
	Step_Y_parameter.i16_Position_Old = posit;
}
void setPositionZ1(int16_t posit)
{
	Step_Z1_parameter.i16_Position_Old = posit;
}
/****************************************************************************************************************
 *
 */
void Syno24_stepper_Init()
{
	Stepper_X.GPIO_Pin_DIR = X_DIR_Pin;
	Stepper_X.GPIO_Pin_STEP = X_STEP_Pin;
	Stepper_X.GPIO_Pin_EN = X_EN_Pin;
	Stepper_X.GPIO_EndStop = X_LIMIT_Pin; // pin cong tac hanh trinh

	Stepper_Y.GPIO_Pin_DIR = Y_DIR_Pin;
	Stepper_Y.GPIO_Pin_STEP = Y_STEP_Pin;
	Stepper_Y.GPIO_Pin_EN = Y_EN_Pin;
	Stepper_Y.GPIO_EndStop = Y_LIMIT_Pin; // pin cong tac hanh trinh

	Stepper_Z1.GPIO_Pin_DIR = Z1_DIR_Pin;
	Stepper_Z1.GPIO_Pin_STEP = Z1_EN_Pin;
	Stepper_Z1.GPIO_Pin_STEP = Z1_STEP_Pin;
	Stepper_Z1.GPIO_EndStop = Z1_LIMIT_Pin;


#ifdef DEBUG_SOFTWARE
	Step_Z1_parameter.i16_Position_Old = 20;
	Step_X_parameter.i16_Position_Old = 20;
	Step_Y_parameter.i16_Position_Old = 50;
	u8_Init_FW_Completed = 0;
#else
	setPositionX(0);
	setPositionY(0);
	setPositionZ1(0);
	u8_Init_FW_Completed = 0;
#endif

}
/**
 * Ham nay truoc khi chay can SET DIRECTION STEPPER  va SET ENABLE STEPPER
 * Sau khi chay can DISABLE STEPPER
 */
//void Stepper_RunNormalSpeed( GPIO_TypeDef *GPIO_PORT_STEP, stepper_t* stepper_control, uint16_t u16_time_tooglePin_STEP, uint32_t u32_NumberStepRun)
//{
//  //HAL_GPIO_WritePin(GPIOx, stepper_control->GPIO_Pin_DIR, stepper_control->u8_DIR);
//  u32_NumberStepRun = u32_NumberStepRun * 2;
//  while(u32_NumberStepRun > 0)
//    {
//      HAL_GPIO_TogglePin(GPIO_PORT_STEP, stepper_control->GPIO_Pin_STEP);
//      DWT_Delay_us(u16_time_tooglePin_STEP);
//      u32_NumberStepRun--;
//    }
//}

/***
 * GPIO_TypeDef *GPIOx
 * stepper_t* 	stepper_control
 * uint16_t 	u16_Distance_mm
 */
/*
void Stepper_RuntoDistance( GPIO_TypeDef *GPIO_STEP_PORT, GPIO_TypeDef *GPIO_ENABLE_PORT,  stepper_t* stepper_control, uint16_t u16_Distance_mm)
{
  // ENABLE STEPPER -- cho phep dong co chay
  HAL_GPIO_WritePin(GPIO_ENABLE_PORT, stepper_control->GPIO_Pin_EN, RESET);
  uint32_t u32_numberPulseTocontrol = u16_Distance_mm * NUMBER_STEP_PER_MM;
  uint16_t u16_numberPulse_Acceleration_Deceleration = u32_numberPulseTocontrol * DISTANCE_PERCENT_RUN_ACCELERATION / 100 ;
  uint32_t u32_numberPulse_Operation = u32_numberPulseTocontrol - u16_numberPulse_Acceleration_Deceleration * 2;
  uint16_t u16_Delta_step = (STEPPER_SPEED_START - STEPPER_SPEED_OPERATIONAL);
  uint16_t u16_step_Acceleration_deceleration = u16_numberPulse_Acceleration_Deceleration / u16_Delta_step;
  if(u16_Distance_mm > MINIMUM_DISTANCE_RUN_ACCELERATION)
    {
      // Acceleration STEPPER tang dan toc do len den toc do lon nhat
      stepper_control->u16_MinSpeed = STEPPER_SPEED_START;
      while(u16_numberPulse_Acceleration_Deceleration > 0)
	{
	  HAL_GPIO_WritePin(GPIO_STEP_PORT, stepper_control->GPIO_Pin_STEP, GPIO_PIN_SET);
	  DWT_Delay_us(stepper_control->u16_MinSpeed);
	  HAL_GPIO_WritePin(GPIO_STEP_PORT, stepper_control->GPIO_Pin_STEP, GPIO_PIN_RESET);
	  DWT_Delay_us(stepper_control->u16_MinSpeed);
	  stepper_control->u16_MinSpeed = STEPPER_SPEED_OPERATIONAL  + (u16_numberPulse_Acceleration_Deceleration / u16_step_Acceleration_deceleration);
	  stepper_control->u16_MinSpeed =  (stepper_control->u16_MinSpeed < STEPPER_SPEED_OPERATIONAL) ? STEPPER_SPEED_OPERATIONAL: stepper_control->u16_MinSpeed;
	  u16_numberPulse_Acceleration_Deceleration--;
	}
      // tinh lai so xung can cho viec tang giam toc do
      u16_numberPulse_Acceleration_Deceleration = u32_numberPulseTocontrol * DISTANCE_PERCENT_RUN_ACCELERATION / 100 ;
      // Operational STEPPER RUN MAX SPEED
      while(u32_numberPulse_Operation > 0)
	{
	  HAL_GPIO_WritePin(GPIO_STEP_PORT, stepper_control->GPIO_Pin_STEP, GPIO_PIN_SET);
	  DWT_Delay_us(STEPPER_SPEED_OPERATIONAL);
	  HAL_GPIO_WritePin(GPIO_STEP_PORT, stepper_control->GPIO_Pin_STEP, GPIO_PIN_RESET);
	  DWT_Delay_us(STEPPER_SPEED_OPERATIONAL);
	  u32_numberPulse_Operation--;
	}
      // deceleration  == x% cuoi cung chau cham dan lai
      while(u16_numberPulse_Acceleration_Deceleration > 0)
	{
	  HAL_GPIO_WritePin(GPIO_STEP_PORT, stepper_control->GPIO_Pin_STEP, GPIO_PIN_SET);
	  DWT_Delay_us(stepper_control->u16_MinSpeed);
	  HAL_GPIO_WritePin(GPIO_STEP_PORT, stepper_control->GPIO_Pin_STEP, GPIO_PIN_RESET);
	  DWT_Delay_us(stepper_control->u16_MinSpeed);
	  stepper_control->u16_MinSpeed = STEPPER_SPEED_START  - (u16_numberPulse_Acceleration_Deceleration / u16_step_Acceleration_deceleration);
	  stepper_control->u16_MinSpeed =  (stepper_control->u16_MinSpeed < STEPPER_SPEED_OPERATIONAL) ? STEPPER_SPEED_OPERATIONAL: stepper_control->u16_MinSpeed;
	  u16_numberPulse_Acceleration_Deceleration--;
	}
    }
  else // neu khoang cach qua ngan thi chay cham dan den toc do nho nhat
    {
      while(u32_numberPulseTocontrol > 0)
	{
	  HAL_GPIO_WritePin(GPIO_STEP_PORT, stepper_control->GPIO_Pin_STEP, GPIO_PIN_SET);
	  DWT_Delay_us(STEPPER_SPEED_OPERATIONAL);
	  HAL_GPIO_WritePin(GPIO_STEP_PORT, stepper_control->GPIO_Pin_STEP, GPIO_PIN_RESET);
	  DWT_Delay_us(STEPPER_SPEED_OPERATIONAL);
	  u32_numberPulseTocontrol--;
	}
    }
  // DISABLE STEPPER - tat dong co di
  HAL_GPIO_WritePin(GPIO_ENABLE_PORT, stepper_control->GPIO_Pin_EN, SET);
}
 */

void setSpeed_X(uint16_t u16_Time_HalfCLK)
{
	STEPPER_SPEED_OPERATIONAL = u16_Time_HalfCLK;
}

void setSpeed_Y(uint16_t u16_Time_HalfCLK)
{
	STEPPER_SPEED_OPERATIONAL_Y = u16_Time_HalfCLK;
}

void setSpeed_Z(uint16_t u16_Time_HalfCLK)
{
	//STEPPER_SPEED_OPERATIONAL = u16_Time_HalfCLK;
}
bool Stepper_AutoHome_SYN024()
{
	HAL_GPIO_WritePin(X_DIR_GPIO_Port, Stepper_X.GPIO_Pin_DIR, STEPPER_REVERSE);
	HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Stepper_Y.GPIO_Pin_DIR, STEPPER_REVERSE);
	HAL_GPIO_WritePin(Z1_DIR_GPIO_Port, Stepper_Z1.GPIO_Pin_DIR, STEPPER_Z1_REVERSE);
	uint8_t State_Home_Stepper_X = 0;
	uint8_t State_Home_Stepper_Y = 0;
	uint8_t State_Home_Stepper_Z1 = 0;
	uint16_t u16_NumberStep_RunExit_SwitchHome = NUMBER_STEP_PER_04mm;
	State_Home_Stepper_X = HAL_GPIO_ReadPin(X_LIMIT_GPIO_Port, X_LIMIT_Pin);
	State_Home_Stepper_Y = HAL_GPIO_ReadPin(Y_LIMIT_GPIO_Port, Y_LIMIT_Pin);
	State_Home_Stepper_Z1 = HAL_GPIO_ReadPin(Z1_LIMIT_GPIO_Port, Z1_LIMIT_Pin);
	//======================================================================================
	while(State_Home_Stepper_Z1 == 1)
	{
		State_Home_Stepper_Z1 = HAL_GPIO_ReadPin(Z1_LIMIT_GPIO_Port, Z1_LIMIT_Pin);
		if(State_Home_Stepper_Z1 == 1)
		{
			HAL_GPIO_TogglePin(Z1_STEP_GPIO_Port, Z1_STEP_Pin);
		}
		else
		{

		}
		DWT_Delay_us(STEPPER_SPEED_GOTOHOME);
	}

	while(State_Home_Stepper_X  == 1 ||  State_Home_Stepper_Y  == 1)
	{
		State_Home_Stepper_X = HAL_GPIO_ReadPin(X_LIMIT_GPIO_Port, X_LIMIT_Pin);
		State_Home_Stepper_Y = HAL_GPIO_ReadPin(Y_LIMIT_GPIO_Port, Y_LIMIT_Pin);
		if(State_Home_Stepper_X)
		{
			HAL_GPIO_TogglePin(X_STEP_GPIO_Port, X_STEP_Pin);
		}
		else
		{
			//DISABLE_STEPPER_X;
		}
		if(State_Home_Stepper_Y)
		{
			HAL_GPIO_TogglePin(Y_STEP_GPIO_Port, Y_STEP_Pin);
		}
		else
		{
			//DISABLE_STEPPER_Y;
		}
		DWT_Delay_us(STEPPER_SPEED_GOTOHOME);

	}
	//======================================================================================
	// THOAT KHOI CONG TAT HANH TRINH
	HAL_GPIO_WritePin(X_DIR_GPIO_Port, Stepper_X.GPIO_Pin_DIR, STEPPER_FORWARD);
	HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Stepper_Y.GPIO_Pin_DIR, STEPPER_FORWARD);
	HAL_GPIO_WritePin(Z1_DIR_GPIO_Port, Stepper_Z1.GPIO_Pin_DIR, STEPPER_Z1_FORWARD);
	State_Home_Stepper_X = HAL_GPIO_ReadPin(X_LIMIT_GPIO_Port, X_LIMIT_Pin);
	State_Home_Stepper_Y = HAL_GPIO_ReadPin(Y_LIMIT_GPIO_Port, Y_LIMIT_Pin);
	State_Home_Stepper_Z1 = HAL_GPIO_ReadPin(Z1_LIMIT_GPIO_Port, Z1_LIMIT_Pin);
	while(State_Home_Stepper_X  == 0 ||  State_Home_Stepper_Y  == 0 || State_Home_Stepper_Z1  == 0)
	{
		State_Home_Stepper_X = HAL_GPIO_ReadPin(X_LIMIT_GPIO_Port, X_LIMIT_Pin);
		State_Home_Stepper_Y = HAL_GPIO_ReadPin(Y_LIMIT_GPIO_Port, Y_LIMIT_Pin);
		State_Home_Stepper_Z1 = HAL_GPIO_ReadPin(Z1_LIMIT_GPIO_Port, Z1_LIMIT_Pin);
		if(!State_Home_Stepper_X)
		{
			HAL_GPIO_TogglePin(X_STEP_GPIO_Port, X_STEP_Pin);
		}
		else
		{
			//DISABLE_STEPPER_X;
		}
		if(!State_Home_Stepper_Y)
		{
			HAL_GPIO_TogglePin(Y_STEP_GPIO_Port, Y_STEP_Pin);
		}
		else
		{
			//DISABLE_STEPPER_Y;
		}
		if(!State_Home_Stepper_Z1)
		{
			HAL_GPIO_TogglePin(Z1_STEP_GPIO_Port, Z1_STEP_Pin);
		}
		else
		{
			//DISABLE_STEPPER_Y;
		}
		DWT_Delay_us(STEPPER_SPEED_GOTOHOME_LOW_SPEED);
	}
	// Z THOAT CONG TAT HANH TRINH == 07-11-2025 khong cho Z ha xuong nua
//	HAL_GPIO_WritePin(Z1_DIR_GPIO_Port, Stepper_Z1.GPIO_Pin_DIR, STEPPER_Z1_FORWARD);
//	while(u16_NumberStep_RunExit_SwitchHome > 0)
//	{
//		HAL_GPIO_TogglePin(Z1_STEP_GPIO_Port, Z1_STEP_Pin);
//		DWT_Delay_us(STEPPER_SPEED_GOTOHOME_LOW_SPEED);
//		u16_NumberStep_RunExit_SwitchHome --;
//	}
	u16_NumberStep_RunExit_SwitchHome = NUMBER_STEP_PER_04mm;
	HAL_GPIO_WritePin(X_DIR_GPIO_Port, Stepper_X.GPIO_Pin_DIR, STEPPER_FORWARD);
	HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Stepper_Y.GPIO_Pin_DIR, STEPPER_FORWARD);
	while(u16_NumberStep_RunExit_SwitchHome > 0)
	{
		HAL_GPIO_TogglePin(Y_STEP_GPIO_Port, Y_STEP_Pin);
		HAL_GPIO_TogglePin(X_STEP_GPIO_Port, X_STEP_Pin);
		DWT_Delay_us(STEPPER_SPEED_GOTOHOME_LOW_SPEED);
		u16_NumberStep_RunExit_SwitchHome --;
	}
	// ======================================= HOME LAN 2 == TOC DO HOME GIAM XUONG =========================================
	DWT_Delay_ms(100); // DELAY tranh tinh trang con gia toc khi dang chay
	HAL_GPIO_WritePin(X_DIR_GPIO_Port, Stepper_X.GPIO_Pin_DIR, STEPPER_REVERSE);
	HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Stepper_Y.GPIO_Pin_DIR, STEPPER_REVERSE);
	HAL_GPIO_WritePin(Z1_DIR_GPIO_Port, Stepper_Z1.GPIO_Pin_DIR, STEPPER_Z1_REVERSE);
	State_Home_Stepper_X = HAL_GPIO_ReadPin(X_LIMIT_GPIO_Port, X_LIMIT_Pin);
	State_Home_Stepper_Y = HAL_GPIO_ReadPin(Y_LIMIT_GPIO_Port, Y_LIMIT_Pin);
	State_Home_Stepper_Z1 = HAL_GPIO_ReadPin(Z1_LIMIT_GPIO_Port, Z1_LIMIT_Pin);
	//======================================================================================
	while(State_Home_Stepper_Z1 == 1)
	{
		State_Home_Stepper_Z1 = HAL_GPIO_ReadPin(Z1_LIMIT_GPIO_Port, Z1_LIMIT_Pin);
		if(State_Home_Stepper_Z1 == 1)
		{
			HAL_GPIO_TogglePin(Z1_STEP_GPIO_Port, Z1_STEP_Pin);
		}
		else
		{

		}
		DWT_Delay_us(STEPPER_SPEED_GOTOHOME_LOW_SPEED);
	}

	while(State_Home_Stepper_X  == 1 ||  State_Home_Stepper_Y  == 1)
	{
		State_Home_Stepper_X = HAL_GPIO_ReadPin(X_LIMIT_GPIO_Port, X_LIMIT_Pin);
		State_Home_Stepper_Y = HAL_GPIO_ReadPin(Y_LIMIT_GPIO_Port, Y_LIMIT_Pin);
		if(State_Home_Stepper_X)
		{
			HAL_GPIO_TogglePin(X_STEP_GPIO_Port, X_STEP_Pin);
		}
		else
		{
			//DISABLE_STEPPER_X;
		}
		if(State_Home_Stepper_Y)
		{
			HAL_GPIO_TogglePin(Y_STEP_GPIO_Port, Y_STEP_Pin);
		}
		else
		{
			//DISABLE_STEPPER_Y;
		}
		DWT_Delay_us(STEPPER_SPEED_GOTOHOME_LOW_SPEED);

	}
	//======================================================================================
	// THOAT KHOI CONG TAT HANH TRINH
	HAL_GPIO_WritePin(X_DIR_GPIO_Port, Stepper_X.GPIO_Pin_DIR, STEPPER_FORWARD);
	HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Stepper_Y.GPIO_Pin_DIR, STEPPER_FORWARD);
	HAL_GPIO_WritePin(Z1_DIR_GPIO_Port, Stepper_Z1.GPIO_Pin_DIR, STEPPER_Z1_FORWARD);
	State_Home_Stepper_X = HAL_GPIO_ReadPin(X_LIMIT_GPIO_Port, X_LIMIT_Pin);
	State_Home_Stepper_Y = HAL_GPIO_ReadPin(Y_LIMIT_GPIO_Port, Y_LIMIT_Pin);
	State_Home_Stepper_Z1 = HAL_GPIO_ReadPin(Z1_LIMIT_GPIO_Port, Z1_LIMIT_Pin);
	while(State_Home_Stepper_X  == 0 ||  State_Home_Stepper_Y  == 0 || State_Home_Stepper_Z1  == 0)
	{
		State_Home_Stepper_X = HAL_GPIO_ReadPin(X_LIMIT_GPIO_Port, X_LIMIT_Pin);
		State_Home_Stepper_Y = HAL_GPIO_ReadPin(Y_LIMIT_GPIO_Port, Y_LIMIT_Pin);
		State_Home_Stepper_Z1 = HAL_GPIO_ReadPin(Z1_LIMIT_GPIO_Port, Z1_LIMIT_Pin);
		if(!State_Home_Stepper_X)
		{
			HAL_GPIO_TogglePin(X_STEP_GPIO_Port, X_STEP_Pin);
		}
		else
		{
			//DISABLE_STEPPER_X;
		}
		if(!State_Home_Stepper_Y)
		{
			HAL_GPIO_TogglePin(Y_STEP_GPIO_Port, Y_STEP_Pin);
		}
		else
		{
			//DISABLE_STEPPER_Y;
		}
		if(!State_Home_Stepper_Z1)
		{
			HAL_GPIO_TogglePin(Z1_STEP_GPIO_Port, Z1_STEP_Pin);
		}
		else
		{
			//DISABLE_STEPPER_Y;
		}

		DWT_Delay_us(STEPPER_SPEED_GOTOHOME_LOW_SPEED);
	}
	//buzzer_mute();
	Step_X_parameter.i16_Position_Old = 0; // da ve home xong  position = 0
	Step_Y_parameter.i16_Position_Old = 0; // da ve home xong  sposition = 0
	Step_Z1_parameter.i16_Position_Old = 0; // da ve home xong  sposition = 0
	u8_Init_FW_Completed = 1; // co nay de luu trang thai - giup stepper ve home nhanh hon khi may dang hoat dong
	//	HAL_GPIO_WritePin(Z1_ENABLE_PORT, Z1_ENABLE_Pin, RESET);
	//	HAL_GPIO_WritePin(X_ENABLE_PORT, X_ENABLE_Pin, RESET);
	//	HAL_GPIO_WritePin(Y_EN_GPIO_Port, Y_EN_Pin, RESET);
	// tra ve true neu home thanh cong
	return true;
}

void Stepper_Z1_Home()
{
	HAL_GPIO_WritePin(Z1_DIR_GPIO_Port, Stepper_Z1.GPIO_Pin_DIR, STEPPER_Z1_REVERSE);
}


/****
 *	di chuyen song song 2 stepper
 *	truyen vao Distance_X Distance_Y
 *	01/03 sua loi khong debug duoc neu co 1 trong 2 stepper chay cung 1 vi tri
 */
void Stepper_move_Coordinates_XY(int u16_Distance_mm_X, int u16_Distance_mm_Y)
{
	if(u16_Distance_mm_X < 0)
	{
		u16_Distance_mm_X = 0;
	}
	if(u16_Distance_mm_X > MAX_POS_X)
	{
		u16_Distance_mm_X = MAX_POS_X;
	}
	if(u16_Distance_mm_Y < 0)
	{
		u16_Distance_mm_Y = 0;
	}
	if(u16_Distance_mm_Y > MAX_POS_Y)
	{
		u16_Distance_mm_Y = MAX_POS_Y;
	}

	//HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, SET); // DISABLE STEPPER
	//HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, SET); // DISABLE STEPPER
	HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, 1);
	HAL_GPIO_WritePin(Y_EN_GPIO_Port, Y_EN_Pin, 1);
	u16_Distance_mm_X -= Step_X_parameter.i16_Position_Old;
	u16_Distance_mm_Y -= Step_Y_parameter.i16_Position_Old;
	Step_X_parameter.i16_Position_Old += u16_Distance_mm_X;
	Step_Y_parameter.i16_Position_Old += u16_Distance_mm_Y;
	if(u16_Distance_mm_X < 0)
	{
		HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, SET);
		HAL_GPIO_WritePin(X_DIR_GPIO_Port, X_DIR_Pin, STEPPER_REVERSE);
		Step_X_parameter.i16_Distance_Move_mm = - u16_Distance_mm_X;
	}
	else
	{
		HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, SET);
		HAL_GPIO_WritePin(X_DIR_GPIO_Port, X_DIR_Pin, STEPPER_FORWARD);
		Step_X_parameter.i16_Distance_Move_mm = u16_Distance_mm_X;
	}
	if(u16_Distance_mm_Y < 0)
	{
		HAL_GPIO_WritePin(Y_EN_GPIO_Port, Y_EN_Pin, SET);
		HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Stepper_Y.GPIO_Pin_DIR, STEPPER_REVERSE);
		Step_Y_parameter.i16_Distance_Move_mm = -u16_Distance_mm_Y;
	}
	else
	{
		HAL_GPIO_WritePin(Y_EN_GPIO_Port, Y_EN_Pin, SET);
		HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Stepper_Y.GPIO_Pin_DIR, STEPPER_FORWARD);
		Step_Y_parameter.i16_Distance_Move_mm = u16_Distance_mm_Y;
	}
	if(Step_X_parameter.i16_Distance_Move_mm <= 90)
	{
		setSpeed_X(70);
	}
	else
	{
		setSpeed_X(30);
	}
	if(Step_Y_parameter.i16_Distance_Move_mm <= 90)
	{
		setSpeed_Y(70);
	}
	else
	{
		setSpeed_Y(30);
	}
	//HAL_Delay(100);
	temp_X = 0;
	temp_Y = 0;
	u32_counter_us = 0;
	Step_X_parameter.u8_Flag_Run_Completed = 1;
	Step_X_parameter.u32_numberPulseTocontrol = Step_X_parameter.i16_Distance_Move_mm * NUMBER_STEP_PER_MM * 2; // tinh so xung control
	// Tinh so xung cua doan tang toc
	Step_X_parameter.u16_numberPulse_Acceleration_Deceleration = Step_X_parameter.u32_numberPulseTocontrol / 100 * DISTANCE_PERCENT_RUN_ACCELERATION;
	// tinh so xung cua doan toc do cao nhat
	Step_X_parameter.u32_numberPulse_Operation = Step_X_parameter.u32_numberPulseTocontrol - Step_X_parameter.u16_numberPulse_Acceleration_Deceleration * 2;

	Step_X_parameter.u16_Delta_step = (STEPPER_SPEED_START - STEPPER_SPEED_OPERATIONAL);
	// tinh so xung cua doan giam toc
	Step_X_parameter.u16_step_Acceleration_deceleration = Step_X_parameter.u16_numberPulse_Acceleration_Deceleration / Step_X_parameter.u16_Delta_step;
	// doan duong ket thuc toc do cao nhat -- bat dau tu doan nay tro ve toc do mac dinh
	temp_X = Step_X_parameter.u32_numberPulseTocontrol -  Step_X_parameter.u16_numberPulse_Acceleration_Deceleration;

	Step_Y_parameter.u8_Flag_Run_Completed = 1;
	Step_Y_parameter.u32_numberPulseTocontrol = Step_Y_parameter.i16_Distance_Move_mm * NUMBER_STEP_PER_MM * 2;
	Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration = Step_Y_parameter.u32_numberPulseTocontrol / 100 * DISTANCE_PERCENT_RUN_ACCELERATION;
	Step_Y_parameter.u32_numberPulse_Operation = Step_Y_parameter.u32_numberPulseTocontrol - Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration * 2;
	Step_Y_parameter.u16_Delta_step = (STEPPER_SPEED_START - STEPPER_SPEED_OPERATIONAL);
	Step_Y_parameter.u16_step_Acceleration_deceleration = Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration / Step_Y_parameter.u16_Delta_step;
	temp_Y = Step_Y_parameter.u32_numberPulseTocontrol - (Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration);
	// ENABLE STEPPER -- cho phep dong co chay

	Step_X_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START;
	Step_Y_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START;
	Step_X_parameter.Last_us = 0;
	Step_Y_parameter.Last_us = 0;
	Step_X_parameter.u8_Flag_Run_Completed = (Step_X_parameter.u32_numberPulseTocontrol == 0) ? 0 : 1;// neu khong can di chuyen thi khoi check và chạy
	Step_Y_parameter.u8_Flag_Run_Completed = (Step_Y_parameter.u32_numberPulseTocontrol == 0) ? 0 : 1;// neu khong can di chuyen thi thoi
	// START RUN DUAL STEPPER
	while((Step_X_parameter.u8_Flag_Run_Completed == true) || (Step_Y_parameter.u8_Flag_Run_Completed == true))
	{
		if(Step_X_parameter.u8_Flag_Run_Completed == true)
		{
			if(((u32_counter_us - Step_X_parameter.Last_us  ) >= Step_X_parameter.u16_Time_HalfCLK))
			{
				if(Step_X_parameter.u32_numberPulseTocontrol > 0)
				{
					HAL_GPIO_TogglePin(X_STEP_GPIO_Port, X_STEP_Pin);
					Step_X_parameter.u32_numberPulseTocontrol--;
					Step_X_parameter.Last_us = u32_counter_us;
				}
				else
				{
					Step_X_parameter.u8_Flag_Run_Completed = false; // set flag -- thoat khoi che do phat xung cho stepper X
				}
				// Kiem soat toc do TRUC X
				if(Step_X_parameter.u32_numberPulseTocontrol > temp_X)
				{
					Step_X_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL + ((Step_X_parameter.u32_numberPulseTocontrol  - temp_X) / Step_X_parameter.u16_step_Acceleration_deceleration);
					Step_X_parameter.u16_Time_HalfCLK =  (Step_X_parameter.u16_Time_HalfCLK < STEPPER_SPEED_OPERATIONAL) ? STEPPER_SPEED_OPERATIONAL: Step_X_parameter.u16_Time_HalfCLK;
				}
				else
				{
					if(Step_X_parameter.u32_numberPulseTocontrol > Step_X_parameter.u16_numberPulse_Acceleration_Deceleration)
					{
						Step_X_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL;
					}
					else
					{
						Step_X_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START  - (Step_X_parameter.u32_numberPulseTocontrol / Step_X_parameter.u16_step_Acceleration_deceleration);
						Step_X_parameter.u16_Time_HalfCLK =  (Step_X_parameter.u16_Time_HalfCLK >= (STEPPER_SPEED_END)) ? (STEPPER_SPEED_END): Step_X_parameter.u16_Time_HalfCLK;
						Step_X_parameter.u16_Time_HalfCLK =  (Step_X_parameter.u16_Time_HalfCLK <= (STEPPER_SPEED_OPERATIONAL)) ? (STEPPER_SPEED_OPERATIONAL): Step_X_parameter.u16_Time_HalfCLK;
					}
				}

			} // end if check time for pulse
		} // end if(Step_X_parameter.u8_Flag_Run_Completed == true)
		if(Step_Y_parameter.u8_Flag_Run_Completed == 1)
		{
			if(((u32_counter_us - Step_Y_parameter.Last_us ) >= Step_Y_parameter.u16_Time_HalfCLK))
			{
				if(Step_Y_parameter.u32_numberPulseTocontrol > 0)
				{
					HAL_GPIO_TogglePin(Y_STEP_GPIO_Port, Y_STEP_Pin);
					Step_Y_parameter.Last_us = u32_counter_us;
					Step_Y_parameter.u32_numberPulseTocontrol--;
				}
				else
				{
					Step_Y_parameter.u8_Flag_Run_Completed = 0; // xoa flag -- ngung cap xung cho stepper Y
				}
				// Kiem soat toc do TRUC Y
				if(Step_Y_parameter.u32_numberPulseTocontrol > temp_Y)
				{
					Step_Y_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL_Y + ((Step_Y_parameter.u32_numberPulseTocontrol  - temp_Y) / Step_Y_parameter.u16_step_Acceleration_deceleration);
					Step_Y_parameter.u16_Time_HalfCLK =  (Step_Y_parameter.u16_Time_HalfCLK < STEPPER_SPEED_OPERATIONAL_Y) ? STEPPER_SPEED_OPERATIONAL_Y: Step_Y_parameter.u16_Time_HalfCLK;
				}
				else
				{
					if(Step_Y_parameter.u32_numberPulseTocontrol > Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration)
					{
						Step_Y_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL_Y;
					}
					else
					{
						Step_Y_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START  - (Step_Y_parameter.u32_numberPulseTocontrol / Step_Y_parameter.u16_step_Acceleration_deceleration);
						Step_Y_parameter.u16_Time_HalfCLK =  (Step_Y_parameter.u16_Time_HalfCLK >= (STEPPER_SPEED_END)) ? (STEPPER_SPEED_END): Step_Y_parameter.u16_Time_HalfCLK;
						Step_Y_parameter.u16_Time_HalfCLK =  (Step_Y_parameter.u16_Time_HalfCLK <= (STEPPER_SPEED_OPERATIONAL_Y)) ? (STEPPER_SPEED_OPERATIONAL_Y): Step_Y_parameter.u16_Time_HalfCLK;
					}
				}
			}
		} // check flag stepper Y
		DWT_Delay_us(1);
		u32_counter_us++;
	}
	//HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, RESET); // DISABLE STEPPER
	//HAL_GPIO_WritePin(Y_EN_GPIO_Port, Y_EN_Pin, RESET); // DISABLE STEPPER
}
/**********************************************************************
 *
 */
void Stepper_Z1_move(int u16_Distance_mm_Z1)
{
	if(u16_Distance_mm_Z1 > MAX_POS_Z)
	{
		u16_Distance_mm_Z1 = MAX_POS_Z;
	}
	else
	{
		if(u16_Distance_mm_Z1 < 0)
		{
			u16_Distance_mm_Z1 = 0;
		}
	}
	u16_Distance_mm_Z1 -= Step_Z1_parameter.i16_Position_Old;
	Step_Z1_parameter.i16_Position_Old += u16_Distance_mm_Z1;
	if(u16_Distance_mm_Z1 < 0)
	{
		HAL_GPIO_WritePin(Z1_EN_GPIO_Port, Z1_EN_Pin, SET);
		HAL_GPIO_WritePin(Z1_DIR_GPIO_Port, Z1_DIR_Pin, STEPPER_Z1_REVERSE);
		Step_Z1_parameter.i16_Distance_Move_mm = - u16_Distance_mm_Z1;
	}
	else
	{
		HAL_GPIO_WritePin(Z1_EN_GPIO_Port, Z1_EN_Pin, SET);
		HAL_GPIO_WritePin(Z1_DIR_GPIO_Port, Z1_DIR_Pin, STEPPER_Z1_FORWARD);
		Step_Z1_parameter.i16_Distance_Move_mm = u16_Distance_mm_Z1;
	}
	static uint32_t temp_Z1;
	u32_counter_us = 0;
	Step_Z1_parameter.u8_Flag_Run_Completed = 1;
	Step_Z1_parameter.u32_numberPulseTocontrol = Step_Z1_parameter.i16_Distance_Move_mm * NUMBER_STEP_Z1_PER_MM * 2; // tinh so xung control
	// Tinh so xung cua doan tang toc
	Step_Z1_parameter.u16_numberPulse_Acceleration_Deceleration = Step_Z1_parameter.u32_numberPulseTocontrol / 100 * DISTANCE_PERCENT_RUN_ACCELERATION;
	// tinh so xung cua doan toc do cao nhat
	Step_Z1_parameter.u32_numberPulse_Operation = Step_Z1_parameter.u32_numberPulseTocontrol - Step_Z1_parameter.u16_numberPulse_Acceleration_Deceleration * 2;

	Step_Z1_parameter.u16_Delta_step = (STEPPER_SPEED_START_Z - STEPPER_SPEED_OPERATIONAL_Z);
	// tinh so xung cua doan giam toc
	Step_Z1_parameter.u16_step_Acceleration_deceleration = Step_Z1_parameter.u16_numberPulse_Acceleration_Deceleration / Step_Z1_parameter.u16_Delta_step;
	// doan duong ket thuc toc do cao nhat -- bat dau tu doan nay tro ve toc do mac dinh
	temp_Z1 = Step_Z1_parameter.u32_numberPulseTocontrol -  Step_Z1_parameter.u16_numberPulse_Acceleration_Deceleration;


	Step_Z1_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START_Z;
	Step_Z1_parameter.Last_us = 0;
	Step_Z1_parameter.u8_Flag_Run_Completed = (Step_Z1_parameter.u32_numberPulseTocontrol == 0) ? 0 : 1;// neu khong can di chuyen thi khoi check và chạy

	while(Step_Z1_parameter.u8_Flag_Run_Completed)
	{
		if(Step_Z1_parameter.u8_Flag_Run_Completed == true)
		{
			if(((u32_counter_us - Step_Z1_parameter.Last_us  ) >= Step_Z1_parameter.u16_Time_HalfCLK))
			{
				if(Step_Z1_parameter.u32_numberPulseTocontrol > 0)
				{
					HAL_GPIO_TogglePin(Z1_STEP_GPIO_Port, Z1_STEP_Pin);
					Step_Z1_parameter.u32_numberPulseTocontrol--;
					Step_Z1_parameter.Last_us = u32_counter_us;
				}
				else
				{
					Step_Z1_parameter.u8_Flag_Run_Completed = 0; // set flag -- thoat khoi che do phat xung cho stepper X
				}
				// Kiem soat toc do TRUC X
				if(Step_Z1_parameter.u32_numberPulseTocontrol > temp_Z1)
				{
					Step_Z1_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL_Z + ((Step_Z1_parameter.u32_numberPulseTocontrol  - temp_Z1) / Step_Z1_parameter.u16_step_Acceleration_deceleration);
					Step_Z1_parameter.u16_Time_HalfCLK =  (Step_Z1_parameter.u16_Time_HalfCLK < STEPPER_SPEED_OPERATIONAL_Z) ? STEPPER_SPEED_OPERATIONAL_Z: Step_Z1_parameter.u16_Time_HalfCLK;
				}
				else
				{
					if(Step_Z1_parameter.u32_numberPulseTocontrol > Step_Z1_parameter.u16_numberPulse_Acceleration_Deceleration)
					{
						Step_Z1_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL_Z;
					}
					else
					{
						Step_Z1_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START_Z  - (Step_Z1_parameter.u32_numberPulseTocontrol / Step_Z1_parameter.u16_step_Acceleration_deceleration);
						Step_Z1_parameter.u16_Time_HalfCLK =  (Step_Z1_parameter.u16_Time_HalfCLK >= (STEPPER_SPEED_END_Z)) ? (STEPPER_SPEED_END_Z): Step_Z1_parameter.u16_Time_HalfCLK;
					}
				}
			} // end if check time for pulse
		}
		DWT_Delay_us(1);
		u32_counter_us++;
	}
	//HAL_GPIO_WritePin(Z1_EN_GPIO_Port, Z1_EN_Pin, RESET);
}

/**************************************************************************************************************
 *
 */
void Stepper_Z1_Run2Normal()
{
	if(getPositionZ1() >= Z_POSITION_NORMAL)
	{
		Stepper_Z1_move(Z_POSITION_NORMAL);
	}
	else
	{

	}
}
/*****
 * LeHoaiGiang
 * 21-03-2024
 * StepperMove2Delivery
 * @para u16_Distance_mm_X to move X
 * @para u16_Distance_mm_Y to move Y
 * function cai tien thoi gian di chuyen, doi luc khong can phai tra Z ve vi tri normal de tiet kiem thoi gian
 */
//void StepperMove2Delivery(int u16_Distance_mm_X, int u16_Distance_mm_Y)
//{
//	if(getPositionZ1() <= Z_POSITION_NORMAL)
//	{
//		// Z nho hon thi cu di chuyen XY khong can ha Z xuong vi tri normal tiet kiem thoi gian
//		Stepper_move_Coordinates_XY(u16_Distance_mm_X, u16_Distance_mm_Y);
//		Stepper_Z1_move(Z_POSITION_FILL_CHEMICAL);
//	}
//	else
//	{
//		Stepper_Z1_move(Z_POSITION_NORMAL);
//		Stepper_move_Coordinates_XY(u16_Distance_mm_X, u16_Distance_mm_Y);
//		Stepper_Z1_move(Z_POSITION_FILL_CHEMICAL);
//	}
//}
//==========================================================================================================================
void Stepper_Move(float f_Distance_mm_X, float f_Distance_mm_Y)
{
	f_Distance_mm_X -= Step_X_parameter.i16_Position_Old;
	f_Distance_mm_Y -= Step_Y_parameter.i16_Position_Old;
	Step_X_parameter.f_Distance_Move_mm += f_Distance_mm_X;
	Step_Y_parameter.f_Distance_Move_mm += f_Distance_mm_Y;
	if(f_Distance_mm_X < 0)
	{
		HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, RESET);
		HAL_GPIO_WritePin(X_DIR_GPIO_Port, Stepper_X.GPIO_Pin_DIR, STEPPER_REVERSE);
		Step_X_parameter.f_Distance_Move_mm = -f_Distance_mm_X;
	}
	else
	{
		HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, RESET);
		HAL_GPIO_WritePin(X_DIR_GPIO_Port, Stepper_X.GPIO_Pin_DIR, STEPPER_FORWARD);
		Step_X_parameter.f_Distance_Move_mm = f_Distance_mm_X;
	}
	if(f_Distance_mm_Y < 0)
	{
		HAL_GPIO_WritePin(Y_EN_GPIO_Port, Y_EN_Pin, RESET);
		HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Stepper_Y.GPIO_Pin_DIR, STEPPER_REVERSE);
		Step_Y_parameter.f_Distance_Move_mm = -f_Distance_mm_Y;
	}
	else
	{
		HAL_GPIO_WritePin(Y_EN_GPIO_Port, Y_EN_Pin, RESET);
		HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Stepper_Y.GPIO_Pin_DIR, STEPPER_FORWARD);
		Step_Y_parameter.f_Distance_Move_mm = f_Distance_mm_Y;
	}
	temp_X = 0;
	temp_Y = 0;
	u32_counter_us = 0;
	Step_X_parameter.u8_Flag_Run_Completed = 1;
	Step_X_parameter.u32_numberPulseTocontrol = Step_X_parameter.f_Distance_Move_mm * NUMBER_STEP_PER_MM * 2; // tinh so xung control
	// Tinh so xung cua doan tang toc
	Step_X_parameter.u16_numberPulse_Acceleration_Deceleration = Step_X_parameter.u32_numberPulseTocontrol * PERCENT_ACCELERATION;
	// tinh so xung cua doan toc do cao nhat
	Step_X_parameter.u32_numberPulse_Operation = Step_X_parameter.u32_numberPulseTocontrol - Step_X_parameter.u16_numberPulse_Acceleration_Deceleration * 2;

	Step_X_parameter.u16_Delta_step = (STEPPER_SPEED_START - STEPPER_SPEED_OPERATIONAL);
	// tinh so xung cua doan giam toc
	Step_X_parameter.u16_step_Acceleration_deceleration = Step_X_parameter.u16_numberPulse_Acceleration_Deceleration / Step_X_parameter.u16_Delta_step;
	// doan duong ket thuc toc do cao nhat -- bat dau tu doan nay tro ve toc do mac dinh
	temp_X = Step_X_parameter.u32_numberPulseTocontrol -  Step_X_parameter.u16_numberPulse_Acceleration_Deceleration;

	Step_Y_parameter.u8_Flag_Run_Completed = 1;
	Step_Y_parameter.u32_numberPulseTocontrol = Step_Y_parameter.f_Distance_Move_mm * NUMBER_STEP_PER_MM * 2;
	Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration = Step_Y_parameter.u32_numberPulseTocontrol * DISTANCE_PERCENT_RUN_ACCELERATION / 100;
	Step_Y_parameter.u32_numberPulse_Operation = Step_Y_parameter.u32_numberPulseTocontrol - Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration * 2;
	Step_Y_parameter.u16_Delta_step = (STEPPER_SPEED_START - STEPPER_SPEED_OPERATIONAL);
	Step_Y_parameter.u16_step_Acceleration_deceleration = Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration / Step_Y_parameter.u16_Delta_step;
	temp_Y = Step_Y_parameter.u32_numberPulseTocontrol - (Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration);
	// ENABLE STEPPER -- cho phep dongg co chay
	Step_X_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START;
	Step_Y_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START;
	Step_X_parameter.Last_us = 0;
	Step_Y_parameter.Last_us = 0;
	Step_X_parameter.u8_Flag_Run_Completed = (Step_X_parameter.u32_numberPulseTocontrol == 0) ? 0 : 1;// neu khong can di chuyen thi khoi check và chạy
	Step_Y_parameter.u8_Flag_Run_Completed = (Step_Y_parameter.u32_numberPulseTocontrol == 0) ? 0 : 1;// neu khong can di chuyen thi thoi
	// START RUN DUAL STEPPER
	while((Step_X_parameter.u8_Flag_Run_Completed == true) || (Step_Y_parameter.u8_Flag_Run_Completed == true))
	{
		if(Step_X_parameter.u8_Flag_Run_Completed == true)
		{
			if(((u32_counter_us - Step_X_parameter.Last_us  ) >= Step_X_parameter.u16_Time_HalfCLK))
			{
				if(Step_X_parameter.u32_numberPulseTocontrol > 0)
				{
					HAL_GPIO_TogglePin(X_STEP_GPIO_Port, X_STEP_Pin);
					Step_X_parameter.u32_numberPulseTocontrol--;
					Step_X_parameter.Last_us = u32_counter_us;
				}
				else
				{
					Step_X_parameter.u8_Flag_Run_Completed = false; // set flag -- thoat khoi che do phat xung cho stepper X
				}
				// Kiem soat toc do TRUC X
				if(Step_X_parameter.u32_numberPulseTocontrol > temp_X)
				{
					Step_X_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL + ((Step_X_parameter.u32_numberPulseTocontrol  - temp_X) / Step_X_parameter.u16_step_Acceleration_deceleration);
					Step_X_parameter.u16_Time_HalfCLK =  (Step_X_parameter.u16_Time_HalfCLK < STEPPER_SPEED_OPERATIONAL) ? STEPPER_SPEED_OPERATIONAL: Step_X_parameter.u16_Time_HalfCLK;
				}
				else
				{
					if(Step_X_parameter.u32_numberPulseTocontrol > Step_X_parameter.u16_numberPulse_Acceleration_Deceleration)
					{
						Step_X_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL;
					}
					else
					{
						Step_X_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START  - (Step_X_parameter.u32_numberPulseTocontrol / Step_X_parameter.u16_step_Acceleration_deceleration);
						Step_X_parameter.u16_Time_HalfCLK =  (Step_X_parameter.u16_Time_HalfCLK >= (STEPPER_SPEED_END)) ? (STEPPER_SPEED_END): Step_X_parameter.u16_Time_HalfCLK;
					}
				}

			} // end if check time for pulse
		} // end if(Step_X_parameter.u8_Flag_Run_Completed == true)
		if(Step_Y_parameter.u8_Flag_Run_Completed == 1)
		{
			if(((u32_counter_us - Step_Y_parameter.Last_us ) >= Step_Y_parameter.u16_Time_HalfCLK))
			{
				if(Step_Y_parameter.u32_numberPulseTocontrol > 0)
				{
					HAL_GPIO_TogglePin(Y_STEP_GPIO_Port, Y_STEP_Pin);
					Step_Y_parameter.Last_us = u32_counter_us;
					Step_Y_parameter.u32_numberPulseTocontrol--;
				}
				else
				{
					Step_Y_parameter.u8_Flag_Run_Completed = 0; // xoa flag -- ngung cap xung cho stepper Y
				}
				// Kiem soat toc do TRUC Y
				if(Step_Y_parameter.u32_numberPulseTocontrol > temp_Y)
				{
					Step_Y_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL + ((Step_Y_parameter.u32_numberPulseTocontrol  - temp_Y) / Step_Y_parameter.u16_step_Acceleration_deceleration);
					Step_Y_parameter.u16_Time_HalfCLK =  (Step_Y_parameter.u16_Time_HalfCLK < STEPPER_SPEED_OPERATIONAL) ? STEPPER_SPEED_OPERATIONAL: Step_Y_parameter.u16_Time_HalfCLK;
				}
				else
				{
					if(Step_Y_parameter.u32_numberPulseTocontrol > Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration)
					{
						Step_Y_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL_Y;
					}
					else
					{
						Step_Y_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START  - (Step_Y_parameter.u32_numberPulseTocontrol / Step_Y_parameter.u16_step_Acceleration_deceleration);
						Step_Y_parameter.u16_Time_HalfCLK =  (Step_Y_parameter.u16_Time_HalfCLK >= (STEPPER_SPEED_END)) ? (STEPPER_SPEED_END): Step_Y_parameter.u16_Time_HalfCLK;
					}
				}
			}
		} // check flag stepper Y
		DWT_Delay_us(1);
		u32_counter_us++;
	}

	HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, SET); // DISABLE STEPPER
	HAL_GPIO_WritePin(Y_EN_GPIO_Port, Y_EN_Pin, SET); // DISABLE STEPPER
}


void Stepper_move_Coordinates_XY_with_Limit_Z(int u16_Distance_mm_X, int u16_Distance_mm_Y)
{
	if(Step_Z1_parameter.i16_Position_Old > Z_POSITION_NORMAL)
	{
		Stepper_Z1_move(Z_POSITION_NORMAL);
	}
	if(u16_Distance_mm_X < 0)
	{
		u16_Distance_mm_X = 0;
	}
	if(u16_Distance_mm_X > MAX_POS_X)
	{
		u16_Distance_mm_X = MAX_POS_X;
	}
	if(u16_Distance_mm_Y < 0)
	{
		u16_Distance_mm_Y = 0;
	}
	if(u16_Distance_mm_Y > MAX_POS_Y)
	{
		u16_Distance_mm_Y = MAX_POS_Y;
	}
	HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, SET); // DISABLE STEPPER
	HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, SET); // DISABLE STEPPER

	u16_Distance_mm_X -= Step_X_parameter.i16_Position_Old;
	u16_Distance_mm_Y -= Step_Y_parameter.i16_Position_Old;
	Step_X_parameter.i16_Position_Old += u16_Distance_mm_X;
	Step_Y_parameter.i16_Position_Old += u16_Distance_mm_Y;
	if(u16_Distance_mm_X < 0)
	{
		HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, RESET);
		HAL_GPIO_WritePin(X_DIR_GPIO_Port, Stepper_X.GPIO_Pin_DIR, STEPPER_REVERSE);
		Step_X_parameter.i16_Distance_Move_mm = - u16_Distance_mm_X;
	}
	else
	{
		HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, RESET);
		HAL_GPIO_WritePin(X_DIR_GPIO_Port, Stepper_X.GPIO_Pin_DIR, STEPPER_FORWARD);
		Step_X_parameter.i16_Distance_Move_mm = u16_Distance_mm_X;
	}
	if(u16_Distance_mm_Y < 0)
	{
		HAL_GPIO_WritePin(Y_EN_GPIO_Port, Y_EN_Pin, RESET);
		HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Stepper_Y.GPIO_Pin_DIR, STEPPER_REVERSE);
		Step_Y_parameter.i16_Distance_Move_mm = -u16_Distance_mm_Y;
	}
	else
	{
		HAL_GPIO_WritePin(Y_EN_GPIO_Port, Y_EN_Pin, RESET);
		HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Stepper_Y.GPIO_Pin_DIR, STEPPER_FORWARD);
		Step_Y_parameter.i16_Distance_Move_mm = u16_Distance_mm_Y;
	}
	temp_X = 0;
	temp_Y = 0;
	u32_counter_us = 0;
	Step_X_parameter.u8_Flag_Run_Completed = 1;
	Step_X_parameter.u32_numberPulseTocontrol = Step_X_parameter.i16_Distance_Move_mm * NUMBER_STEP_PER_MM * 2; // tinh so xung control
	// Tinh so xung cua doan tang toc
	Step_X_parameter.u16_numberPulse_Acceleration_Deceleration = Step_X_parameter.u32_numberPulseTocontrol / 100 * DISTANCE_PERCENT_RUN_ACCELERATION;
	// tinh so xung cua doan toc do cao nhat
	Step_X_parameter.u32_numberPulse_Operation = Step_X_parameter.u32_numberPulseTocontrol - Step_X_parameter.u16_numberPulse_Acceleration_Deceleration * 2;

	Step_X_parameter.u16_Delta_step = (STEPPER_SPEED_START - STEPPER_SPEED_OPERATIONAL);
	// tinh so xung cua doan giam toc
	Step_X_parameter.u16_step_Acceleration_deceleration = Step_X_parameter.u16_numberPulse_Acceleration_Deceleration / Step_X_parameter.u16_Delta_step;
	// doan duong ket thuc toc do cao nhat -- bat dau tu doan nay tro ve toc do mac dinh
	temp_X = Step_X_parameter.u32_numberPulseTocontrol -  Step_X_parameter.u16_numberPulse_Acceleration_Deceleration;

	Step_Y_parameter.u8_Flag_Run_Completed = 1;
	Step_Y_parameter.u32_numberPulseTocontrol = Step_Y_parameter.i16_Distance_Move_mm * NUMBER_STEP_PER_MM * 2;
	Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration = Step_Y_parameter.u32_numberPulseTocontrol / 100 * DISTANCE_PERCENT_RUN_ACCELERATION;
	Step_Y_parameter.u32_numberPulse_Operation = Step_Y_parameter.u32_numberPulseTocontrol - Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration * 2;
	Step_Y_parameter.u16_Delta_step = (STEPPER_SPEED_START - STEPPER_SPEED_OPERATIONAL);
	Step_Y_parameter.u16_step_Acceleration_deceleration = Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration / Step_Y_parameter.u16_Delta_step;
	temp_Y = Step_Y_parameter.u32_numberPulseTocontrol - (Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration);
	// ENABLE STEPPER -- cho phep dongg co chay

	Step_X_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START;
	Step_Y_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START;
	Step_X_parameter.Last_us = 0;
	Step_Y_parameter.Last_us = 0;
	Step_X_parameter.u8_Flag_Run_Completed = (Step_X_parameter.u32_numberPulseTocontrol == 0) ? 0 : 1;// neu khong can di chuyen thi khoi check và chạy
	Step_Y_parameter.u8_Flag_Run_Completed = (Step_Y_parameter.u32_numberPulseTocontrol == 0) ? 0 : 1;// neu khong can di chuyen thi thoi
	// START RUN DUAL STEPPER
	while((Step_X_parameter.u8_Flag_Run_Completed == true) || (Step_Y_parameter.u8_Flag_Run_Completed == true))
	{
		if(Step_X_parameter.u8_Flag_Run_Completed == true)
		{
			if(((u32_counter_us - Step_X_parameter.Last_us  ) >= Step_X_parameter.u16_Time_HalfCLK))
			{
				if(Step_X_parameter.u32_numberPulseTocontrol > 0)
				{
					HAL_GPIO_TogglePin(X_STEP_GPIO_Port, X_STEP_Pin);
					Step_X_parameter.u32_numberPulseTocontrol--;
					Step_X_parameter.Last_us = u32_counter_us;
				}
				else
				{
					Step_X_parameter.u8_Flag_Run_Completed = false; // set flag -- thoat khoi che do phat xung cho stepper X
				}
				// Kiem soat toc do TRUC X
				if(Step_X_parameter.u32_numberPulseTocontrol > temp_X)
				{
					Step_X_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL + ((Step_X_parameter.u32_numberPulseTocontrol  - temp_X) / Step_X_parameter.u16_step_Acceleration_deceleration);
					Step_X_parameter.u16_Time_HalfCLK =  (Step_X_parameter.u16_Time_HalfCLK < STEPPER_SPEED_OPERATIONAL) ? STEPPER_SPEED_OPERATIONAL: Step_X_parameter.u16_Time_HalfCLK;
				}
				else
				{
					if(Step_X_parameter.u32_numberPulseTocontrol > Step_X_parameter.u16_numberPulse_Acceleration_Deceleration)
					{
						Step_X_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL;
					}
					else
					{
						Step_X_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START  - (Step_X_parameter.u32_numberPulseTocontrol / Step_X_parameter.u16_step_Acceleration_deceleration);
						Step_X_parameter.u16_Time_HalfCLK =  (Step_X_parameter.u16_Time_HalfCLK >= (STEPPER_SPEED_END)) ? (STEPPER_SPEED_END): Step_X_parameter.u16_Time_HalfCLK;
					}
				}

			} // end if check time for pulse
		} // end if(Step_X_parameter.u8_Flag_Run_Completed == true)
		if(Step_Y_parameter.u8_Flag_Run_Completed == 1)
		{
			if(((u32_counter_us - Step_Y_parameter.Last_us ) >= Step_Y_parameter.u16_Time_HalfCLK))
			{
				if(Step_Y_parameter.u32_numberPulseTocontrol > 0)
				{
					HAL_GPIO_TogglePin(Y_STEP_GPIO_Port, Y_STEP_Pin);
					Step_Y_parameter.Last_us = u32_counter_us;
					Step_Y_parameter.u32_numberPulseTocontrol--;
				}
				else
				{
					Step_Y_parameter.u8_Flag_Run_Completed = 0; // xoa flag -- ngung cap xung cho stepper Y
				}
				// Kiem soat toc do TRUC Y
				if(Step_Y_parameter.u32_numberPulseTocontrol > temp_Y)
				{
					Step_Y_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL + ((Step_Y_parameter.u32_numberPulseTocontrol  - temp_Y) / Step_Y_parameter.u16_step_Acceleration_deceleration);
					Step_Y_parameter.u16_Time_HalfCLK =  (Step_Y_parameter.u16_Time_HalfCLK < STEPPER_SPEED_OPERATIONAL) ? STEPPER_SPEED_OPERATIONAL: Step_Y_parameter.u16_Time_HalfCLK;
				}
				else
				{
					if(Step_Y_parameter.u32_numberPulseTocontrol > Step_Y_parameter.u16_numberPulse_Acceleration_Deceleration)
					{
						Step_Y_parameter.u16_Time_HalfCLK = STEPPER_SPEED_OPERATIONAL_Y;
					}
					else
					{
						Step_Y_parameter.u16_Time_HalfCLK = STEPPER_SPEED_START  - (Step_Y_parameter.u32_numberPulseTocontrol / Step_Y_parameter.u16_step_Acceleration_deceleration);
						Step_Y_parameter.u16_Time_HalfCLK =  (Step_Y_parameter.u16_Time_HalfCLK >= (STEPPER_SPEED_END)) ? (STEPPER_SPEED_END): Step_Y_parameter.u16_Time_HalfCLK;
					}
				}
			}
		} // check flag stepper Y
		DWT_Delay_us(1);
		u32_counter_us++;
	}
	HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, RESET); // DISABLE STEPPER
	HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, RESET); // DISABLE STEPPER
}

