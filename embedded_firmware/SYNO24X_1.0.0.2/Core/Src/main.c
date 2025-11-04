/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2024 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/*
 * Author: LeHoaiGiang
 * email : lehoaigiangg@gmail.com
 * Contact : 0336379944
 * I am SYNO24X machine
 * Project name: SYNO24X (oligonucleotide synthesis system)
 * 04-11-2024 edit function send data uart - need testting
 * 23-04-2025 v1.0.0.3 SYNO24X design Coupling advanced for floatting amidite
 */

#include "dwt_stm32_delay.h"
#include "struct.h"
#include <stdio.h>
#include "hdc1080.h"
#include <stdarg.h>
#include "hmp110.h"
#include "ads1115x.h"
#include "global_extern.h"
#include "i2c-lcd.h"
#include "control_air.h"
#include "ADS1115.h"
#include "stepper.h"
#include "gpio_timer.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_VALVE 40
const GPIO_Config VALVE[40] =
{
		{V1_GPIO_Port, V1_Pin},
		{V2_GPIO_Port, V2_Pin},
		{V3_GPIO_Port, V3_Pin},
		{V4_GPIO_Port, V4_Pin},
		{V5_GPIO_Port, V5_Pin},
		{V6_GPIO_Port, V6_Pin},
		{V7_GPIO_Port, V7_Pin},
		{V8_GPIO_Port, V8_Pin},
		{V9_GPIO_Port, V9_Pin},
		{V10_GPIO_Port, V10_Pin}, // end 10 valve

		{V11_GPIO_Port, V11_Pin},
		{V12_GPIO_Port, V12_Pin},
		{V13_GPIO_Port, V13_Pin},
		{V14_GPIO_Port, V14_Pin},
		{V15_GPIO_Port, V15_Pin},
		{V16_GPIO_Port, V16_Pin},
		{V17_GPIO_Port, V17_Pin}, // ket thuc valve hoa chat roi
		{V18_GPIO_Port, V18_Pin},
		{V19_GPIO_Port, V19_Pin},
		{V20_GPIO_Port, V20_Pin},

		{V21_GPIO_Port, V21_Pin},
		{V22_GPIO_Port, V22_Pin},
		{V23_GPIO_Port, V23_Pin},
		{V24_GPIO_Port, V24_Pin},
		{V25_GPIO_Port, V25_Pin},
		{V26_GPIO_Port, V26_Pin},
		{V27_GPIO_Port, V27_Pin},
		{V28_GPIO_Port, V28_Pin},
		{V29_GPIO_Port, V29_Pin},
		{V30_GPIO_Port, V30_Pin},

		{V31_GPIO_Port, V31_Pin},
		{V32_GPIO_Port, V32_Pin},
		{V33_GPIO_Port, V33_Pin},
		{V34_GPIO_Port, V34_Pin},
		{V35_GPIO_Port, V35_Pin},
		{V36_GPIO_Port, V36_Pin},
		{V37_GPIO_Port, V37_Pin},
		{V38_GPIO_Port, V38_Pin},
		{V39_GPIO_Port, V39_Pin},
		{V40_GPIO_Port, V40_Pin},
		// Valve 40



};

GPIO_Config SOLENOID[8] =
{
		{V33_GPIO_Port, V33_Pin},
		{V34_GPIO_Port, V34_Pin},
		{V35_GPIO_Port, V35_Pin},
		{V32_GPIO_Port, V32_Pin}, // 3 fan in box
		{V37_GPIO_Port, V37_Pin},
		{V38_GPIO_Port, V38_Pin},
		{V39_GPIO_Port, V39_Pin},
		{V40_GPIO_Port, V40_Pin},
};
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim14;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart1_tx;

/* USER CODE BEGIN PV */
float f_Volt_T;
float f_Volt_H;
int State_Input[4];
const uint8_t idx_start_opt_vaccum = 50;
const uint8_t idx_start_time_process = 60;
const uint8_t idx_start_time_wait = 80; // 20byte tuu 80 den 99
const uint16_t idx_start_time_fill_mixfunction_1 = 100; // 20byte tu
const uint16_t idx_start_time_fill_mixfunction_2 = 150; // 20byte tu
const uint16_t idx_start_time_fill_mixfunction_3 = 200; // 20byte tu
const uint16_t idx_start_sequence_amidite = 290;
volatile float Temperature;
volatile uint8_t Humidity;
// Luu command tam thoi, tranh bi thay doi data khi dang chay
uint8_t Command[UART_LENGTH_DATA_OLIGO];
uint8_t	u8_index_well = 0;
// khoi tao bien toan cuc
Global_var_t global_variable = {0};
uint16_t crc_result ;
uint16_t crc_result_data ;
float f_Volt_T;
float f_Volt_H;
GPIO_Timer_Object FanInBox;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM14_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
/* USER CODE BEGIN PFP */
void Syno24_stepper_Init();
void UART_Send_Command_SW();
void uint8ToHexString(uint8_t num, char* hexString);
void START_OLIGO_SYNTHETIC();
void PNA_wait_and_process_sync(uint16_t u16_intervaltime_process);
void update_status_and_sensor_to_SW();
void Get_sensor();
void MANUAL_RUN();
void pushdown_LowPressure_Enable();
void pushdown_LowPressure_Disable();
void pushdown_HighPressure_Enable();
void pushdown_HighPressure_Disable();
void buzzer_blink();
void syno24_fill_air2well();
void uart_send_Feedback_Status_Run(uint8_t u8_function_run, uint8_t u8_subfunction_run);
void Syno24_get_and_auto_control_Humidity();
void syno24_Control_Air_Humidity();
void send_status_machine();// 29-10-24
void process_pressure_state(uint8_t option_pressure, uint16_t procs_time, uint16_t waitting_time);
void process_pressure_state_ptr(uint8_t *option_pressure, uint16_t *procs_time, uint16_t *waitting_time);
void pressure_control(uint8_t u8_option_pressure[], uint16_t u16tb_procs_time[], uint16_t u16tb_waitting_after_time[], uint8_t num_states);
void autoPrimming_beforeCoupling();
void DNA_wait_time(uint16_t u16_intervaltime_process);
void FeatureVacuumBox();
void UpdateVacuumBox();
void DisableVacuumBox();
void EnableVacuumBox(uint16_t timeOpen);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Stepper_Run();
void CheckInput();
void buzzerBlink();
void CheckOutPut();
uint8_t State_Home_Stepper_X = 0;
uint8_t State_Home_Stepper_Y = 0;
uint8_t State_Home_Stepper_Z1 = 0;
#define DEBUG_SOFTWARE
/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_I2C1_Init();
	MX_USART1_UART_Init();
	MX_TIM14_Init();
	MX_USART2_UART_Init();
	MX_ADC1_Init();
	/* USER CODE BEGIN 2 */
	DWT_Delay_Init();
	HAL_GPIO_WritePin(CTRL_OE1_GPIO_Port, CTRL_OE1_Pin, 0);
	HAL_GPIO_WritePin(CTRL_OE2_GPIO_Port, CTRL_OE2_Pin, 0);
	HAL_GPIO_WritePin(X_EN_GPIO_Port, X_EN_Pin, 1);
	HAL_GPIO_WritePin(X_DIR_GPIO_Port,X_DIR_Pin , 0);
	HAL_GPIO_WritePin(Y_EN_GPIO_Port, Y_EN_Pin, 1);
	HAL_GPIO_WritePin(Y_DIR_GPIO_Port,Y_DIR_Pin , 0);
	HAL_GPIO_WritePin(Z1_EN_GPIO_Port, Z1_EN_Pin, 1);
	for(int i = 0; i < 8; i++)
	{
		HAL_GPIO_WritePin(SOLENOID[i].Port, SOLENOID[i].Pin, RESET);
	}
	//DWT_Delay_Init(); // init dwt delay
	//Stepper_Run();
	Syno24_stepper_Init();
	// HAL_TIM_PWM_Start(&htim5, TIM_CHANNEL_3);
	// HAL_TIM_Base_Start(&htim2);
	//__HAL_TIM_SET_COMPARE(&htim5, TIM_CHANNEL_3, 0); // TIMER5 CHANNEL 3 timer 10khz
	HAL_UART_Receive_DMA(&huart1, &global_variable.UART_Command.u8_Data_Rx_Buffer[0], UART_LENGTH_COMMAND);
	global_variable.UART_Command.b_FW_Rx_Command_Flag = 0;
	global_variable.signal_running.b_signal_update_status = 0;
	global_variable.signal_running.b_signal_connect_software = 0;
	global_variable.signal_running.b_signal_runing_oligo = false;
	global_variable.control_air.u16_counter_2second = 0;
	global_variable.status_and_sensor.u16_temperature.Data = 0;
	global_variable.status_and_sensor.u16_humidity.Data = 0;
	ADS1115_ReadADC_Single(&hi2c1, 0);
	//HAL_GPIO_WritePin(buzzer_GPIO_Port, buzzer_Pin, 0);
	State_Home_Stepper_X = HAL_GPIO_ReadPin(X_LIMIT_GPIO_Port, X_LIMIT_Pin);
	State_Home_Stepper_Y = HAL_GPIO_ReadPin(Y_LIMIT_GPIO_Port, Y_LIMIT_Pin);
	State_Home_Stepper_Z1 = HAL_GPIO_ReadPin(Z1_LIMIT_GPIO_Port, Z1_LIMIT_Pin);
	// Khoi dong GPIO cua quat
	GPIO_Timer_init(&FanInBox, SOLENOID[FAN_VACUUM_BOX].Port, SOLENOID[FAN_VACUUM_BOX].Pin);
	//buzzerBlink();
	//Stepper_Run();
#ifdef DEBUG_SOFTWARE

#else
	Stepper_AutoHome_SYN024();
