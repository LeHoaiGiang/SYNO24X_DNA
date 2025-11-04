/*
 * function.h
 *
 *  Created on: Dec 3, 2022
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

#ifndef INC_FUNCTION_H_
#define INC_FUNCTION_H_
#include "main.h"
#include "struct.h"
//========================================== POSITION SYSTEM
#define Z_POSITION_NORMAL 120
#define Z_POSITION_CLEAN_AIR 120

//#define Z_POSITION_FILL_CHEMICAL 160
#define Z_POSITION_FILL_CHEMICAL 155
#define X_POSITION_PUSH_DOWN  1290
#define Y_POSITION_PUSH_DOWN  1292
#define Z_POSITION_PUSH_DOWN  185

#define	X_PRIMMING_POS_1			1370
#define	X_PRIMMING_POS_2			1370
#define	Y_PRIMMING_POS				1145
#define Z_POSITION_PRIMMING 		110


#define	X_CALIB_POS_1				((uint16_t)1280)
#define	Y_CALIB_POS					((uint16_t)1145)
#define Z_POSITION_CALIB 			110
void Chemical_fill_process(Global_var_t* p_global_variable, uint8_t type_sulphite, uint16_t u16_volume, uint8_t u8_pos_X,  uint8_t u8_pos_Y);
void Valve_EnaAll();
void Valve_DisAll();
void Amidite_process(Global_var_t* p_global_variable, uint8_t u8_idx);
void Coupling2FillBufferProcess(Global_var_t* p_global_variable);
uint16_t valve_calculator_timefill(Global_var_t* global_variable, uint8_t type_sulphite, uint16_t u16_Volume);
uint16_t calculator_volume_avg(Global_var_t* global_variable, uint8_t u8_mixed_base_code, uint16_t u16_volumme_sum);
void calibVolumeProcess(uint8_t type_sulphite, uint16_t timming, uint16_t u8_pos_X,  uint16_t u8_pos_Y);
void Calib_volume(uint8_t type_sulphite, uint16_t u16_time_fill, uint16_t u8_pos_X,  uint16_t u8_pos_Y);
void chemical_fill(uint8_t type, uint32_t u32_time_fill);
void openValve(uint8_t type);
void buzzer_blink();
float mapFloat(float value, float fromLow, float fromHigh, float toLow, float toHigh);
#endif /* INC_FUNCTION_H_ */
