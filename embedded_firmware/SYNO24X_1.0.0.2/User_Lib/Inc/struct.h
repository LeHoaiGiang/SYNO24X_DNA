/*
 * struct.h
 *
 *  Created on: Sep 16, 2022
 *      Author: LeHoaiGiang
 */
/* USER CODE BEGIN Includes */
/*
 * Author: LeHoaiGiang
 * email : lehoaigiangg@gmail.com
 * Contact : 0336379944
 * I am SYNO24X machine
 * Project name: SYNO24X (oligonucleotide synthesis system)
 */

#ifndef INC_STRUCT_H_
#define INC_STRUCT_H_
#include "macro.h"
#include "stdbool.h"
typedef struct {
	GPIO_TypeDef *Port;
	uint16_t Pin;
} GPIO_Config;
typedef union
{
	uint16_t  Data;
	uint8_t Byte[2];
}TwoByte_to_u16;
typedef union
{
	uint32_t Data;
	uint8_t Byte[4];
}FourByte_to_u32;
typedef union
{
	float Data;
	char Byte[4];
}float_to_u8_t;
typedef struct
{
	float a;
	float b;
	float_to_u8_t f_a;
	float_to_u8_t f_b;
}Parameter_valve_t;// struct nay chua thong tin thoi gian bom hoa chat sau khi da calib
//=========================================
typedef struct
{
	uint8_t u8_Valve[MAX_NUMBER_VALVE];
	TwoByte_to_u16 u16tb_Time2Fill[MAX_NUMBER_VALVE];
}Valve_control_t;
typedef struct
{
	TwoByte_to_u16 u16tb_X_Distance;
	TwoByte_to_u16 u16tb_Y_Distance;
	TwoByte_to_u16 u16tb_Z1_Distance;
	TwoByte_to_u16 u16tb_Z2_Distance;
	uint8_t u8_Init_FW_Completed;
	uint8_t stateControlValve[MAX_NUMBER_VALVE];
	uint8_t stateControlSolenoid[8];
}Signal_control_t;
/***
 *
 */
typedef struct
{
	volatile uint8_t u8_Data_Rx_Buffer[UART_LENGTH_COMMAND];
	volatile uint8_t u8_Data_Tx_Buffer[UART_LENGTH_COMMAND];
	volatile uint8_t u8_Data_sensor[UART_LENGTH_COMMAND];
	volatile bool b_FW_Rx_Command_Flag; // Flag = 1 have command from uart Flag = 0 Don't Have
}UART_Communication_t;
typedef struct
{
	volatile bool b_signal_update_status; // tin hieu nay true thi gui du lieu ve cho Software
	volatile bool b_signal_connect_software; // da connect voi software thanh cong
	volatile bool b_signal_runing_oligo;
	volatile uint16_t u16_counter_base_finished;
	volatile uint16_t u16_counter_step;
}signal_running_t;
typedef struct
{
	volatile uint16_t u16_counter_2second;
	volatile bool  b_five_minute;
	volatile bool  b_one_minute;
	volatile bool  b_auto_control_NITO;
}control_air_t;
//============================================================= STEP RUN OLIGO ====================================================
typedef struct
{
	char u8_type_chemical[3];
	TwoByte_to_u16 u16tb_Volume[3];
}mix_function_chemical_t;

typedef struct
{
	TwoByte_to_u16 u16tb_procs_time[10];
	TwoByte_to_u16 u16tb_waitting_after_time[10];
	uint8_t u8_option_pressure[10];
}control_pressure_t;
typedef struct
{
	char u8_first_type_chemical;
	mix_function_chemical_t mix_funtion;
	TwoByte_to_u16 u16tb_Volume;
	TwoByte_to_u16 u16tb_wait_after_fill;
}fill_chemical_t;

typedef struct
{
	TwoByte_to_u16 u16_timewell_decoder_1[24];
	TwoByte_to_u16 u16_timewell_decoder_2[24];
	TwoByte_to_u16 u16_timewell_decoder_3[24];
	TwoByte_to_u16 u16_timewell_decoder_4[24];
}amidite_decoder_data_t;