#endif
	//Stepper_AutoHome_SYN024();
	//Flash_Check_ReadWrite();
	/* USER CODE END 2 */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
		if(global_variable.UART_Command.b_FW_Rx_Command_Flag == true)
		{
			global_variable.UART_Command.b_FW_Rx_Command_Flag = false; // disable flag have command uart
			memcpy(&Command[0], &global_variable.UART_Command.u8_Data_Rx_Buffer[0], UART_LENGTH_COMMAND);
			//crc_result = calcCRC(&Command, UART_LENGTH_COMMAND - 2 );
			//crc_result_data = ( Command[UART_LENGTH_COMMAND -2]<< 8) | Command[UART_LENGTH_COMMAND-1];
			switch(Command[0])
			{
			case CMD_ASK_VENDOR_ID:
			{
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_ASK_VENDOR_ID;
				global_variable.signal_running.b_signal_connect_software = true;
				UART_Send_Command_SW();
				break;
			}
			case CMD_RECIVED_SETTING:
			{
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_RECIVED_SETTING;
				for(uint8_t index_valve = 0; index_valve < MAX_NUMBER_VALVE; index_valve++)
				{
					global_variable.valve_setting[index_valve].f_a.Byte[0] = Command[index_valve*8 + 1];
					global_variable.valve_setting[index_valve].f_a.Byte[1] = Command[index_valve*8 + 2];
					global_variable.valve_setting[index_valve].f_a.Byte[2] = Command[index_valve*8 + 3];
					global_variable.valve_setting[index_valve].f_a.Byte[3] = Command[index_valve*8 + 4];
					global_variable.valve_setting[index_valve].f_b.Byte[0] = Command[index_valve*8 + 5];
					global_variable.valve_setting[index_valve].f_b.Byte[1] = Command[index_valve*8 + 6];
					global_variable.valve_setting[index_valve].f_b.Byte[2] = Command[index_valve*8 + 7];
					global_variable.valve_setting[index_valve].f_b.Byte[3] = Command[index_valve*8 + 8];
				}// 8 * 17 = 136 ket thuc index setting valve

				UART_Send_Command_SW();
				break;
			}
			case CMD_PRESURE_TESTING:
			{
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_PRESURE_TESTING;
				UART_Send_Command_SW();
				break;
			}
			case CMD_CONTROL_MIXED_AIR: // command nay ket hop push up va push down
			{
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_CONTROL_MIXED_AIR;
				buzzer_blink();
				UART_Send_Command_SW();
				break;
			}
			case CMD_FIRMWARE_UPDATE_STATUS_AND_SENSOR:
			{
				Get_sensor();
				if(Command[1] == CMD_FIRMWARE_UPDATE_STATUS_AND_SENSOR) // neu yeu cau doi do am thi bat khi vao ngay lap tuc
				{
					//HAL_GPIO_WritePin(CTRL6_OPEN_NITO_GPIO_Port, CTRL6_OPEN_NITO_Pin, SET);
					HAL_GPIO_WritePin(SOLENOID[OPEN_NITOR_SV].Port, SOLENOID[OPEN_NITOR_SV].Pin, SET);
					global_variable.status_and_sensor.flag_enable_auto_control_air_Nito = SET;
					global_variable.status_and_sensor.u16tb_humidity_Preset.Byte[0] = Command[2];
					global_variable.status_and_sensor.u16tb_humidity_Preset.Byte[1] = Command[3];
					global_variable.status_and_sensor.u8_high_limit_humidity = 	global_variable.status_and_sensor.u16tb_humidity_Preset.Data + OFFSET_LIMIT_VALUE;
					global_variable.status_and_sensor.u8_low_limit_humidity = global_variable.status_and_sensor.u16tb_humidity_Preset.Data;
				}
				global_variable.status_and_sensor.u16_temperature.Data = global_variable.status_and_sensor.f_temperature * 100;
				global_variable.status_and_sensor.u16_humidity.Data = global_variable.status_and_sensor.f_humidity * 100;
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_FIRMWARE_UPDATE_STATUS_AND_SENSOR;
				global_variable.UART_Command.u8_Data_Tx_Buffer[1] = global_variable.status_and_sensor.u16_temperature.Byte[0];
				global_variable.UART_Command.u8_Data_Tx_Buffer[2] = global_variable.status_and_sensor.u16_temperature.Byte[1];
				global_variable.UART_Command.u8_Data_Tx_Buffer[3] = global_variable.status_and_sensor.u16_humidity.Byte[0];
				global_variable.UART_Command.u8_Data_Tx_Buffer[4] = global_variable.status_and_sensor.u16_humidity.Byte[1];
				global_variable.UART_Command.u8_Data_Tx_Buffer[5] = ((getPositionX()) & 0xFF);
				global_variable.UART_Command.u8_Data_Tx_Buffer[6] = ((getPositionX() >> 8)&0xFF);
				global_variable.UART_Command.u8_Data_Tx_Buffer[7] = ((getPositionY()) & 0xFF);
				global_variable.UART_Command.u8_Data_Tx_Buffer[8] = ((getPositionY() >> 8)&0xFF);
				global_variable.UART_Command.u8_Data_Tx_Buffer[9] = ((getPositionZ1()) & 0xFF);
				global_variable.UART_Command.u8_Data_Tx_Buffer[10] = ((getPositionZ1() >> 8)&0xFF);
				UART_Send_Command_SW();
				break;
			}
			case CMD_RUN2HOME:
			{
				Stepper_Z1_move(Z_POSITION_NORMAL);
#ifdef DEBUG_SOFTWARE

#else
				//HAL_GPIO_WritePin(CTRL6_OPEN_NITO_GPIO_Port, CTRL6_OPEN_NITO_Pin, RESET);
				HAL_GPIO_WritePin(SOLENOID[OPEN_NITOR_SV].Port, SOLENOID[OPEN_NITOR_SV].Pin, RESET);
				Stepper_AutoHome_SYN024();
#endif
				Stepper_move_Coordinates_XY(180, 0);
				// ==========================	send command to Software	==================================
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_RUN2HOME;
				buzzer_blink();
				UART_Send_Command_SW();
				break;
			}

			case CMD_CONTROL_AIR_START:
			{
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_CONTROL_AIR_START;
				buzzer_blink();
				UART_Send_Command_SW();
				break;
			}
			case CMD_RUNSTEPPER:
			{
				global_variable.signal_control.u16tb_X_Distance.Byte[0] = Command[1];
				global_variable.signal_control.u16tb_X_Distance.Byte[1] = Command[2];
				global_variable.signal_control.u16tb_Y_Distance.Byte[0] = Command[3];
				global_variable.signal_control.u16tb_Y_Distance.Byte[1] = Command[4];
				global_variable.signal_control.u16tb_Z1_Distance.Byte[0] = Command[5];
				global_variable.signal_control.u16tb_Z1_Distance.Byte[1] = Command[6];
				Stepper_Z1_move(global_variable.signal_control.u16tb_Z1_Distance.Data);
				Stepper_move_Coordinates_XY( global_variable.signal_control.u16tb_X_Distance.Data, global_variable.signal_control.u16tb_Y_Distance.Data);
				// ==========================	 send command software		=========================================================
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_RUNSTEPPER;
				buzzer_blink();
				UART_Send_Command_SW();
				break;
			}

			case CMD_CALIBRATION_VALVE: // software tinh toan thoi gian phun cua Valve gui xuong - firmware chi can phun dung thoi gian
			{
				//Stepper_Z1_move(Z_POSITION_NORMAL);
				// Phun hóa chất Calibration Valve
				global_variable.primming_control.u8_valve_sellect = Command[1];
				global_variable.primming_control.u32fb_time_primming_calib.Byte[0] = Command[2];
				global_variable.primming_control.u32fb_time_primming_calib.Byte[1] = Command[3];
				global_variable.primming_control.u32fb_time_primming_calib.Byte[2] = Command[4];
				global_variable.primming_control.u32fb_time_primming_calib.Byte[3] = Command[5];
				//Stepper_Z1_move(Z_POSITION_CALIB);
				calibVolumeProcess(global_variable.primming_control.u8_valve_sellect, global_variable.primming_control.u32fb_time_primming_calib.Data, X_CALIB_POS_1, Y_CALIB_POS);
				//==================== send command software  =======================================================================
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_CALIBRATION_VALVE;
				buzzer_blink();
				Stepper_Z1_move(Z_POSITION_NORMAL);
				Stepper_move_Coordinates_XY(900 ,Y_CALIB_POS);
				UART_Send_Command_SW();
				break;
			}

			case CMD_PRIMMING: // XA BOT KHI -- Cap Nhat tinh nang nay cho tung valve 04/01/2023
			{
				Stepper_Z1_move(Z_POSITION_NORMAL);
				global_variable.primming_control.valve[0] = Command[1];
				global_variable.primming_control.valve[1] = Command[2];
				global_variable.primming_control.valve[2] = Command[3];
				global_variable.primming_control.valve[3] = Command[4];
				global_variable.primming_control.valve[4] = Command[5];
				global_variable.primming_control.valve[5] = Command[6];
				global_variable.primming_control.valve[6] = Command[7];
				global_variable.primming_control.valve[7] = Command[8];
				global_variable.primming_control.valve[8] = Command[9];
				global_variable.primming_control.valve[9] = Command[10];
				global_variable.primming_control.valve[10] = Command[11];
				global_variable.primming_control.valve[11] = Command[12];
				global_variable.primming_control.valve[12] = Command[13];
				global_variable.primming_control.valve[13] = Command[14];
				global_variable.primming_control.valve[14] = Command[15];
				global_variable.primming_control.valve[15] = Command[16];
				global_variable.primming_control.valve[16] = Command[17];
				global_variable.primming_control.u8_time_primming_control = Command[20];
				uint16_t X_Pos = 0;
				uint16_t u16_time_fill = 0;
				const uint16_t offsetX = 990;
				// PRIMMING A T G C a
				if(global_variable.primming_control.valve[A] == true  || global_variable.primming_control.valve[T] == true ||
						global_variable.primming_control.valve[G] == true  || global_variable.primming_control.valve[C] == true
						|| global_variable.primming_control.valve[a] == true )
				{
					X_Pos = Fill_Position_X[0][0] + offsetX;
					Stepper_move_Coordinates_XY(X_Pos, Y_CALIB_POS);
					Stepper_Z1_move(Z_POSITION_PRIMMING);
					for(uint8_t i = A; i<= a; i++)
					{
						if(global_variable.primming_control.valve[i])
						{
							openValve(i);
						}
					}
					u16_time_fill = global_variable.primming_control.u8_time_primming_control* 100;
					while (u16_time_fill > 0)
					{
						u16_time_fill--;
						HAL_Delay(1);
					}
					Valve_DisAll();
				}
				// t c g U I
				if(global_variable.primming_control.valve[t] == true  || global_variable.primming_control.valve[g] == true ||
						global_variable.primming_control.valve[c] == true  || global_variable.primming_control.valve[U] == true
						|| global_variable.primming_control.valve[I] == true )
				{
					X_Pos = Fill_Position_X[5][0] + offsetX;
					Stepper_move_Coordinates_XY(X_Pos, Y_CALIB_POS);
					Stepper_Z1_move(Z_POSITION_PRIMMING);
					for(uint8_t i = t; i<= U; i++)
					{
						if(global_variable.primming_control.valve[i])
						{
							openValve(i);
						}
					}
					u16_time_fill = global_variable.primming_control.u8_time_primming_control* 100;
					while (u16_time_fill > 0)
					{
						u16_time_fill--;
						HAL_Delay(1);
					}
					Valve_DisAll();
				}
				// PRIMMING Activator TCA_in_DCM  WASH_ACN_DCM  OXIDATION_IODINE OXIDATION_IODINE2
				if(global_variable.primming_control.valve[Activator] == true  || global_variable.primming_control.valve[TCA_in_DCM] == true ||
						global_variable.primming_control.valve[WASH_ACN_DCM] == true  || global_variable.primming_control.valve[OXIDATION_IODINE] == true
						|| global_variable.primming_control.valve[OXIDATION_IODINE2] == true )
				{
					X_Pos = Fill_Position_X[10][0] + offsetX;
					Stepper_move_Coordinates_XY(X_Pos, Y_CALIB_POS);
					Stepper_Z1_move(Z_POSITION_PRIMMING);
					for(uint8_t i = Activator; i<= OXIDATION_IODINE2; i++)
					{
						if(global_variable.primming_control.valve[i])
						{
							openValve(i);
						}
					}
					u16_time_fill = global_variable.primming_control.u8_time_primming_control* 100;
					while (u16_time_fill > 0)
					{
						u16_time_fill--;
						HAL_Delay(1);
					}
					Valve_DisAll();
				}
				// PRIMMING CAPPING_CAPB CAPPING_CAPA
				if(global_variable.primming_control.valve[CAPPING_CAPA] == true  || global_variable.primming_control.valve[CAPPING_CAPB] == true)
				{
					X_Pos = Fill_Position_X[15][0] + offsetX;
					Stepper_move_Coordinates_XY(X_Pos, Y_CALIB_POS);
					Stepper_Z1_move(Z_POSITION_PRIMMING);
					for(uint8_t i = CAPPING_CAPB; i<= CAPPING_CAPA; i++)
					{
						if(global_variable.primming_control.valve[i])
						{
							openValve(i);
						}
					}
					u16_time_fill = global_variable.primming_control.u8_time_primming_control* 100;
					while (u16_time_fill > 0)
					{
						u16_time_fill--;
						HAL_Delay(1);
					}
					Valve_DisAll();
				}
				Valve_DisAll(); // ngung Primming
				//==================== send command software ==============================================
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_PRIMMING;
				buzzer_blink();
				UART_Send_Command_SW();
				break;
			}
			case CMD_DATA_OLIGO:
			{
				// Get Data Oligo from software
				//==================== send command software ==============================================
				global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical = Command[1]; // stepname cua buoc nay
				global_variable.synthetic_oligo.fill_chemical.u16tb_Volume.Byte[0] = Command[2];
				global_variable.synthetic_oligo.fill_chemical.u16tb_Volume.Byte[1] = Command[3];
				global_variable.synthetic_oligo.b_douple_coupling_first_base = Command[4];
				global_variable.synthetic_oligo.fill_chemical.mix_funtion.u16tb_Volume[0].Byte[0] = Command[5];
				global_variable.synthetic_oligo.fill_chemical.mix_funtion.u16tb_Volume[0].Byte[1] = Command[6];
				global_variable.synthetic_oligo.fill_chemical.mix_funtion.u16tb_Volume[1].Byte[0] = Command[7];
				global_variable.synthetic_oligo.fill_chemical.mix_funtion.u16tb_Volume[1].Byte[1] = Command[8];
				global_variable.synthetic_oligo.fill_chemical.mix_funtion.u16tb_Volume[2].Byte[0] = Command[9];
				global_variable.synthetic_oligo.fill_chemical.mix_funtion.u16tb_Volume[2].Byte[1] = Command[10];
				global_variable.synthetic_oligo.fill_chemical.mix_funtion.u8_type_chemical[0] = Command[11];
				global_variable.synthetic_oligo.fill_chemical.mix_funtion.u8_type_chemical[1] = Command[12];
				global_variable.synthetic_oligo.fill_chemical.mix_funtion.u8_type_chemical[2] = Command[13];
				global_variable.synthetic_oligo.fill_chemical.u16tb_wait_after_fill.Byte[0] = Command[14];
				global_variable.synthetic_oligo.fill_chemical.u16tb_wait_after_fill.Byte[1] = Command[15];
				global_variable.status_and_sensor.flag_enable_auto_control_air_Nito = Command[16]; // CONTROL NITOR HAY KHONG
				global_variable.synthetic_oligo.isSpecialBase = Command[17]; // isSpecialBase
				global_variable.synthetic_oligo.u16_scale_volume.Byte[0] = Command[18];
				global_variable.synthetic_oligo.u16_scale_volume.Byte[1] = Command[19];
				global_variable.synthetic_oligo.u16_scale_time.Byte[0] = Command[20];
				global_variable.synthetic_oligo.u16_scale_time.Byte[1] = Command[21];

				//global_variable.signal_running.u16_counter_base_finished = ((Command[23]<<8) | Command[22]);

				global_variable.signal_running.u16_counter_step = Command[26]; // 07/09/2024 them gui step dang chay cho autocheck washing
				global_variable.advanced_setting.flag_auto_primming_chemical = Command[27];
				global_variable.advanced_setting.u16tb_autoPrim_volume_amidite.Byte[0] = Command[28];
				global_variable.advanced_setting.u16tb_autoPrim_volume_amidite.Byte[1] = Command[29];
				global_variable.advanced_setting.u16tb_autoPrim_volume_Activator.Byte[0] = Command[30];
				global_variable.advanced_setting.u16tb_autoPrim_volume_Activator.Byte[1] = Command[31];

				global_variable.Coupling2Setting.EnableFillWellDone = Command[32];
				global_variable.Coupling2Setting.typeReagent = Command[33];
				global_variable.Coupling2Setting.volume.Byte[0] = Command[34];
				global_variable.Coupling2Setting.volume.Byte[1] = Command[35];
				for(uint8_t u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
				{
					global_variable.synthetic_oligo.u8_well_sequence_run[u8_idx_well] = Command[idx_start_sequence_amidite + u8_idx_well];
					global_variable.synthetic_oligo.u16_timefill_func_mix_well_1[u8_idx_well].Byte[0] =  Command[idx_start_time_fill_mixfunction_1 + u8_idx_well*2];
					global_variable.synthetic_oligo.u16_timefill_func_mix_well_1[u8_idx_well].Byte[1] =  Command[idx_start_time_fill_mixfunction_1 + u8_idx_well*2 + 1];
					global_variable.synthetic_oligo.u16_timefill_func_mix_well_2[u8_idx_well].Byte[0] =  Command[idx_start_time_fill_mixfunction_2 + u8_idx_well*2];
					global_variable.synthetic_oligo.u16_timefill_func_mix_well_2[u8_idx_well].Byte[1] =  Command[idx_start_time_fill_mixfunction_2 + u8_idx_well*2 + 1];
					global_variable.synthetic_oligo.u16_timefill_func_mix_well_3[u8_idx_well].Byte[0] =  Command[idx_start_time_fill_mixfunction_3 + u8_idx_well*2];
					global_variable.synthetic_oligo.u16_timefill_func_mix_well_3[u8_idx_well].Byte[1] =  Command[idx_start_time_fill_mixfunction_3 + u8_idx_well*2 + 1];
				}
				for(uint8_t idx_process = 0; idx_process < 10; idx_process++)
				{
					global_variable.synthetic_oligo.control_pressure.u8_option_pressure[idx_process] = Command[idx_start_opt_vaccum + idx_process];
					global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[idx_process].Byte[0] = Command[idx_start_time_process + idx_process*2];
					global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[idx_process].Byte[1] = Command[idx_start_time_process + idx_process*2 + 1];
					global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[idx_process].Byte[0]  = Command[idx_start_time_wait + idx_process*2];
					global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[idx_process].Byte[1] = Command[idx_start_time_wait + idx_process*2 + 1];
				}

				int currentIdx = 350; // Biến theo dõi chỉ số hiện tại
				global_variable.advanced_setting.VacuumBox.Enablefeature = Command[currentIdx];
				currentIdx++;
				global_variable.advanced_setting.VacuumBox.En_WASH = Command[currentIdx];
				currentIdx++;
				global_variable.advanced_setting.VacuumBox.En_Deblock = Command[currentIdx];
				currentIdx++;
				global_variable.advanced_setting.VacuumBox.En_Coupling = Command[currentIdx];
				currentIdx++;
				global_variable.advanced_setting.VacuumBox.En_Ox = Command[currentIdx];
				currentIdx++;
				global_variable.advanced_setting.VacuumBox.En_Cap = Command[currentIdx];
				currentIdx++;
				global_variable.advanced_setting.VacuumBox.time.Byte[0] = Command[currentIdx];
				currentIdx++;
				global_variable.advanced_setting.VacuumBox.time.Byte[1]  = Command[currentIdx];
				currentIdx++;


				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_DATA_OLIGO;

				UART_Send_Command_SW();
				break;
			}
			case CMD_START_OLIGO_STEP: // start synthetic oligo
			{
				global_variable.signal_running.b_signal_runing_oligo = true;
				//==================== send command software ==============================
				Stepper_Z1_move(Z_POSITION_NORMAL);
				START_OLIGO_SYNTHETIC();
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_START_OLIGO_STEP;
				global_variable.signal_running.b_signal_runing_oligo = false;
				buzzer_blink();
				UART_Send_Command_SW();
				break;
			}
			case CMD_OX_SENQUENCE:
			{
				for(uint8_t u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
				{
					global_variable.synthetic_oligo.u8_ox_sequence[u8_idx_well] = Command[u8_idx_well + 1];
				}
				global_variable.signal_running.u16_counter_base_finished = ((Command[151]<<8) | Command[150]);
				for(uint8_t u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
				{
					global_variable.synthetic_oligo.u8_well_sequence[u8_idx_well] = Command[u8_idx_well + 97];
					if(global_variable.signal_running.u16_counter_base_finished == 0)
					{
						global_variable.synthetic_oligo.wellFirstsequence[u8_idx_well] = Command[u8_idx_well + 97];
					}
				}
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_OX_SENQUENCE;
				UART_Send_Command_SW();
				break;
			}
			case CMD_MANUAL_RUN:
			{
				Stepper_Z1_move(Z_POSITION_NORMAL);
				global_variable.manual_run.U8_TASK_CONTROL = Command[25];
				global_variable.manual_run.u8_typeof_chemical = Command[26];
				global_variable.manual_run.u16_volume.Byte[0] = Command[27];
				global_variable.manual_run.u16_volume.Byte[1] = Command[28];
				global_variable.manual_run.u8_option_pressure[0] = Command[29];
				global_variable.manual_run.u16tb_procs_time[0].Byte[0] = Command[30];
				global_variable.manual_run.u16tb_procs_time[0].Byte[1] = Command[31];
				global_variable.manual_run.u16tb_waitting_after_time[0].Byte[0] = Command[32];
				global_variable.manual_run.u16tb_waitting_after_time[0].Byte[1] = Command[33];
				global_variable.manual_run.u8_option_pressure[1] = Command[34];
				global_variable.manual_run.u16tb_procs_time[1].Byte[0] = Command[35];
				global_variable.manual_run.u16tb_procs_time[1].Byte[1] = Command[36];
				global_variable.manual_run.u16tb_waitting_after_time[1].Byte[0] = Command[37];
				global_variable.manual_run.u16tb_waitting_after_time[1].Byte[1] = Command[38];
				global_variable.manual_run.u8_option_pressure[2] = Command[39];
				global_variable.manual_run.u16tb_procs_time[2].Byte[0] = Command[40];
				global_variable.manual_run.u16tb_procs_time[2].Byte[1] = Command[41];
				global_variable.manual_run.u16tb_waitting_after_time[2].Byte[0] = Command[42];
				global_variable.manual_run.u16tb_waitting_after_time[2].Byte[1] = Command[43];
				global_variable.manual_run.u8_option_pressure[3] = Command[44];
				global_variable.manual_run.u16tb_procs_time[3].Byte[0] = Command[45];
				global_variable.manual_run.u16tb_procs_time[3].Byte[1] = Command[46];
				global_variable.manual_run.u16tb_waitting_after_time[3].Byte[0] = Command[47];
				global_variable.manual_run.u16tb_waitting_after_time[3].Byte[1] = Command[48];

				for(uint8_t idx_valve = 0; idx_valve < MAX_WELL_AMIDITE; idx_valve++)
				{
					global_variable.manual_run.u8_checked_well[idx_valve] = Command[idx_valve + 1];
				}
				MANUAL_RUN();
				// global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_MANUAL_RUN;
				buzzer_blink();
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_MANUAL_RUN;
				UART_Send_Command_SW();
				break;
			}
			case CMD_STOP_SYSTHETIC_OLIGO:
			{
				// 21-03 thêm tính năng bật quạt sau khi hoàn thành chu trình chạy
				Valve_DisAll();
				global_variable.advanced_setting.flag_exhaustFan = Command[1];
				global_variable.advanced_setting.u16tb_timeExhaustFan.Byte[0] = Command[2];
				global_variable.advanced_setting.u16tb_timeExhaustFan.Byte[1] = Command[3];
				// FAN_SV
				// HAL_GPIO_WritePin(SOLENOID[FAN_SV].Port, SOLENOID[FAN_SV].Pin, global_variable.signal_control.stateControlSolenoid[i]);
				HAL_GPIO_WritePin(SOLENOID[FAN_SV].Port, SOLENOID[FAN_SV].Pin, SET);
				if(global_variable.advanced_setting.flag_exhaustFan)
				{
					HAL_Delay(global_variable.advanced_setting.u16tb_timeExhaustFan.Data * 60 * 1000); // software phut
				}
				HAL_GPIO_WritePin(SOLENOID[FAN_SV].Port, SOLENOID[FAN_SV].Pin, RESET);
				global_variable.signal_running.b_signal_runing_oligo = false;
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_STOP_SYSTHETIC_OLIGO;
				UART_Send_Command_SW();
				break;
			}
			case CMD_REQUEST_PAUSE:
			{
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_REQUEST_PAUSE;
				UART_Send_Command_SW();
				break;
			}
			case CMD_FIRMWARE_REQUEST_DATA_AGAIN:
			{
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_FIRMWARE_REQUEST_DATA_AGAIN;
				UART_Send_Command_SW();
				break;
			}
			case CMD_CONTROL_SYNCHRONIZE_IO:
			{
				uint8_t index = 1;
				for(uint8_t i = 0; i < MAX_NUMBER_VALVE; i++)
				{
					global_variable.signal_control.stateControlValve[i] = Command[index];
					HAL_GPIO_WritePin(VALVE[i].Port, VALVE[i].Pin, global_variable.signal_control.stateControlValve[i]);
					index++;
				}

				// have 8 solenoid
				for(uint8_t i = 0; i < 8; i++)
				{
					global_variable.signal_control.stateControlSolenoid[i] = Command[index];
					HAL_GPIO_WritePin(SOLENOID[i].Port, SOLENOID[i].Pin, global_variable.signal_control.stateControlSolenoid[i]);
					index++;
				}
				// response data software
				global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_CONTROL_SYNCHRONIZE_IO;
				UART_Send_Command_SW();
				break;
			}
			default:
			{
				break;
			}
			} // end switch check command
		} // end if flag have command

	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 4;
	RCC_OscInitStruct.PLL.PLLN = 168;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 4;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
	{
		Error_Handler();
	}
}

/**
 * @brief ADC1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC1_Init(void)
{

	/* USER CODE BEGIN ADC1_Init 0 */

	/* USER CODE END ADC1_Init 0 */

	ADC_ChannelConfTypeDef sConfig = {0};

	/* USER CODE BEGIN ADC1_Init 1 */

	/* USER CODE END ADC1_Init 1 */

	/** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
	 */
	hadc1.Instance = ADC1;
	hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc1.Init.Resolution = ADC_RESOLUTION_12B;
	hadc1.Init.ScanConvMode = DISABLE;
	hadc1.Init.ContinuousConvMode = DISABLE;
	hadc1.Init.DiscontinuousConvMode = DISABLE;
	hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc1.Init.NbrOfConversion = 1;
	hadc1.Init.DMAContinuousRequests = DISABLE;
	hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc1) != HAL_OK)
	{
		Error_Handler();
	}

	/** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
	 */
	sConfig.Channel = ADC_CHANNEL_15;
	sConfig.Rank = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
	if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN ADC1_Init 2 */

	/* USER CODE END ADC1_Init 2 */

}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void)
{

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.ClockSpeed = 100000;
	hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief TIM14 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM14_Init(void)
{

	/* USER CODE BEGIN TIM14_Init 0 */

	/* USER CODE END TIM14_Init 0 */

	/* USER CODE BEGIN TIM14_Init 1 */

	/* USER CODE END TIM14_Init 1 */
	htim14.Instance = TIM14;
	htim14.Init.Prescaler = 8399;
	htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim14.Init.Period = 29999;
	htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
	if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN TIM14_Init 2 */

	/* USER CODE END TIM14_Init 2 */

}

/**
 * @brief USART1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART1_UART_Init(void)
{

	/* USER CODE BEGIN USART1_Init 0 */

	/* USER CODE END USART1_Init 0 */

	/* USER CODE BEGIN USART1_Init 1 */

	/* USER CODE END USART1_Init 1 */
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART1_Init 2 */

	/* USER CODE END USART1_Init 2 */

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void)
{

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 115200;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	if (HAL_UART_Init(&huart2) != HAL_OK)
	{
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * Enable DMA controller clock
 */
static void MX_DMA_Init(void)
{

	/* DMA controller clock enable */
	__HAL_RCC_DMA2_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA2_Stream5_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream5_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream5_IRQn);
	/* DMA2_Stream7_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	/* USER CODE BEGIN MX_GPIO_Init_1 */
	/* USER CODE END MX_GPIO_Init_1 */

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_GPIOD_CLK_ENABLE();
	__HAL_RCC_GPIOG_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE, V1_Pin|V2_Pin|V3_Pin|V4_Pin
			|V5_Pin|PWM1_Pin|PWM4_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, V6_Pin|V20_Pin|V21_Pin|V22_Pin
			|V23_Pin|V27_Pin|V26_Pin|V25_Pin
			|V24_Pin|X_STEP_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOF, V7_Pin|V8_Pin|V9_Pin|V10_Pin
			|V11_Pin|V12_Pin|V13_Pin|V14_Pin
			|V15_Pin|V16_Pin|V17_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOB, V19_Pin|V18_Pin|buzzer_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOD, V39_Pin|V40_Pin|V38_Pin|V37_Pin
			|V36_Pin|V35_Pin|X_EN_Pin|Y_DIR_Pin
			|Y_STEP_Pin|CTRL_OE2_Pin|Y_EN_Pin|Z1_EN_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOG, V34_Pin|V33_Pin|V32_Pin|V31_Pin
			|V30_Pin|V29_Pin|V28_Pin|Z1_DIR_Pin
			|Z1_STEP_Pin|CTRL_OE1_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(X_DIR_GPIO_Port, X_DIR_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pins : V1_Pin V2_Pin V3_Pin V4_Pin
                           V5_Pin PWM1_Pin PWM4_Pin */
	GPIO_InitStruct.Pin = V1_Pin|V2_Pin|V3_Pin|V4_Pin
			|V5_Pin|PWM1_Pin|PWM4_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : V6_Pin V20_Pin V21_Pin V22_Pin
                           V23_Pin V27_Pin V26_Pin V25_Pin
                           V24_Pin */
	GPIO_InitStruct.Pin = V6_Pin|V20_Pin|V21_Pin|V22_Pin
			|V23_Pin|V27_Pin|V26_Pin|V25_Pin
			|V24_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pins : V7_Pin V8_Pin V9_Pin V10_Pin
                           V11_Pin V12_Pin V13_Pin V14_Pin
                           V15_Pin V16_Pin V17_Pin */
	GPIO_InitStruct.Pin = V7_Pin|V8_Pin|V9_Pin|V10_Pin
			|V11_Pin|V12_Pin|V13_Pin|V14_Pin
			|V15_Pin|V16_Pin|V17_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

	/*Configure GPIO pins : V19_Pin V18_Pin */
	GPIO_InitStruct.Pin = V19_Pin|V18_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : V39_Pin V40_Pin V38_Pin V37_Pin
                           V36_Pin V35_Pin CTRL_OE2_Pin */
	GPIO_InitStruct.Pin = V39_Pin|V40_Pin|V38_Pin|V37_Pin
			|V36_Pin|V35_Pin|CTRL_OE2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pins : V34_Pin V33_Pin V32_Pin V31_Pin
                           V30_Pin V29_Pin V28_Pin CTRL_OE1_Pin */
	GPIO_InitStruct.Pin = V34_Pin|V33_Pin|V32_Pin|V31_Pin
			|V30_Pin|V29_Pin|V28_Pin|CTRL_OE1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/*Configure GPIO pin : X_DIR_Pin */
	GPIO_InitStruct.Pin = X_DIR_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(X_DIR_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pin : X_STEP_Pin */
	GPIO_InitStruct.Pin = X_STEP_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(X_STEP_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : X_EN_Pin Y_DIR_Pin Y_STEP_Pin Y_EN_Pin
                           Z1_EN_Pin */
	GPIO_InitStruct.Pin = X_EN_Pin|Y_DIR_Pin|Y_STEP_Pin|Y_EN_Pin
			|Z1_EN_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

	/*Configure GPIO pins : Z1_DIR_Pin Z1_STEP_Pin */
	GPIO_InitStruct.Pin = Z1_DIR_Pin|Z1_STEP_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

	/*Configure GPIO pin : buzzer_Pin */
	GPIO_InitStruct.Pin = buzzer_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
	HAL_GPIO_Init(buzzer_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : X_LIMIT_Pin Y_LIMIT_Pin */
	GPIO_InitStruct.Pin = X_LIMIT_Pin|Y_LIMIT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/*Configure GPIO pins : Z1_LIMIT_Pin Z2_LIMIT_Pin */
	GPIO_InitStruct.Pin = Z1_LIMIT_Pin|Z2_LIMIT_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/* USER CODE BEGIN MX_GPIO_Init_2 */
	/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void Stepper_Run()
{
	HAL_GPIO_WritePin(X_DIR_GPIO_Port,X_DIR_Pin , 1); // reverce v�? công tắc
	for(int i = 0; i < 20000; i++)
	{
		HAL_GPIO_TogglePin(X_STEP_GPIO_Port, X_STEP_Pin);
		DWT_Delay_us(100);
	}
	HAL_GPIO_WritePin(Y_DIR_GPIO_Port,Y_DIR_Pin , 1); // v�? phía công tắc hành trình
	for(int i = 0; i < 20000; i++)
	{
		HAL_GPIO_TogglePin(Y_STEP_GPIO_Port, Y_STEP_Pin);
		DWT_Delay_us(100);
	}
	HAL_Delay(200);
	//	HAL_GPIO_WritePin(X_DIR_GPIO_Port,X_DIR_Pin, 0 );
	//	for(int i =0; i < 10000; i++)
	//	{
	//		HAL_GPIO_TogglePin(X_STEP_GPIO_Port, X_STEP_Pin);
	//		DWT_Delay_us(100);
	//	}
	//	HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Y_DIR_Pin , 0);
	//	for(int i =0; i < 10000; i++)
	//	{
	//		HAL_GPIO_TogglePin(Y_STEP_GPIO_Port, Y_STEP_Pin);
	//		DWT_Delay_us(100);
	//	}
	//	HAL_Delay(200);

	//	HAL_GPIO_WritePin(Z1_DIR_GPIO_Port, Z1_DIR_Pin , 0); // đi xuống
	//	for(int i = 0; i < 1000; i++)
	//	{
	//		HAL_GPIO_TogglePin(Z1_STEP_GPIO_Port, Z1_STEP_Pin);
	//		DWT_Delay_us(100);
	//	}
	//
	//	HAL_GPIO_WritePin(Z1_DIR_GPIO_Port, Z1_DIR_Pin , 1); // �?I LÊN
	//	for(int i = 0; i < 1000; i++)
	//	{
	//		HAL_GPIO_TogglePin(Z1_STEP_GPIO_Port, Z1_STEP_Pin);// v�? phía công tắc hành trình
	//		DWT_Delay_us(100);
	//	}
}


void CheckInput()
{
	if(HAL_GPIO_ReadPin(X_LIMIT_GPIO_Port, X_LIMIT_Pin) == 0)
	{
		buzzerBlink();
	}
	if(HAL_GPIO_ReadPin(Y_LIMIT_GPIO_Port, Y_LIMIT_Pin) == 0)
	{
		buzzerBlink();
	}
	if(HAL_GPIO_ReadPin(Z1_LIMIT_GPIO_Port, Z1_LIMIT_Pin) == 0)
	{
		buzzerBlink();
	}
	if(HAL_GPIO_ReadPin(Z2_LIMIT_GPIO_Port, Z2_LIMIT_Pin) == 0)
	{
		buzzerBlink();
	}
}

void CheckOutPut()
{
	for(int i = 0; i < 10; i++)
	{
		HAL_GPIO_WritePin(VALVE[i].Port, VALVE[i].Pin, 1);
	}
	HAL_Delay(2500);
	for(int i = 0; i < 10; i++)
	{
		HAL_GPIO_WritePin(VALVE[i].Port, VALVE[i].Pin, 0);
	}
	for(int i = 10; i < 20; i++)
	{
		HAL_GPIO_WritePin(VALVE[i].Port, VALVE[i].Pin, 1);
	}
	HAL_Delay(2500);
	for(int i = 10; i < 20; i++)
	{
		HAL_GPIO_WritePin(VALVE[i].Port, VALVE[i].Pin, 0);
	}
	for(int i = 20; i < 30; i++)
	{
		HAL_GPIO_WritePin(VALVE[i].Port, VALVE[i].Pin, 1);
	}
	HAL_Delay(2500);
	for(int i = 20; i < 30; i++)
	{
		HAL_GPIO_WritePin(VALVE[i].Port, VALVE[i].Pin, 0);
	}
	for(int i = 30; i < 40; i++)
	{
		HAL_GPIO_WritePin(VALVE[i].Port, VALVE[i].Pin, 1);
	}
	HAL_Delay(2500);
	for(int i = 30; i < 40; i++)
	{
		HAL_GPIO_WritePin(VALVE[i].Port, VALVE[i].Pin, 0);
	}
}

void buzzerBlink()
{
	HAL_GPIO_TogglePin(buzzer_GPIO_Port, buzzer_Pin);
	HAL_Delay(200);
	HAL_GPIO_TogglePin(buzzer_GPIO_Port, buzzer_Pin);
	HAL_Delay(500);
	HAL_GPIO_TogglePin(buzzer_GPIO_Port, buzzer_Pin);
	HAL_Delay(200);
	HAL_GPIO_TogglePin(buzzer_GPIO_Port, buzzer_Pin);
	HAL_Delay(500);
}

void checkHardware()
{
	//		State_Input[0] = HAL_GPIO_ReadPin(X_LIMIT_GPIO_Port, X_LIMIT_Pin);
	//		State_Input[1] = HAL_GPIO_ReadPin(Y_LIMIT_GPIO_Port, Y_LIMIT_Pin);
	//		State_Input[2] = HAL_GPIO_ReadPin(Z1_LIMIT_GPIO_Port, Z1_LIMIT_Pin);
	//		State_Input[3] = HAL_GPIO_ReadPin(Z2_LIMIT_GPIO_Port, Z2_LIMIT_Pin);
	//HAL_GPIO_TogglePin(PWM4_GPIO_Port, PWM4_Pin);
	//HAL_Delay(2000);
	Stepper_Run();
	//CheckInput();
	CheckOutPut();

	//	Stepper_Run();
	//	CheckInput();
	//	State_Home_Stepper_X = HAL_GPIO_ReadPin(X_LIMIT_GPIO_Port, X_LIMIT_Pin);
	//	State_Home_Stepper_Y = HAL_GPIO_ReadPin(Y_LIMIT_GPIO_Port, Y_LIMIT_Pin);
	//	State_Home_Stepper_Z1 = HAL_GPIO_ReadPin(Z1_LIMIT_GPIO_Port, Z1_LIMIT_Pin);
}


//==================================================================================================
void buzzer_blink()
{
	HAL_GPIO_WritePin(buzzer_GPIO_Port, buzzer_Pin, SET);
	DWT_Delay_ms(60);
	HAL_GPIO_WritePin(buzzer_GPIO_Port, buzzer_Pin, RESET);
	DWT_Delay_ms(50);
	HAL_GPIO_WritePin(buzzer_GPIO_Port, buzzer_Pin, SET);
	DWT_Delay_ms(60);
	HAL_GPIO_WritePin(buzzer_GPIO_Port, buzzer_Pin, RESET);
}
//================================================================================================================
/**
 *  UART_Send_Command_SW - response command
 */

void UART_Send_Command_SW()
{
	HAL_TIM_Base_Stop_IT(&htim14);
	memset(&global_variable.UART_Command.u8_Data_Rx_Buffer[0], 0, UART_LENGTH_COMMAND);
	//HAL_UART_Transmit(&huart1, &global_variable.UART_Command.u8_Data_Tx_Buffer[0], UART_LENGTH_COMMAND_TX, 5000);
	// 04-11-24 function DMA add
	// testing timming and data ?? waitting test and valid data
	// GiangLH da test function DMA cần kiểm tra thêm khi chạy
	HAL_UART_Transmit_DMA(&huart1, &global_variable.UART_Command.u8_Data_Tx_Buffer[0], UART_LENGTH_COMMAND_TX);
	HAL_TIM_Base_Start_IT(&htim14);
}
/**
 * HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)
	{

		global_variable.UART_Command.b_FW_Rx_Command_Flag  = true; // have command from software
		HAL_UART_Receive_DMA(&huart1, &global_variable.UART_Command.u8_Data_Rx_Buffer[0], UART_LENGTH_COMMAND); // copy data to buffer
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == htim14.Instance)
	{
		if(global_variable.signal_running.b_signal_runing_oligo)
		{
			global_variable.control_air.u16_counter_2second ++;
			if(global_variable.control_air.u16_counter_2second == 60)
			{
				global_variable.signal_running.b_signal_update_status = true;
			}
		}
	}
}


void START_OLIGO_SYNTHETIC()
{
	Syno24_get_and_auto_control_Humidity();
	const uint8_t df_timming_delay_fill_ms = 2;
	static uint8_t column_x;
	static uint8_t row_y;
	static uint8_t u8_idx_fnc_mix;
	static uint8_t u8_position_y;
	u8_position_y = 0;
	column_x = 0;
	row_y = 0;
	u8_idx_fnc_mix = 0;
	u8_position_y = 0;
	// BAT DAU BOM HOA CHAT
	Stepper_Z1_move(Z_POSITION_NORMAL);
	// tat ngat nhan
	HAL_TIM_Base_Stop_IT(&htim14);
	if(global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == COUPLING
			||  global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == CAPPING
			|| global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == COUPLING2)
	{
		// enable primming bo hoa chat loi
		// primming amidite 02.09.2024 tinh nang nay tranh hoa chat hu hong
		// 10-02-2025 them tu dong primming first base
		if(global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == COUPLING
				|| global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == COUPLING2)
		{
			// tu dong primming base 0
			if(global_variable.signal_running.u16_counter_base_finished == 0)
			{
				// primming ammidite
				// can dua len software
				const uint16_t offsetX = 990;
				uint16_t X_Pos = 0;
				uint16_t u16_time_fill = 100;
				X_Pos = Fill_Position_X[0][0] + offsetX;
				Stepper_move_Coordinates_XY(X_Pos, Y_CALIB_POS);
				Stepper_Z1_move(Z_POSITION_PRIMMING);
				// primming amidite A T G C
				X_Pos = Fill_Position_X[0][0] + offsetX;
				Stepper_move_Coordinates_XY(X_Pos, Y_CALIB_POS);
				Stepper_Z1_move(Z_POSITION_PRIMMING);
				// 15-10-2025
				// uint8_t i = A; i<= a; i++) sua khong primming amidite modify nữa
				for(uint8_t i = A; i<= C; i++)
				{
					openValve(i);
				}
				u16_time_fill =  valve_calculator_timefill(&global_variable, A , 15);
				while (u16_time_fill > 0)
				{
					u16_time_fill--;
					HAL_Delay(1);
				}
				//DWT_Delay_ms();
				Valve_DisAll();

				// t c g U I
				// 15-10-2025
				// uint8_t i = A; i<= a; i++) sua khong primming amidite modify nữa
				/*
				X_Pos = Fill_Position_X[5][0] + offsetX;
				Stepper_move_Coordinates_XY(X_Pos, Y_CALIB_POS);
				Stepper_Z1_move(Z_POSITION_PRIMMING);
				for(uint8_t i = t; i<= U; i++)
				{
					openValve(i);
				}
				u16_time_fill = valve_calculator_timefill(&global_variable, t , 15);
				while (u16_time_fill > 0)
				{
					u16_time_fill--;
					HAL_Delay(1);
				}
				Valve_DisAll();
				*/
				Valve_DisAll();
			}
			autoPrimming_beforeCoupling();
			Stepper_Z1_Run2Normal();
		}
		u8_index_well = 0;
		for( u8_idx_fnc_mix = 0; u8_idx_fnc_mix < 3; u8_idx_fnc_mix++)
		{
			if(global_variable.synthetic_oligo.fill_chemical.mix_funtion.u8_type_chemical[u8_idx_fnc_mix] == CLG_AMIDITE)
			{
				Amidite_process(&global_variable, u8_idx_fnc_mix);

			}
			else
			{
				column_x = 0;
				row_y = 0;
				// 20-05-2025 // neu la capping thi chay theo sequenceRaw tuc la cac cot
				if(global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == CAPPING)
				{
					for(column_x = 0; column_x < 3; column_x++ ) // chay 3 COT -- toa độ X
					{
						if(column_x % 2 == 0)
						{
							for(row_y = 0; row_y < 8; row_y++ ) // chay 8 hang
							{
								if(global_variable.synthetic_oligo.u8_well_sequence[column_x * 8 + row_y]!= CHEMICAL_SUBTANCE_EMPTY) // kiem tra xem da ket thuc chu trinh bom hoa chat chua
								{
									Chemical_fill_process(&global_variable, global_variable.synthetic_oligo.fill_chemical.mix_funtion.u8_type_chemical[u8_idx_fnc_mix],
											global_variable.synthetic_oligo.fill_chemical.mix_funtion.u16tb_Volume[u8_idx_fnc_mix].Data, column_x, row_y);
									DWT_Delay_ms(df_timming_delay_fill_ms);
								}
							}
						}
						else
						{
							for(row_y = 0; row_y < 8; row_y++ ) // chay 8 hang
							{
								if(global_variable.synthetic_oligo.u8_well_sequence[column_x * 8 +  7 - row_y]!= CHEMICAL_SUBTANCE_EMPTY) // kiem tra xem da ket thuc chu trinh bom hoa chat chua
								{
									u8_position_y = 7 - row_y;
									Chemical_fill_process(&global_variable, global_variable.synthetic_oligo.fill_chemical.mix_funtion.u8_type_chemical[u8_idx_fnc_mix],
											global_variable.synthetic_oligo.fill_chemical.mix_funtion.u16tb_Volume[u8_idx_fnc_mix].Data, column_x, u8_position_y);
									DWT_Delay_ms(df_timming_delay_fill_ms);
								}
							}
						}
					}// end for vong lap bom hoa chat for(column_x = 0; column_x < 3; column_x++ ) // chay 3 COT -- t�?a độ X
				}
				else // coupling 1 va coupling 2 thi chay theo sequenceRun da duoc tinh toan san software 20-05-2025
				{
					for(column_x = 0; column_x < 3; column_x++ ) // chay 3 COT -- toa độ X
					{
						if(column_x % 2 == 0)
						{
							for(row_y = 0; row_y < 8; row_y++ ) // chay 8 hang
							{
								if(global_variable.synthetic_oligo.u8_well_sequence_run[column_x * 8 + row_y]!= CHEMICAL_SUBTANCE_EMPTY) // kiem tra xem da ket thuc chu trinh bom hoa chat chua
								{
									Chemical_fill_process(&global_variable, global_variable.synthetic_oligo.fill_chemical.mix_funtion.u8_type_chemical[u8_idx_fnc_mix],
											global_variable.synthetic_oligo.fill_chemical.mix_funtion.u16tb_Volume[u8_idx_fnc_mix].Data, column_x, row_y);
									DWT_Delay_ms(df_timming_delay_fill_ms);
								}
							}
						}
						else
						{
							for(row_y = 0; row_y < 8; row_y++ ) // chay 8 hang
							{
								if(global_variable.synthetic_oligo.u8_well_sequence_run[column_x * 8 +  7 - row_y]!= CHEMICAL_SUBTANCE_EMPTY) // kiem tra xem da ket thuc chu trinh bom hoa chat chua
								{
									u8_position_y = 7 - row_y;
									Chemical_fill_process(&global_variable, global_variable.synthetic_oligo.fill_chemical.mix_funtion.u8_type_chemical[u8_idx_fnc_mix],
											global_variable.synthetic_oligo.fill_chemical.mix_funtion.u16tb_Volume[u8_idx_fnc_mix].Data, column_x, u8_position_y);
									DWT_Delay_ms(df_timming_delay_fill_ms);
								}
							}
						}
					}// end for vong lap bom hoa chat for(column_x = 0; column_x < 3; column_x++ ) // chay 3 COT -- t�?a độ X
				}
			} // END IF AMIDITE
		} // ket thuc function mix
	}// neu lua chon hoa chat la coupling va Cap
	else // KHONG PHAI COUPLING va CAP hoac COUPLING2 them 05-04-2025
	{
		column_x = 0;
		row_y = 0;
		u8_position_y = 0;
		// get time fill buffer N
		//uint16_t u16_time_fill_N_buffer = valve_calculator_timefill(&global_variable, U, global_variable.synthetic_oligo.u16tb_volume_N_buffer.Data);
		for(column_x = 0; column_x < 3; column_x++ ) // chay 3 COT -- t�?a độ X
		{
			if(column_x % 2 == 0)
			{
				for(row_y = 0; row_y < 8; row_y++ ) // chay 8 hang
				{
					// u8_well_sequence
					if(global_variable.synthetic_oligo.u8_well_sequence[column_x * 8 + row_y]!= CHEMICAL_SUBTANCE_EMPTY) // kiem tra xem da ket thuc chu trinh bom hoa chat chua
					{
						if(global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == OXIDATION_IODINE
								|| global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == OXIDATION_IODINE2 )
						{
							// global_variable.synthetic_oligo.u8_ox_sequence[column_x * 8 + row_y] == 1
							if(global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == OXIDATION_IODINE
									&& global_variable.synthetic_oligo.u8_ox_sequence[column_x * 8 + row_y] == 1)
							{
								Chemical_fill_process(&global_variable, global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical,
										global_variable.synthetic_oligo.fill_chemical.u16tb_Volume.Data, column_x, row_y);
								DWT_Delay_ms(df_timming_delay_fill_ms);
							}
							if(global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == OXIDATION_IODINE2
									&& global_variable.synthetic_oligo.u8_ox_sequence[column_x * 8 + row_y] == 2)
							{
								Chemical_fill_process(&global_variable, global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical,
										global_variable.synthetic_oligo.fill_chemical.u16tb_Volume.Data, column_x, row_y);
								DWT_Delay_ms(df_timming_delay_fill_ms);
							}
						}
						else
						{
							Chemical_fill_process(&global_variable, global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical,
									global_variable.synthetic_oligo.fill_chemical.u16tb_Volume.Data, column_x, row_y);
							DWT_Delay_ms(df_timming_delay_fill_ms);
						}
					}
					else
					{
					}
				}
			}
			else
			{
				for(row_y = 0; row_y < 8; row_y++ ) // chay 8 hang
				{
					//
					if(global_variable.synthetic_oligo.u8_well_sequence[column_x * 8 + 7 - row_y]!= CHEMICAL_SUBTANCE_EMPTY) // kiem tra xem da ket thuc chu trinh bom hoa chat chua
					{
						u8_position_y =  7 - row_y;
						// neu la Oxidation 1 hoac Oxidation 2
						if(global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == OXIDATION_IODINE
								|| global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical ==  OXIDATION_IODINE2)
						{
							// kiem tra oxidation 1
							if(global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == OXIDATION_IODINE
									&& global_variable.synthetic_oligo.u8_ox_sequence[column_x * 8 + 7 - row_y] == 1)
							{
								Chemical_fill_process(&global_variable, global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical,
										global_variable.synthetic_oligo.fill_chemical.u16tb_Volume.Data, column_x, u8_position_y);
								DWT_Delay_ms(df_timming_delay_fill_ms);
							}
							// neu la ox2 thi phai kiem tra sequence ox2
							if(global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == OXIDATION_IODINE2
									&& global_variable.synthetic_oligo.u8_ox_sequence[column_x * 8 + 7 - row_y] == 2)
							{
								Chemical_fill_process(&global_variable, global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical,
										global_variable.synthetic_oligo.fill_chemical.u16tb_Volume.Data, column_x, u8_position_y);
								DWT_Delay_ms(df_timming_delay_fill_ms);
							}
						}
						else
						{
							Chemical_fill_process(&global_variable, global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical,
									global_variable.synthetic_oligo.fill_chemical.u16tb_Volume.Data, column_x, u8_position_y);
							DWT_Delay_ms(df_timming_delay_fill_ms);
						}
					}
					else
					{
					}
				}
			}
		}// end for vong lap bom hoa chat
	}// ket thuc bom hoa chat khong phải Coupling Cap mixed Function
	// 09-05-2025 gianglh22 chuc nang fill buffer amidte cho coupling2
	if(global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == COUPLING2
			&& global_variable.Coupling2Setting.EnableFillWellDone == true )
	{
		Coupling2FillBufferProcess(&global_variable);
	}
	// kiem tra xem co sequence OX2	khong , neu co thi thuc hien nhu mot buoc binh thuong khong thi khong can
	//
	uint8_t have_ox2 =0;
	for(int i = 0; i < 24; i++)
	{
		if(global_variable.synthetic_oligo.u8_ox_sequence[i] == 2)
		{
			have_ox2 = true;
			break;
		}
	}
	// kiem tra sequence Coupling 2
	bool have_coupling2 = false;
	for(int i = 0; i < 24; i++)
	{
		if(global_variable.synthetic_oligo.u8_well_sequence_run[i] != CHEMICAL_SUBTANCE_EMPTY)
		{
			have_coupling2 = true;
			break;
		}
	}
	// kiem tra xem co can day hoac ep khong
	bool need_progress = false;
	for(int i = 0; i < 24; i++)
	{
		if(global_variable.synthetic_oligo.u8_well_sequence_run[i] != CHEMICAL_SUBTANCE_EMPTY &&
				global_variable.synthetic_oligo.u8_well_sequence[i] != CHEMICAL_SUBTANCE_EMPTY	)
		{
			need_progress = true;
			break;
		}
	}
	//xử lý nếu có Ox2
	if(global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == OXIDATION_IODINE2 || global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == COUPLING2)
	{
		if((have_ox2 == true && global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == OXIDATION_IODINE2 )
				||( have_coupling2 == true && global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical == COUPLING2 ))
		{
			uart_send_Feedback_Status_Run(0, WAIT_AFTERFILL); // update function  = 0 count = 0 la state bom va doi
			DWT_Delay_ms(global_variable.synthetic_oligo.fill_chemical.u16tb_wait_after_fill.Data);
			// open FAN IN BOX 13-08-2025
			FeatureVacuumBox();

			for(uint8_t counter_state = 0; counter_state < 10; counter_state++)
			{
				// kiem tra va dieu khien do am
				Syno24_get_and_auto_control_Humidity();
				switch(global_variable.synthetic_oligo.control_pressure.u8_option_pressure[counter_state])
				{
				case HIGH_VACUUM:
				{
					if((global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data != 0) &&
							global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data != 0)
					{
					}
					break;
				}
				case LOW_VACUUM:
				{
					if((global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data != 0) &&
							global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data != 0)
					{
					}
					break;
				}
				case HIGH_PUSH:
				{
					if((global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data != 0) &&
							global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data != 0)
					{
						pushdown_HighPressure_Enable();
						uart_send_Feedback_Status_Run(counter_state, PUSHDOWN_FNC); // update function progress for software ui
						DWT_Delay_ms(global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data); // waitting time
						pushdown_HighPressure_Disable();
						//pushdown_compression_disable();
#ifdef SYNO24_PNA
						global_variable.UART_Command.u8_Data_Tx_Buffer[2] = global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Byte[0];
						global_variable.UART_Command.u8_Data_Tx_Buffer[3] = global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Byte[1];
						// SEND FOR START CLOCKDOWN COUNTER
						uart_send_Feedback_Status_Run(1);
						// DELAY for pressure stable
						PNA_wait_and_process_sync(global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Data);
#else
						uart_send_Feedback_Status_Run(counter_state, WAIT_FNC); // update function progress for software ui 2 la wait
						DNA_wait_time(global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Data); // waitting time

#endif
						//HAL_GPIO_WritePin(SOLENOID_2_GPIO_Port, SOLENOID_2_Pin, RESET); // DONG DUONG XA KHI  // 31-12
						//HAL_GPIO_WritePin(SOLENOID_1_GPIO_Port, SOLENOID_1_Pin, RESET); // DONG KHI CB VACUUM  // 31-12
					}
					break;
				}
				case LOW_PUSH:
				{

					pushdown_LowPressure_Enable();
					uart_send_Feedback_Status_Run(counter_state, PUSHDOWN_FNC); // update function progress for software ui
					DWT_Delay_ms(global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data); // waitting time
					pushdown_LowPressure_Disable();
					//HAL_GPIO_WritePin(SOLENOID_1_GPIO_Port, SOLENOID_1_Pin, RESET); // thoi khi len  // 31-12
					global_variable.UART_Command.u8_Data_Tx_Buffer[2] = global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Byte[0];
					global_variable.UART_Command.u8_Data_Tx_Buffer[3] = global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Byte[1];
#ifdef SYNO24_PNA
					global_variable.UART_Command.u8_Data_Tx_Buffer[2] = global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Byte[0];
					global_variable.UART_Command.u8_Data_Tx_Buffer[3] = global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Byte[1];
					// SEND FOR START CLOCKDOWN COUNTER
					uart_send_Feedback_Status_Run(1);
					// DELAY for pressure stable

					PNA_wait_and_process_sync(global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Data);
#else
					uart_send_Feedback_Status_Run(counter_state, WAIT_FNC); // update function progress for software ui 2 la wait
					DNA_wait_time(global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Data); // waitting time
#endif
					//DWT_Delay_ms(global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Data); // waitting time
					break;
				}
				default:
				{
					break;
				}
				}
			}
		}
	}
	else
	{
		// gianglh can day ep cho nay 03-09-2025
		if(need_progress == true)
		{

#ifdef SYNO24_PNA
			global_variable.UART_Command.u8_Data_Tx_Buffer[2] = global_variable.synthetic_oligo.fill_chemical.u16tb_wait_after_fill.Byte[0];
			global_variable.UART_Command.u8_Data_Tx_Buffer[3] = global_variable.synthetic_oligo.fill_chemical.u16tb_wait_after_fill.Byte[1];
			uart_send_Feedback_Status_Run(1);
			PNA_wait_and_process_sync(global_variable.synthetic_oligo.fill_chemical.u16tb_wait_after_fill.Data);
#else
			//HAL_TIM_Base_Start_IT(&htim14); // mo lai ngat de update thong tin len software
			// Doi sau khi bom hoa chat

			uart_send_Feedback_Status_Run(0, WAIT_AFTERFILL); // update function  = 0 count = 0 la state bom va doi
			// open FAN IN BOX 13-08-2025

			if(global_variable.synthetic_oligo.fill_chemical.u16tb_wait_after_fill.Data > 0)
			{
				Stepper_move_Coordinates_XY( X_POSITION_PUSH_DOWN, Y_POSITION_PUSH_DOWN);
				Stepper_Z1_move((Z_POSITION_PUSH_DOWN -10));
				HAL_Delay(100);
				DWT_Delay_ms(global_variable.synthetic_oligo.fill_chemical.u16tb_wait_after_fill.Data);
			}
			else
			{

			}
			FeatureVacuumBox();
#endif
			for(uint8_t counter_state = 0; counter_state < 10; counter_state++)
			{
				// kiem tra va dieu khien do am
				Syno24_get_and_auto_control_Humidity();
				switch(global_variable.synthetic_oligo.control_pressure.u8_option_pressure[counter_state])
				{
				case HIGH_VACUUM:
				{
					if((global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data != 0) &&
							global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data != 0)
					{

					}
					break;
				}
				case LOW_VACUUM:
				{
					if((global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data != 0) &&
							global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data != 0)
					{

					}
					break;
				}
				case HIGH_PUSH:
				{
					if((global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data != 0) &&
							global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data != 0)
					{

						pushdown_HighPressure_Enable();
						uart_send_Feedback_Status_Run(counter_state, PUSHDOWN_FNC); // update function progress for software ui
						DWT_Delay_ms(global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data); // process time
						pushdown_HighPressure_Disable();
						//pushdown_compression_disable();
#ifdef SYNO24_PNA
						global_variable.UART_Command.u8_Data_Tx_Buffer[2] = global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Byte[0];
						global_variable.UART_Command.u8_Data_Tx_Buffer[3] = global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Byte[1];
						// SEND FOR START CLOCKDOWN COUNTER
						uart_send_Feedback_Status_Run(1);
						// DELAY for pressure stable
						PNA_wait_and_process_sync(global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Data);
#else
						uart_send_Feedback_Status_Run(counter_state, WAIT_FNC); // update function progress for software ui 2 la wait
						DNA_wait_time(global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Data); // waitting time
#endif
					}
					break;
				}
				case LOW_PUSH:
				{
					pushdown_LowPressure_Enable();
					uart_send_Feedback_Status_Run(counter_state, PUSHDOWN_FNC); // update function progress for software ui
					DWT_Delay_ms(global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[counter_state].Data); // process time
					pushdown_LowPressure_Disable();
					global_variable.UART_Command.u8_Data_Tx_Buffer[2] = global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Byte[0];
					global_variable.UART_Command.u8_Data_Tx_Buffer[3] = global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Byte[1];
#ifdef SYNO24_PNA
					global_variable.UART_Command.u8_Data_Tx_Buffer[2] = global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Byte[0];
					global_variable.UART_Command.u8_Data_Tx_Buffer[3] = global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Byte[1];
					// SEND FOR START CLOCKDOWN COUNTER
					uart_send_Feedback_Status_Run(1);
					// DELAY for pressure stable

					PNA_wait_and_process_sync(global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Data);
#else
					uart_send_Feedback_Status_Run(counter_state, WAIT_FNC); // update function progress for software ui
					DNA_wait_time(global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Data); // waitting time
					//DWT_Delay_ms(global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[counter_state].Data); // waitting time
#endif
					break;
				}
				default:
				{
					break;
				}
				}
			}
		}
	}

	DisableVacuumBox(); // tat quat hut trong hop dam bao tat khi het step
}

//======================================= MANUAL RUN START ===========================================================================================

void MANUAL_RUN()
{
	Stepper_Z1_move(Z_POSITION_NORMAL);
	static uint8_t column_x;
	static uint8_t row_y;
	static uint8_t u8_position_y = 0;
	column_x = 0;
	row_y = 0;
	u8_position_y = 0;
	if(global_variable.manual_run.U8_TASK_CONTROL == 1)
	{
		for(column_x = 0; column_x < 3; column_x++ ) // chay 3 cot
		{
			if(column_x % 2 == 0)
			{
				for(row_y = 0; row_y < 8; row_y++ ) // chay 8 hang
				{
					if(global_variable.manual_run.u8_checked_well[column_x * 8 + row_y] == 1) // kiem tra xem da ket thuc chu trinh bom hoa chat chua
					{
						Chemical_fill_process(&global_variable, global_variable.manual_run.u8_typeof_chemical,
								global_variable.manual_run.u16_volume.Data, column_x, row_y);
						DWT_Delay_ms(10);
					}
				}
			} // end if column_x % 2 == 0
			else
			{
				for(row_y = 0; row_y < 8; row_y++ ) // chay 8 hang
				{
					if(global_variable.manual_run.u8_checked_well[column_x * 8 +  7 - row_y] == 1) // kiem tra xem da ket thuc chu trinh bom hoa chat chua
					{
						u8_position_y = 7 - row_y;
						Chemical_fill_process(&global_variable, global_variable.manual_run.u8_typeof_chemical,
								global_variable.manual_run.u16_volume.Data, column_x, u8_position_y);
						DWT_Delay_ms(10);
					}
				} /// END FOR row_y
			} // end else column_x % 2 == 0
		} // end for column_x = 0; chay 3 cot
	} // end 	if(global_variable.manual_run.U8_TASK_CONTROL == 1)
	else
	{
		for(uint8_t u8_counter_state = 0 ; u8_counter_state < 4; u8_counter_state++)
		{
			switch(global_variable.manual_run.u8_option_pressure[u8_counter_state])
			{
			case HIGH_VACUUM:
			{
				break;
			}
			case LOW_VACUUM:
			{
				if((global_variable.manual_run.u16tb_procs_time[u8_counter_state].Data != 0) &&
						global_variable.manual_run.u16tb_waitting_after_time[u8_counter_state].Data != 0)
				{

				}
				break;
			}
			case HIGH_PUSH:
			{
				if((global_variable.manual_run.u16tb_procs_time[u8_counter_state].Data != 0) &&
						global_variable.manual_run.u16tb_waitting_after_time[u8_counter_state].Data != 0)
				{
					//==================================================================================
					pushdown_HighPressure_Enable();
					DWT_Delay_ms(global_variable.manual_run.u16tb_procs_time[u8_counter_state].Data); // process time
					pushdown_HighPressure_Disable();
#ifdef SYNO24_PNA
					global_variable.UART_Command.u8_Data_Tx_Buffer[2] = global_variable.manual_run.u16tb_waitting_after_time[u8_counter_state].Byte[0];
					global_variable.UART_Command.u8_Data_Tx_Buffer[3] = global_variable.manual_run.u16tb_waitting_after_time[u8_counter_state].Byte[1];
					uart_send_Feedback_Status_Run(1);
					// DELAY for pressure stable
					PNA_wait_and_process_sync(global_variable.manual_run.u16tb_waitting_after_time[u8_counter_state].Data);
#else
					DWT_Delay_ms(global_variable.manual_run.u16tb_waitting_after_time[u8_counter_state].Data); // waitting time
#endif
				}
				break;
			}
			case LOW_PUSH:
			{

				if((global_variable.manual_run.u16tb_procs_time[u8_counter_state].Data != 0) &&
						global_variable.manual_run.u16tb_waitting_after_time[u8_counter_state].Data != 0)
				{
					pushdown_LowPressure_Enable();
					DWT_Delay_ms(global_variable.manual_run.u16tb_procs_time[u8_counter_state].Data); // waitting time
					pushdown_LowPressure_Disable();
					DWT_Delay_ms(global_variable.manual_run.u16tb_waitting_after_time[u8_counter_state].Data); // waitting time
				}
				break;
			}
			default:
			{
				break;
			}
			} // end switch
		}// end for uint8_t u8_counter_state = 0, chay 4 lan day khi xuong
	} // end else
	Stepper_Z1_move(Z_POSITION_NORMAL);
}

//==================================================================================================================
void pushdown_LowPressure_Enable()
{
	Stepper_move_Coordinates_XY( X_POSITION_PUSH_DOWN, Y_POSITION_PUSH_DOWN);
	Stepper_Z1_move(Z_POSITION_PUSH_DOWN);
	HAL_Delay(100);
	HAL_GPIO_WritePin(SOLENOID[LOW_PUSH_SV].Port, SOLENOID[LOW_PUSH_SV].Pin, SET);
}
//===================================================================================================================
void pushdown_LowPressure_Disable()
{
	HAL_GPIO_WritePin(SOLENOID[LOW_PUSH_SV].Port, SOLENOID[LOW_PUSH_SV].Pin, RESET);
}
//====================================================================================================================
void pushdown_HighPressure_Enable()
{
	Stepper_move_Coordinates_XY( X_POSITION_PUSH_DOWN, Y_POSITION_PUSH_DOWN);
	Stepper_Z1_move(Z_POSITION_PUSH_DOWN);
	HAL_Delay(100);
	HAL_GPIO_WritePin(SOLENOID[HIGH_PUSH_SV].Port, SOLENOID[HIGH_PUSH_SV].Pin, SET);
}
//====================================================================================================================
void pushdown_HighPressure_Disable()
{
	HAL_GPIO_WritePin(SOLENOID[HIGH_PUSH_SV].Port, SOLENOID[HIGH_PUSH_SV].Pin, RESET);
}


void Syno24_get_and_auto_control_Humidity()
{
	ADS1115_Read_A0(&f_Volt_H);// a0 la do am
	ADS1115_Read_A1(&f_Volt_T); // a1 nhiet do
	global_variable.status_and_sensor.f_temperature = mapFloat(f_Volt_T, 0, 5, -40, 80);
	global_variable.status_and_sensor.f_humidity = mapFloat(f_Volt_H, 0, 5, 0 , 100);
	global_variable.status_and_sensor.u16_temperature.Data = global_variable.status_and_sensor.f_temperature * 100;
	global_variable.status_and_sensor.u16_humidity.Data = global_variable.status_and_sensor.f_humidity * 100;
	syno24_Control_Air_Humidity();
}

void syno24_Control_Air_Humidity()
{
	if(     global_variable.signal_running.b_signal_runing_oligo == true
			&& global_variable.status_and_sensor.flag_enable_auto_control_air_Nito == SET
	)
	{
		if(global_variable.status_and_sensor.f_humidity >= global_variable.status_and_sensor.u8_high_limit_humidity)
		{
			//HAL_GPIO_WritePin(CTRL6_OPEN_NITO_GPIO_Port, CTRL6_OPEN_NITO_Pin, SET);
			HAL_GPIO_WritePin(SOLENOID[OPEN_NITOR_SV].Port, SOLENOID[OPEN_NITOR_SV].Pin, SET);
		}
		else
		{
			if(global_variable.status_and_sensor.f_humidity <= global_variable.status_and_sensor.u8_low_limit_humidity)
			{
				///HAL_GPIO_WritePin(CTRL6_OPEN_NITO_GPIO_Port, CTRL6_OPEN_NITO_Pin, RESET);
				HAL_GPIO_WritePin(SOLENOID[OPEN_NITOR_SV].Port, SOLENOID[OPEN_NITOR_SV].Pin, RESET);
			}
		}
	}
	else
	{
		//HAL_GPIO_WritePin(CTRL6_OPEN_NITO_GPIO_Port, CTRL6_OPEN_NITO_Pin, RESET);
		HAL_GPIO_WritePin(SOLENOID[OPEN_NITOR_SV].Port, SOLENOID[OPEN_NITOR_SV].Pin, RESET);
	}
}
/*
 * *************************************************************************************************************
 * GIANGLH FUNCTION uart_send_Feedback_Status_Run GiangLH22 update 05-05-2025
 * *************************************************************************************************************
 */
void uart_send_Feedback_Status_Run(uint8_t u8_function_count, uint8_t u8_subfunction_run)
{
	global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_FEEDBACK_STATUS_RUN;
	global_variable.UART_Command.u8_Data_Tx_Buffer[1] =	u8_function_count; //
	global_variable.UART_Command.u8_Data_Tx_Buffer[2] =	u8_subfunction_run; //
	if(u8_subfunction_run == 0)
	{
		global_variable.UART_Command.u8_Data_Tx_Buffer[3] = 0;
		global_variable.UART_Command.u8_Data_Tx_Buffer[4] = 0;
		global_variable.UART_Command.u8_Data_Tx_Buffer[5] = (global_variable.synthetic_oligo.fill_chemical.u16tb_wait_after_fill.Data) & 0xFF;
		global_variable.UART_Command.u8_Data_Tx_Buffer[6] = (global_variable.synthetic_oligo.fill_chemical.u16tb_wait_after_fill.Data >> 8)&0xFF;
	}
	else
	{
		global_variable.UART_Command.u8_Data_Tx_Buffer[3] = (global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[u8_function_count].Data) & 0xFF;
		global_variable.UART_Command.u8_Data_Tx_Buffer[4] = (global_variable.synthetic_oligo.control_pressure.u16tb_procs_time[u8_function_count].Data >> 8)&0xFF;
		global_variable.UART_Command.u8_Data_Tx_Buffer[5] = (global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[u8_function_count].Data) & 0xFF;
		global_variable.UART_Command.u8_Data_Tx_Buffer[6] = (global_variable.synthetic_oligo.control_pressure.u16tb_waitting_after_time[u8_function_count].Data >> 8)&0xFF;
	}

	HAL_UART_Transmit_DMA(&huart1, &global_variable.UART_Command.u8_Data_Tx_Buffer[0], UART_LENGTH_COMMAND_TX);
}

void send_status_machine()
{
	global_variable.status_and_sensor.u16_temperature.Data = global_variable.status_and_sensor.f_temperature * 100;
	global_variable.status_and_sensor.u16_humidity.Data = global_variable.status_and_sensor.f_humidity * 100;
	global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_FIRMWARE_UPDATE_STATUS_AND_SENSOR;
	global_variable.UART_Command.u8_Data_Tx_Buffer[1] = global_variable.status_and_sensor.u16_temperature.Byte[0];
	global_variable.UART_Command.u8_Data_Tx_Buffer[2] = global_variable.status_and_sensor.u16_temperature.Byte[1];
	global_variable.UART_Command.u8_Data_Tx_Buffer[3] = global_variable.status_and_sensor.u16_humidity.Byte[0];
	global_variable.UART_Command.u8_Data_Tx_Buffer[4] = global_variable.status_and_sensor.u16_humidity.Byte[1];
	global_variable.UART_Command.u8_Data_Tx_Buffer[5] = (getPositionX()) & 0xFF;
	global_variable.UART_Command.u8_Data_Tx_Buffer[6] = (getPositionX() >> 8)&0xFF;
	global_variable.UART_Command.u8_Data_Tx_Buffer[7] = (getPositionY()) & 0xFF;
	global_variable.UART_Command.u8_Data_Tx_Buffer[8] = (getPositionY() >> 8)&0xFF;
	global_variable.UART_Command.u8_Data_Tx_Buffer[9] = (getPositionZ1()) & 0xFF;
	global_variable.UART_Command.u8_Data_Tx_Buffer[10] = (getPositionZ1() >> 8)&0xFF;
	UART_Send_Command_SW();
}

/*
 *
 *
 */
void updateRunningState()
{
	global_variable.UART_Command.u8_Data_Tx_Buffer[0] = CMD_FIRMWARE_UPDATE_RUNNING_STATE;
	// update stepname
	global_variable.UART_Command.u8_Data_Tx_Buffer[1] = global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical;
	// update working state
	global_variable.UART_Command.u8_Data_Tx_Buffer[2] = 1;
	// update state solenoid air
}

/*
 *
 *
 *
 */
void Get_sensor()
{
	ADS1115_Read_A0(&f_Volt_H);
	ADS1115_Read_A1(&f_Volt_T);
	global_variable.status_and_sensor.f_temperature = mapFloat(f_Volt_T, 0, 5, -40, 80);
	global_variable.status_and_sensor.f_humidity = mapFloat(f_Volt_H, 0, 5, 0 , 100);
#ifdef UART_DEBUG
	printf("debug Temperature %f \r\n", Temperature);
	printf("debug Humidity %f \r\n", Humidity);
#endif
}
/**
 *
 */
void PNA_wait_and_process_sync(uint16_t u16_intervaltime_process)
{
	HAL_Delay(u16_intervaltime_process * 1000);
}
/***
 *	process_pressure_state
 *
 */
void process_pressure_state(uint8_t option_pressure, uint16_t procs_time, uint16_t waitting_time) {
	switch(option_pressure) {
	case HIGH_VACUUM: {
		// Không làm gì cả
		break;
	}
	case LOW_VACUUM: {
		if((procs_time != 0) && (waitting_time != 0)) {
			// Thêm xử lý cho LOW_VACUUM tại đây nếu cần
		}
		break;
	}
	case HIGH_PUSH: {
		if((procs_time != 0) && (waitting_time != 0)) {
			pushdown_HighPressure_Enable();
			DWT_Delay_ms(procs_time);
			pushdown_HighPressure_Disable();
			DWT_Delay_ms(waitting_time);
		}
		break;
	}
	case LOW_PUSH: {
		if((procs_time != 0) && (waitting_time != 0)) {
			pushdown_LowPressure_Enable();
			DWT_Delay_ms(procs_time);
			pushdown_LowPressure_Disable();
			DWT_Delay_ms(waitting_time);
		}
		break;
	}
	default: {
		// Xử lý lỗi hoặc trư�?ng hợp mặc định
		break;
	}
	} // end switch
}

/**
 * process_pressure_state_ptr(uint8_t *option_pressure, uint16_t *procs_time, uint16_t *waitting_time)
 * for(uint8_t i = 0; i < num_states; i++) {
    process_pressure_state(&option_pressure[i], &procs_time[i], &waitting_time[i]);
  }
 *
 */
void process_pressure_state_ptr(uint8_t *option_pressure, uint16_t *procs_time, uint16_t *waitting_time) {
	switch(*option_pressure) {
	case HIGH_VACUUM: {
		// Không làm gì cả
		break;
	}
	case LOW_VACUUM: {
		if((*procs_time != 0) && (*waitting_time != 0)) {
			// Thêm xử lý cho LOW_VACUUM tại đây nếu cần
		}
		break;
	}
	case HIGH_PUSH: {
		if((*procs_time != 0) && (*waitting_time != 0)) {
			pushdown_HighPressure_Enable();
			DWT_Delay_ms(*procs_time);
			pushdown_HighPressure_Disable();
#ifdef SYNO24_PNA
			global_variable.UART_Command.u8_Data_Tx_Buffer[2] = (*waitting_time).Byte[0]; // Cần kiểm tra lại cách truy cập Byte[0]
			global_variable.UART_Command.u8_Data_Tx_Buffer[3] = (*waitting_time).Byte[1]; // Cần kiểm tra lại cách truy cập Byte[1]
			//uart_send_Feedback_Status_Run
			PNA_wait_and_process_sync(*waitting_time);
#else
			DWT_Delay_ms(*waitting_time);
#endif
		}
		break;
	}
	case LOW_PUSH: {
		if((*procs_time != 0) && (*waitting_time != 0)) {
			pushdown_LowPressure_Enable();
			DWT_Delay_ms(*procs_time);
			pushdown_LowPressure_Disable();
			DWT_Delay_ms(*waitting_time);
		}
		break;
	}
	default: {
		// Xử lý lỗi hoặc trư�?ng hợp mặc định
		break;
	}
	} // end switch
}

/**
 * 	pressure_control
 *  // Goi hàm pressure_control với số lượng trạng thái động
 * 	pressure_control(option_pressure, procs_time, waitting_time, num_states);
 */
void pressure_control(uint8_t u8_option_pressure[], uint16_t u16tb_procs_time[], uint16_t u16tb_waitting_after_time[], uint8_t num_states) {
	for(uint8_t u8_counter_state = 0; u8_counter_state < num_states; u8_counter_state++) {
		process_pressure_state(u8_option_pressure[u8_counter_state],
				u16tb_procs_time[u8_counter_state],
				u16tb_waitting_after_time[u8_counter_state]);
	}
}

void autoPrimming_beforeCoupling()
{
	if(global_variable.advanced_setting.flag_auto_primming_chemical == true)
	{
		// primming ammidite
		// can dua len software
		const uint16_t offsetX = 990;
		uint16_t X_Pos = 0;
		uint16_t u16_time_fill = 100;
		X_Pos = Fill_Position_X[0][0] + offsetX;
		Stepper_move_Coordinates_XY(X_Pos, Y_CALIB_POS);
		Stepper_Z1_move(Z_POSITION_PRIMMING);
		// primming amidite A T G C
		X_Pos = Fill_Position_X[0][0] + offsetX;
		Stepper_move_Coordinates_XY(X_Pos, Y_CALIB_POS);
		Stepper_Z1_move(Z_POSITION_PRIMMING);
		for(uint8_t i = A; i<= a; i++)
		{
			openValve(i);
		}
		u16_time_fill =  valve_calculator_timefill(&global_variable, A , global_variable.advanced_setting.u16tb_autoPrim_volume_amidite.Data);
		while (u16_time_fill > 0)
		{
			u16_time_fill--;
			HAL_Delay(1);
		}
		//DWT_Delay_ms();
		Valve_DisAll();
		// t c g U I
		X_Pos = Fill_Position_X[5][0] + offsetX;
		Stepper_move_Coordinates_XY(X_Pos, Y_CALIB_POS);
		Stepper_Z1_move(Z_POSITION_PRIMMING);
		for(uint8_t i = t; i<= U; i++)
		{
			openValve(i);
		}
		u16_time_fill = valve_calculator_timefill(&global_variable, t , global_variable.advanced_setting.u16tb_autoPrim_volume_amidite.Data);
		while (u16_time_fill > 0)
		{
			u16_time_fill--;
			HAL_Delay(1);
		}
		Valve_DisAll();
		DWT_Delay_ms(u16_time_fill);
		Valve_DisAll();
	}
}


/*
 * Gianglh 11-08-25 tinh nang VacuumBox
 *
 */
void EnableVacuumBox(uint16_t timeOpen)
{
	// 09-06-2025 quan ly dong mo bang thu vien moi viet
	// gianglh22
	GPIO_Timer_turnOn(&FanInBox, timeOpen * 1000);

}
void DisableVacuumBox()
{
	// 09-06-2025 quan ly dong mo bang thu vien moi viet
	// gianglh22
	GPIO_Timer_turnOff(&FanInBox);

}

void UpdateVacuumBox()
{
	GPIO_Timer_update(&FanInBox);
}
//=====================================================================================================================
void FeatureVacuumBox()
{
	if(global_variable.advanced_setting.VacuumBox.Enablefeature)
	{
		switch(global_variable.synthetic_oligo.fill_chemical.u8_first_type_chemical)
		{
		case WASH_ACN_DCM:
		{
			if(global_variable.advanced_setting.VacuumBox.En_WASH)
			{
				EnableVacuumBox(global_variable.advanced_setting.VacuumBox.time.Data);
			}
			break;
		}
		case TCA_in_DCM:
		{
			if(global_variable.advanced_setting.VacuumBox.En_Deblock)
			{
				EnableVacuumBox(global_variable.advanced_setting.VacuumBox.time.Data);
			}
			break;
		}
		case COUPLING:
		{
			if(global_variable.advanced_setting.VacuumBox.En_Coupling)
			{
				EnableVacuumBox(global_variable.advanced_setting.VacuumBox.time.Data);
			}
			break;
		}
		case CAPPING:
		{
			if(global_variable.advanced_setting.VacuumBox.En_Cap)
			{
				EnableVacuumBox(global_variable.advanced_setting.VacuumBox.time.Data);
			}
			break;
		}
		case OXIDATION_IODINE:
		{
			if(global_variable.advanced_setting.VacuumBox.En_Ox)
			{
				EnableVacuumBox(global_variable.advanced_setting.VacuumBox.time.Data);
			}
			break;
		}
		default :
			break;
		}
	}
}
/*
 * void DNA wait time
 */
void DNA_wait_time(uint16_t u16_intervaltime_process)
{
	static uint16_t counter;
	// 14-04-25 thay doi tu counter = u16_intervaltime_process / 100 thanh counter = u16_intervaltime_process / 10
	// vi tren software da chia cho 10 nen chung ta x10
	counter = u16_intervaltime_process;// KHONG CAN CHIA NUA 16-04-25 tren ui đã / 100 rồi
	for(uint16_t time = 0; time < counter; time++)
	{
		HAL_Delay(100);
		UpdateVacuumBox();// tu dong tat Fan in Box --  tinh nang advanced setting mo Fan Hut khi ben trong
	}
	HAL_Delay(u16_intervaltime_process % 10);
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
