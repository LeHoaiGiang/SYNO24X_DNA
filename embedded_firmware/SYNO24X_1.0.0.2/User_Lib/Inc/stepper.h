/*
 * stepper.h
 *
 *  Created on: Jan 11, 2024
 *      Author: LeHoaiGiang
 *      22-02-2024 V1.0.0.2:
 *      + cap nhat tinh nang auto home - auto return 2 time with switchlimit
 *      + delete function not use
 *		+ add function setSpeed_X() setSpeed_Y() for stepper
 *		+ get position stepper
 *		07-05-2024
 *		+ sua lai cach tinh toa do x10 de co the dung so le, toa do dung so x10
 *		+
 */
// ==>>>> 1mm = 3200/8 = 400 xung
// vi buoc 32*200 = 6400 xung = 8mm
// ==> 1 mm = 6400/8 = 800 xung
#ifndef INC_STEPPER_H__
#define INC_STEPPER_H__

#include "stdbool.h"
#include "main.h"
#include <stdint.h>
#include "function.h"
#define USING_SYNO96

//=========================================================================================================
#ifdef USING_SYNO96
#define STEPPER_FORWARD						0
#define STEPPER_REVERSE						1
#define STEPPER_Z1_FORWARD					0
#define STEPPER_Z1_REVERSE					1
// =========================================SETTING PULSE
#define NUMBER_STEP_PER_MM 					20
#define NUMBER_STEP_Z1_PER_MM 				125
#define NUMBER_STEP_PER_3MM 				25*3*2 // 1mm x 3 x 2(x2 so xung do tooglePin Stepper)
//========================================== MOTION CONTROL
#define STEPPER_SPEED_START					100 // toc do khoi dong tinh bang us
#define STEPPER_SPEED_END					100 // toc do thap nhat -- thoi gian delay tao xung tinh bang us  30
#define STEPPER_SPEED_GOTOHOME				35
#define STEPPER_SPEED_GOTOHOME_LOW_SPEED	500
#define STEPPER_SPEED_START_Z				120 // toc do khoi dong tinh bang us
#define STEPPER_SPEED_END_Z					120 // toc do thap nhat -- thoi gian delay tao xung tinh bang us  30
#define STEPPER_SPEED_OPERATIONAL_Z			50 // toc do binh thuong -- thoi gian delay tao xung tinh bang us 8


//#define Z_POSITION_NORMAL 300
//#define Z_POSITION_FILL_CHEMICAL 375

#endif



#define MINIMUM_DISTANCE_RUN_ACCELERATION 	10 // tinh bang mm == neu di chuyen lon hon 50mm thi moi chay tang toc dan
#define DISTANCE_PERCENT_RUN_ACCELERATION	20 // tinh bang % doan duong dau tien dong co se tang toc dan và 15% doan duong cuoi se giam toc do dan
#define DIR_HOME_X_STEPPER					STEPPER_REVERSE
#define DIR_HOME_Y_STEPPER					STEPPER_REVERSE
#define DIR_HOME_Z1_STEPPER					STEPPER_FORWARD
#define DIR_HOME_Z2_STEPPER					STEPPER_FORWARD

#define	MAX_POS_X							1660
#define	MAX_POS_Y							1650
#define	MAX_POS_Z							200
// 28-10-2024 Software Limit define number pulse for auto home
// khi home neu di qua so xung ma dong co khong ngung co nghia loi cam bien home
#define	NUMBER_PULSE_SOFTWARE_LIMIT_X				1700 * NUMBER_STEP_PER_MM
#define	NUMBER_PULSE_SOFTWARE_LIMIT_Y				1600 * NUMBER_STEP_PER_MM
#define	NUMBER_PULSE_SOFTWARE_LIMIT_Z				200 * NUMBER_STEP_PER_MM