typedef struct
{
	// khai bao state 2 đên state  11
	bool b_douple_coupling_first_base;
	control_pressure_t control_pressure;
	//control_pressure_t control_pressure_Ox2;
	fill_chemical_t fill_chemical;
	uint8_t wellFirstsequence[MAX_WELL_AMIDITE]; //
	uint8_t u8_well_sequence[MAX_WELL_AMIDITE]; //
	uint8_t u8_well_sequence_run[MAX_WELL_AMIDITE];//  05-05-2025 sequence nay la sequence raw gui tư software detect cot empty va cot co duoc bom
	uint8_t u8_ox_sequence[MAX_WELL_AMIDITE];
	TwoByte_to_u16 u16_timefill_func_mix_well_1[MAX_WELL_AMIDITE];
	TwoByte_to_u16 u16_timefill_func_mix_well_2[MAX_WELL_AMIDITE];
	TwoByte_to_u16 u16_timefill_func_mix_well_3[MAX_WELL_AMIDITE];
	uint8_t u8_enable_fill_buffer_N[MAX_WELL_AMIDITE];//  flag cho phép phun hóa chất đầy các cột còn trống
	TwoByte_to_u16 u16tb_volume_N_buffer;
	TwoByte_to_u16 u16_scale_volume;
	TwoByte_to_u16 u16_scale_time;
	bool isSpecialBase;
}Step_process_parameter_t;
//==================================================================
typedef struct
{
	uint8_t u8_valve_sellect; // sellect valve for calibration
	uint8_t valve[MAX_NUMBER_VALVE];
	FourByte_to_u32 u32fb_time_primming_calib;// sellect valve for calibration
	uint8_t u8_time_primming_control;
}primming_control_t;

typedef struct
{
	TwoByte_to_u16  u16tb_humidity_Preset;
	uint8_t u8_high_limit_humidity;
	uint8_t u8_low_limit_humidity;
	TwoByte_to_u16 u16tb_Pressure_sensor;
	TwoByte_to_u16 u16_temperature;
	TwoByte_to_u16 u16_humidity;
	double f_temperature;
	double f_humidity;
	float f_pressure_sensor;
	bool flag_enable_auto_control_air_Nito;
}status_and_sensor_t;
typedef struct
{
	uint8_t u8_checked_well[MAX_WELL_AMIDITE];
	uint8_t u8_typeof_chemical;
	TwoByte_to_u16 u16_volume;
	uint8_t u8_option_pressure[4];
	TwoByte_to_u16 u16tb_procs_time[4];
	TwoByte_to_u16 u16tb_waitting_after_time[4];
	uint8_t U8_TASK_CONTROL;

}manual_run_t;

typedef struct  {
	bool Enablefeature;
	uint8_t En_WASH;
	uint8_t En_Deblock;
	uint8_t En_Coupling;
	uint8_t En_Cap;
	uint8_t En_Ox;
	TwoByte_to_u16 time;
}VacuumBox_t; // tinh nang fill hoa chat vao cac cot bi trong

typedef struct
{
	bool flag_autocheck_pressure; // cờ kiểm tra áp suất
	bool flag_auto_clean_box; // cờ xả hóa chất
	bool flag_auto_primming_chemical;
	bool flag_vacumm_waste;
	bool flag_exhaustFan; // Trạng thái quạt (bật/tắt)
	bool flag_speacial_volume_trityl;
	TwoByte_to_u16 volume_trityl_collection;
	TwoByte_to_u16 u16tb_autoPrim_volume_amidite;
	TwoByte_to_u16 u16tb_autoPrim_volume_Activator;
	TwoByte_to_u16 u16tb_time_auto_clean;
	TwoByte_to_u16 u16tb_timeExhaustFan;
	float f_pressure_setting;
	VacuumBox_t VacuumBox;
}advanced_setting_t;

typedef struct
{
	uint16_t u16_volumme_sum ;
	uint16_t u16_volumme_avg ; // average value
}mixed_base_t;

typedef struct  {
    bool EnableFillWellDone;
    uint8_t typeReagent;
    TwoByte_to_u16 volume;
}Coupling2Setting_t;

typedef struct
{
	UART_Communication_t UART_Command;
	Signal_control_t signal_control;
	Valve_control_t Fill_control;
	control_air_t control_air;
	signal_running_t signal_running;
	Step_process_parameter_t synthetic_oligo;
	primming_control_t primming_control;
	status_and_sensor_t status_and_sensor;
	manual_run_t manual_run;
	advanced_setting_t advanced_setting;
	Parameter_valve_t valve_setting[MAX_NUMBER_VALVE];
    Coupling2Setting_t Coupling2Setting;
	mixed_base_t mixed_base;
}Global_var_t;
//******************************************************** Khai Bao *******************************************************************************
/*
 * Author: LeHoaiGiang
 * email : lehoaigiangg@gmail.com
 * Contact : 0336379944
 * I am SYNO24X machine
 * Project name: SYNO24X (oligonucleotide synthesis system)
 *
 */
#endif /* INC_STRUCT_H_ */