#define NUMBER_STEP_PER_04mm				40*4*2 // 1mm x 3 x 2(x2 so xung do tooglePin Stepper)
// NEW FUNCTION CONTROL STEPPER
#define PERCENT_ACCELERATION				0.2
#define PERCENT_DECELERATION				0.2
#define PERCENT_OPERATIONAL					0.6
typedef struct
{
	uint16_t u16_MaxSpeed;
	uint16_t u16_MinSpeed;
	uint16_t GPIO_Pin_DIR; // pin Direction dong co
	uint16_t GPIO_Pin_STEP; // Pin Step dong co
	uint16_t GPIO_Pin_EN; // Pin Enable dong co
	uint16_t GPIO_EndStop; // Pin con tac hanh trinh dong co
	uint8_t u8_DIR; // 1 foward 0 reverse
}stepper_t;
typedef struct
{
	uint32_t u32_numberPulseTocontrol ;
	uint32_t u32_numberPulse_Operation ;
	uint32_t Last_us;
	uint16_t u16_numberPulse_Acceleration_Deceleration;
	uint16_t u16_Delta_step;
	uint16_t u16_step_Acceleration_deceleration;
	volatile uint16_t u16_Time_HalfCLK;
	uint8_t u8_Flag_Run_Completed;
	volatile int16_t i16_Position_Old;
	volatile int16_t i16_Distance_Move_mm;
	float f_Distance_Move_mm;
	float f_Position_Old;
}Cal_parameter_run_stepper_t;
#define MAX_DISTANCE_Z1			14
#define X_ENABLE_PORT			X_EN_GPIO_Port
#define Y_ENABLE_PORT			Y_EN_GPIO_Port
#define Z1_ENABLE_PORT			Z1_EN_GPIO_Port

#define X_ENABLE_Pin			X_EN_Pin
#define Y_ENABLE_Pin			Y_EN_Pin
#define Z1_ENABLE_Pin			Z1_EN_Pin

#define X_LIMIT_PORT			X_LIMIT_GPIO_Port
#define Y_LIMIT_PORT			Y_LIMIT_GPIO_Port
#define Z1_LIMIT_PORT			Z1_LIMIT_GPIO_Port
#define Z2_LIMIT_PORT			Z2_LIMIT_GPIO_Port

#define X_DIR_PORT			X_DIR_GPIO_Port
#define Y_DIR_PORT			Y_DIR_GPIO_Port
#define Z1_DIR_PORT			Z1_DIR_GPIO_Port
// define Function
#define ENABLE_STEPPER_X		HAL_GPIO_WritePin(X_ENABLE_PORT, X_ENABLE_Pin, RESET)
#define ENABLE_STEPPER_Y		HAL_GPIO_WritePin(Y_ENABLE_PORT, Y_ENABLE_Pin, RESET)
#define ENABLE_STEPPER_Z1		HAL_GPIO_WritePin(Z1_ENABLE_PORT, Z1_ENABLE_Pin, RESET)
//#define ENABLE_STEPPER_Z2		HAL_GPIO_WritePin(Z2_ENABLE_PORT, Z2_ENABLE_Pin, RESET)
#define DISABLE_STEPPER_X		HAL_GPIO_WritePin(X_ENABLE_PORT, X_ENABLE_Pin, SET)
#define DISABLE_STEPPER_Y		HAL_GPIO_WritePin(Y_ENABLE_PORT, Y_ENABLE_Pin, SET)
#define DISABLE_STEPPER_Z1		HAL_GPIO_WritePin(Z1_ENABLE_PORT, Z1_ENABLE_Pin, SET)
//#define DISABLE_STEPPER_Z2		HAL_GPIO_WritePin(Z2_ENABLE_PORT, Z2_ENABLE_Pin, SET)
int16_t getPositionX();
int16_t getPositionY();
int16_t getPositionZ1();
void setPositionX(int16_t posit);
void setPositionY(int16_t posit);
void setPositionZ1(int16_t posit);
bool Stepper_AutoHome_SYN024();
void Stepper_move_Coordinates_XY(int u16_Distance_mm_X, int u16_Distance_mm_Y);
void Stepper_Z1_move(int u16_Distance_mm_Z1);
void Stepper_Z1_Home();
void Stepper_Z1_Run2Normal();
void StepperMove2Delivery(int u16_Distance_mm_X, int u16_Distance_mm_Y);
#endif /* STEPPER_H_ */
