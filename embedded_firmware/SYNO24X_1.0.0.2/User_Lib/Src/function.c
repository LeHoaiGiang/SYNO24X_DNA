/*
 * function.c
 *
 *  Created on: Dec 3, 2022
 *      Author: LeHoaiGiang
 */
#include "function.h"
#include "dwt_stm32_delay.h"
#include "struct.h"
#include "stepper.h"
#include "macro.h"
//typedef enum
//{
//	WAIT_AFTERFILL = 0,
//	VACUUM_1 = 1,
//	VACUUM_2 = 2,
//	POSITIVE_1 = 3,
//	VACUUM_3 = 4,
//	POSITIVE_2 = 5,
//	VACUUM_4 = 6,
//	FAST_VACUUM = 7,
//	END_PROCESS = 99,
//}PROCESS_SYNTHETIC;

const GPIO_Config VALVE_[17] =
{
		/** AMIDITE **/
		//		{VALVE_1_GPIO_Port, VALVE_1_Pin},//A
		//		{VALVE_2_GPIO_Port, VALVE_2_Pin},//T
		//		{VALVE_3_GPIO_Port, VALVE_3_Pin},//G
		//		{VALVE_4_GPIO_Port, VALVE_4_Pin},//C
		//		{VALVE_5_GPIO_Port, VALVE_5_Pin}, //a
		//		{VALVE_6_GPIO_Port, VALVE_6_Pin}, //t
		//		{VALVE_7_GPIO_Port, VALVE_7_Pin}, //g
		//		{VALVE_8_GPIO_Port, VALVE_8_Pin}, //c
		//		{VALVE_9_GPIO_Port, VALVE_9_Pin}, // U FLOATTING 1
		//		{VALVE_10_GPIO_Port, VALVE_10_Pin},// I FLOATTING 2
		//		{VALVE_11_GPIO_Port, VALVE_11_Pin}, // ACTIVATOR
		//		{VALVE_12_GPIO_Port, VALVE_12_Pin}, // DEBLOCK
		//		{SOLENOID_1_GPIO_Port, SOLENOID_1_Pin},//WASH
		//		{SOLENOID_2_GPIO_Port, SOLENOID_2_Pin},//OX1
		//		{SOLENOID_3_GPIO_Port, SOLENOID_3_Pin},//OX2
		//		{SOLENOID_4_GPIO_Port, SOLENOID_4_Pin},//CAP B
		//		{VALVE_3_MAP_CTR5_GPIO_Port, VALVE_3_MAP_CTR5_Pin},//CAP A
		{V1_GPIO_Port, V1_Pin}, // A
		{V2_GPIO_Port, V2_Pin}, // T
		{V3_GPIO_Port, V3_Pin}, // G
		{V4_GPIO_Port, V4_Pin}, // C
		{V5_GPIO_Port, V5_Pin}, // a
		{V6_GPIO_Port, V6_Pin}, // t
		{V7_GPIO_Port, V7_Pin}, // g
		{V8_GPIO_Port, V8_Pin}, // c
		{V9_GPIO_Port, V9_Pin}, // U FLOATTING 1
		{V10_GPIO_Port, V10_Pin}, // I FLOATTING 2
		{V11_GPIO_Port, V11_Pin}, // ACTIVATOR
		{V12_GPIO_Port, V12_Pin}, // DEBLOCK
		{V13_GPIO_Port, V13_Pin}, // WASH
		{V14_GPIO_Port, V14_Pin}, // OX1
		{V15_GPIO_Port, V15_Pin}, // OX2
		{V16_GPIO_Port, V16_Pin}, // CAP B
		{V17_GPIO_Port, V17_Pin}, // CAP A
};
#ifdef USING_BETA
volatile int Fill_Position_X[12][3]=
{
		{27, 18, 9},
		{27, 18, 9},
		{27, 18, 9},
		{27, 18, 9},
		{27, 18, 9},
		{27, 18, 9},


		{36 ,27, 18},
		{36 ,27, 18},
		{36 ,27, 18},
		{36 ,27, 18},
		{36 ,27, 18},
		{36 ,27, 18},
};
volatile int Fill_Position_Y[12][8]=
{
		{45, 54, 63, 72, 81, 90, 99, 108},//0
		{54, 63, 72, 81, 90, 99, 108, 117},//1
		{63, 72, 81, 90, 99, 108, 117, 126},//2
		{72, 81, 90, 99, 108, 117, 126, 135},//3
		{81, 90, 99, 108, 117, 126, 135, 144},//4
		{90, 99, 108, 117, 126, 135, 144, 153},//5
		//=========================================
		{45, 54, 63, 72, 81, 90, 99, 108},//0
		{54, 63, 72, 81, 90, 99, 108, 117},//1
		{63, 72, 81, 90, 99, 108, 117, 126},//2
		{72, 81, 90, 99, 108, 117, 126, 135},//3
		{81, 90, 99, 108, 117, 126, 135, 144},//4
		{90, 99, 108, 117, 126, 135, 144, 153},//5
};

#endif
#ifdef VERSION_1
const int Fill_Position_X[12][3]=
{
		{191,200, 209},
		{191,200, 209},
		{191,200, 209},
		{191,200, 209},
		{191,200, 209},
		{191,200, 209},

		{182 ,191,200},
		{182 ,191,200},
		{182 ,191,200},
		{182 ,191,200},
		{182 ,191,200},
		{182 ,191,200}

};
const int Fill_Position_Y[12][8]=
{
		// ======= neu muon next valve thi tang toa do Y len 9
		{0, 0, 7, 16, 25, 34, 43, 52},//0
		{0, 7, 16, 25, 34, 43, 52, 61},//1
		{7, 16, 25, 34, 43, 52, 61, 70},//2
		{16, 25, 34, 43, 52, 61, 70, 79},//3
		{25, 34, 43, 52, 61, 70, 79, 88},//4
		{ 34, 43, 52, 61, 70, 79, 88, 97},//5
		//=======================================================
		{0, 0, 7, 16, 25, 34, 43, 52},//0
		{0, 7, 16, 25, 34, 43, 52, 61},//1
		{7, 16, 25, 34, 43, 52, 61, 70},//2
		{16, 25, 34, 43, 52, 61, 70, 79},//3
		{25, 34, 43, 52, 61, 70, 79, 88},//4
		{ 34, 43, 52, 61, 70, 79, 88, 97},//5
};
#endif

#ifdef USING_SYNO96
volatile int Fill_Position_X[17][3]=
{
		{380, 290, 200},// 1
		{380, 290, 200},// 2
		{380, 290, 200},// 3
		{380, 290, 200},// 4
		{380, 290, 200},// 5
		// X giữ nguyên cho valve cùng cột

		{470, 380, 290}, // 6
		{470, 380, 290}, // 7
		{470, 380, 290}, // 8
		{470, 380, 290}, // 9
		{470, 380, 290}, // 10

		{555, 465, 375},//11
		{555, 465, 375},//12
		{555, 465, 375},//13
		{555, 465, 375},//14
		{555, 465, 375},//15
		//		{560, 470, 380},//11
		//		{560, 470, 380},//12
		//		{560, 470, 380},//13
		//		{560, 470, 380},//14
		//		{560, 470, 380},//15

		{650 , 560, 470},
		{650 , 560, 470},
};
volatile int Fill_Position_Y[17][8]=
{
		// y sẽ tăng 9 đơn vị theo hàng
		// mỗi cột lệch thêm 9 đơn vị
		//1-5 giống 6-10 giống 11-15 // 16-20
		{505, 595, 685, 775, 865, 955, 1045, 1135},//0
		{595, 685, 775, 865, 955, 1045, 1135, 1225},//1
		{685, 775, 865, 955, 1045, 1135, 1225, 1315},//2
		{775, 865, 955, 1045, 1135, 1225, 1315, 1405},//3
		{ 865, 955, 1045, 1135, 1225, 1315, 1405, 1495},//4
		//=========================================
		{505, 595, 685, 775, 865, 955, 1045, 1135},//0
		{595, 685, 775, 865, 955, 1045, 1135, 1225},//1
		{685, 775, 865, 955, 1045, 1135, 1225, 1315},//2
		{775, 865, 955, 1045, 1135, 1225, 1315, 1405},//3
		{ 865, 955, 1045, 1135, 1225, 1315, 1405, 1495},//4
		//=========================================
		{505, 595, 685, 775, 865, 955, 1045, 1135},//0
		{595, 685, 775, 865, 955, 1045, 1135, 1225},//1
		{685, 775, 865, 955, 1045, 1135, 1225, 1315},//2
		{775, 865, 955, 1045, 1135, 1225, 1315, 1405},//3
		{ 865, 955, 1045, 1135, 1225, 1315, 1405, 1495},//4

		{505, 595, 685, 775, 865, 955, 1045, 1135},//0
		{595, 685, 775, 865, 955, 1045, 1135, 1225},//1

};
#endif
/*
 *
 *  AMD_A = 0, // valve 1
    AMD_T = 1,
    AMD_G = 2,
    AMD_C = 3,
    AMD_I = 4,
    AMD_U = 5,
    AMD_Y = 6,
    AMD_R = 7,
    AMD_W = 8,
    AMD_S = 9,
    AMD_K = 10,
    AMD_M = 11,
    AMD_D = 12,
    AMD_V = 13,
    AMD_N = 14,
 */
const int MIXED_BASE[21][4]=
{
		{AMD_A,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY}, //0
		{AMD_T,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//1
		{AMD_G, CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//2
		{AMD_C, CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//3
		{AMD_a,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY}, //0
		{AMD_t,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//1
		{AMD_g, CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//2
		{AMD_c, CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//3
		{AMD_I, CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//4
		{AMD_U, CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//5

		{AMD_C, AMD_T, CHEMICAL_SUBTANCE_EMPTY, CHEMICAL_SUBTANCE_EMPTY},//6 AMD_Y
		{AMD_G, AMD_A, CHEMICAL_SUBTANCE_EMPTY, CHEMICAL_SUBTANCE_EMPTY},//7 AMD_R
		{AMD_A, AMD_T, CHEMICAL_SUBTANCE_EMPTY, CHEMICAL_SUBTANCE_EMPTY},//8 AMD_W
		{AMD_G, AMD_C, CHEMICAL_SUBTANCE_EMPTY, CHEMICAL_SUBTANCE_EMPTY},//9 AMD_S
		{AMD_G, AMD_T, CHEMICAL_SUBTANCE_EMPTY, CHEMICAL_SUBTANCE_EMPTY},//10 AMD_K
		{AMD_A, AMD_C, CHEMICAL_SUBTANCE_EMPTY, CHEMICAL_SUBTANCE_EMPTY},//11 AMD_M
		{AMD_G, AMD_A, AMD_T, CHEMICAL_SUBTANCE_EMPTY},//12 AMD_D
		{AMD_G, AMD_A, AMD_C, CHEMICAL_SUBTANCE_EMPTY},//13 AMD_V
		{AMD_G, AMD_A, AMD_C, AMD_T},//14 AMD_N
		{AMD_A, AMD_C, AMD_T, CHEMICAL_SUBTANCE_EMPTY},//15 AMD_H
		{AMD_G, AMD_C, AMD_T, CHEMICAL_SUBTANCE_EMPTY},//16 AMD_B
};
// debug variable
uint8_t idx_debug =0;
//**********************************************************************************************************
/*
 * uint8_t sulphite ---- loai hoa chat 0-10 -- tổng cộng có 11 loại hóa chất
 * u8_pos_X max vi tri x la 3
 * u8_pos_Y max vi tri y la 8
 */
void Chemical_fill_process(Global_var_t* p_global_variable, uint8_t type_sulphite, uint16_t u16_volume, uint8_t u8_pos_X,  uint8_t u8_pos_Y)
{
	static int pos_x;
	static int pos_y;
	uint16_t timefill;
	if(u16_volume != 0)
	{
		pos_x = Fill_Position_X[type_sulphite][u8_pos_X];
		pos_y = Fill_Position_Y[type_sulphite][u8_pos_Y];
		Stepper_move_Coordinates_XY(pos_x, pos_y);
		Stepper_Z1_move(Z_POSITION_FILL_CHEMICAL);
		DWT_Delay_ms(10);

		//Chemical_substance(type_sulphite, u16_volume);
		timefill = valve_calculator_timefill(p_global_variable, type_sulphite, u16_volume);
		chemical_fill(type_sulphite, timefill);
	}
}

void Amidite_process(Global_var_t* p_global_variable, uint8_t u8_idx)
{
	static int position_X;
	static int position_Y;
	static uint8_t chemical;
	uint8_t chemical_reality;
	uint16_t u16_volume;
	uint16_t u16_time_fill;
	//uint8_t u8_counter_formula;
	for(uint8_t column_x = 0; column_x < 3; column_x++ )
	{
		for(uint8_t row_y = 0; row_y < 8; row_y++ )
		{
			chemical = p_global_variable->synthetic_oligo.u8_well_sequence_run[column_x * 8 + row_y];
			// get time fill chemical to well
			if(u8_idx == 0)
			{
				u16_volume = p_global_variable->synthetic_oligo.u16_timefill_func_mix_well_1[column_x * 8 + row_y].Data;
			}
			if(u8_idx == 1)
			{
				u16_volume = p_global_variable->synthetic_oligo.u16_timefill_func_mix_well_2[column_x * 8 + row_y].Data;
			}
			if(u8_idx == 2)
			{
				u16_volume = p_global_variable->synthetic_oligo.u16_timefill_func_mix_well_3[column_x * 8 + row_y].Data;
			}
			calculator_volume_avg(p_global_variable , chemical, u16_volume);
			// process amidite mixed base
			if(chemical != CHEMICAL_SUBTANCE_EMPTY)
			{
				for(uint8_t u8_mix_idx = 0; u8_mix_idx < 4; u8_mix_idx++)
				{
					// chuyen doi sang hoa chat that = truy cap mang du lieu
					chemical_reality = MIXED_BASE[chemical][u8_mix_idx];
					// tinh toan do can di chuyen
					if(chemical_reality != CHEMICAL_SUBTANCE_EMPTY)
					{
						position_X = Fill_Position_X[chemical_reality][column_x];
						position_Y = Fill_Position_Y[chemical_reality][row_y];
						// tinh thoi gian bom
						u16_time_fill = valve_calculator_timefill(p_global_variable, chemical_reality, p_global_variable->mixed_base.u16_volumme_avg);
						Stepper_move_Coordinates_XY(position_X, position_Y);
						Stepper_Z1_move(Z_POSITION_FILL_CHEMICAL);
						DWT_Delay_ms(10);
						chemical_fill(chemical_reality, u16_time_fill);
					}
				}
			} // chemical != CHEMICAL_SUBTANCE_EMPTY

		} // chay well tren 1 hang
	}// chay
}
/*	22-05-2025
 *
 *
 *
 *
 *
 */

void Coupling2FillBufferProcess(Global_var_t* p_global_variable)
{
	static int position_X;
	static int position_Y;
	uint8_t chemical_reality;
	uint16_t u16_time_fill;

	//uint8_t u8_counter_formula;
	for(uint8_t column_x = 0; column_x < 3; column_x++ )
	{
		for(uint8_t row_y = 0; row_y < 8; row_y++ )
		{

			if(p_global_variable->synthetic_oligo.u8_well_sequence_run[column_x * 8 + row_y] == CHEMICAL_SUBTANCE_EMPTY
			&& /*p_global_variable->synthetic_oligo.u8_well_sequence_run[column_x * 8 + row_y] != CHEMICAL_SUBTANCE_EMPTY*/
			p_global_variable->synthetic_oligo.wellFirstsequence[column_x * 8 + row_y] != CHEMICAL_SUBTANCE_EMPTY)
			{
				idx_debug = column_x * 8 + row_y;
				// chuyen doi sang hoa chat that = truy cap mang du lieu
				chemical_reality = p_global_variable->Coupling2Setting.typeReagent;
				// tinh toan do can di chuyen
				if(chemical_reality != CHEMICAL_SUBTANCE_EMPTY)
				{
					position_X = Fill_Position_X[chemical_reality][column_x];
					position_Y = Fill_Position_Y[chemical_reality][row_y];
					// tinh thoi gian bom
					u16_time_fill = valve_calculator_timefill(p_global_variable, chemical_reality, p_global_variable->Coupling2Setting.volume.Data);
					Stepper_move_Coordinates_XY(position_X, position_Y);
					Stepper_Z1_move(Z_POSITION_FILL_CHEMICAL);
					DWT_Delay_ms(10);
					chemical_fill(chemical_reality, u16_time_fill);
				}
			} // chemical != CHEMICAL_SUBTANCE_EMPTY
		} // chay well tren 1 hang
	}// chay for(uint8_t column_x = 0; column_x < 3; column_x++ ) end
}
void Valve_EnaAll()
{
	//	HAL_GPIO_WritePin(VALVE_1_GPIO_Port, VALVE_1_Pin, SET);
	//	HAL_GPIO_WritePin(VALVE_2_GPIO_Port, VALVE_2_Pin, SET);
	//	HAL_GPIO_WritePin(VALVE_3_MAP_CTR5_GPIO_Port, VALVE_3_MAP_CTR5_Pin, SET);
	//	HAL_GPIO_WritePin(VALVE_4_GPIO_Port, VALVE_4_Pin, SET);
	//	HAL_GPIO_WritePin(VALVE_5_GPIO_Port, VALVE_5_Pin, SET);
	//	HAL_GPIO_WritePin(VALVE_6_GPIO_Port, VALVE_6_Pin, SET);
	//	HAL_GPIO_WritePin(VALVE_7_GPIO_Port, VALVE_7_Pin, SET);
	//	HAL_GPIO_WritePin(VALVE_8_GPIO_Port, VALVE_8_Pin, SET);
	//	HAL_GPIO_WritePin(VALVE_9_GPIO_Port, VALVE_9_Pin, SET);
	//	HAL_GPIO_WritePin(VALVE_10_GPIO_Port, VALVE_10_Pin, SET);
	//	HAL_GPIO_WritePin(VALVE_11_GPIO_Port, VALVE_11_Pin, SET);
	//	HAL_GPIO_WritePin(VALVE_12_GPIO_Port, VALVE_12_Pin, SET);
	for(uint8_t i = 0; i < 17 ; i++)
	{
		HAL_GPIO_WritePin(VALVE_[i].Port, VALVE_[i].Pin, SET);
	}
}

void Valve_DisAll()
{
	for(uint8_t i = 0; i < 17 ; i++)
	{
		HAL_GPIO_WritePin(VALVE_[i].Port, VALVE_[i].Pin, RESET);
	}
}

/****
 * valve_calculator_timefill
 * tinh toan the tich can bom cua valve
 */

uint16_t valve_calculator_timefill(Global_var_t* global_variable, uint8_t type_sulphite, uint16_t u16_Volume)
{
	if(u16_Volume <= 0)
	{
		return 0;
	}
	else
	{
		double db_time = u16_Volume * global_variable->valve_setting[type_sulphite].f_a.Data +global_variable->valve_setting[type_sulphite].f_b.Data;
		return (uint16_t)(db_time);
	}
}

uint16_t calculator_volume_avg( Global_var_t* global_variable, uint8_t u8_mixed_base_code, uint16_t u16_volumme_sum)
{
	switch(u8_mixed_base_code)
	{
	case AMD_A:
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum;
		break;
	}
	case AMD_T:
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum;
		break;
	}
	case AMD_G:
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum;
		break;
	}

	case AMD_C:
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum;
		break;
	}

	case AMD_a:
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum;
		break;
	}
	case AMD_t:
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum;
		break;
	}
	case AMD_g:
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum;
		break;
	}

	case AMD_c:
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum;
		break;
	}

	case AMD_I:
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum;
		break;
	}
	case AMD_U:
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum;
		break;
	}
	case AMD_Y: // C & T
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum / 2;
		break;
	}
	case AMD_R:	// G & 	A
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum / 2;
		break;
	}
	case AMD_W: // A & T
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum / 2;
		break;
	}
	case AMD_S: //G & C
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum / 2;
		break;
	}
	case AMD_K: // G & T
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum / 2;
		break;
	}
	case AMD_M: // A & C
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum / 2;
		break;
	}
	case AMD_D: // G & A & T
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum / 3;
		break;
	}
	case AMD_V:// G & A & C
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum / 3;
		break;
	}
	case AMD_N: // G A C T
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum / 4;
		break;
	}
	case AMD_H: // G A C T
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum / 3;
		break;
	}
	case AMD_B: // G A C T
	{
		global_variable->mixed_base.u16_volumme_avg = u16_volumme_sum / 3;
		break;
	}
	}
	if(global_variable->mixed_base.u16_volumme_avg  < 10)
	{
		global_variable->mixed_base.u16_volumme_avg = 10;
	}
}



void calibVolumeProcess(uint8_t type_sulphite, uint16_t u16_time_fill, uint16_t u8_pos_X,  uint16_t u8_pos_Y)
{
	/*
	Activator = 10,
	TCA_in_DCM = 11,
	WASH_ACN_DCM = 12,
	OXIDATION_IODINE = 13,
	OXIDATION_IODINE2 = 14,
	CAPPING_CAPB = 15,
	CAPPING_CAPA = 16,
	COUPLING = 17,
	FUNTION_MIXED = 18,
	CAPPING = 19
	 */
	uint16_t X_Pos = 0;
	const uint16_t offsetX = 900;
	if(u16_time_fill != 0)
	{
		switch(type_sulphite)
		{
		case A:
		case T:
		case G:
		case C:
		case a:
			X_Pos = Fill_Position_X[0][0] + offsetX;
			Calib_volume(type_sulphite, u16_time_fill, X_Pos, Y_CALIB_POS);
			break;
		case t:
		case g:
		case c:
		case I:
		case U:
			X_Pos = Fill_Position_X[5][0] + offsetX;
			Calib_volume(type_sulphite, u16_time_fill, X_Pos, Y_CALIB_POS);
			break;
		case Activator:
		case TCA_in_DCM:
		case WASH_ACN_DCM:
		case OXIDATION_IODINE:
		case OXIDATION_IODINE2:
			X_Pos = Fill_Position_X[10][0] + offsetX;
			Calib_volume(type_sulphite, u16_time_fill, X_Pos, Y_CALIB_POS);
			break;
		case CAPPING_CAPA:
		case CAPPING_CAPB:
			X_Pos = Fill_Position_X[15][0] + offsetX;
			Calib_volume(type_sulphite, u16_time_fill, X_Pos, Y_CALIB_POS);
			break;
		default:
			break;
		}
		//X_Pos = Fill_Position_X[0][0] + offsetX;
		//Stepper_Z1_Run2Normal();
		//Stepper_move_Coordinates_XY(u8_pos_X, u8_pos_Y);
		//Stepper_Z1_move(Z_POSITION_CALIB);
		//HAL_GPIO_WritePin(VALVE_[type_sulphite].Port, VALVE_[type_sulphite].Pin, SET);
		//		while (u16_time_fill > 0)
		//		{
		//			u16_time_fill--;
		//			HAL_Delay(1);
		//		}
		//		HAL_GPIO_WritePin(VALVE_[type_sulphite].Port, VALVE_[type_sulphite].Pin, RESET);
	}
}

void Calib_volume(uint8_t type_sulphite, uint16_t u16_time_fill, uint16_t u8_pos_X,  uint16_t u8_pos_Y)
{

	if(u16_time_fill != 0)
	{
		Stepper_Z1_Run2Normal();
		Stepper_move_Coordinates_XY(u8_pos_X, u8_pos_Y);
		Stepper_Z1_move(Z_POSITION_CALIB);
		HAL_GPIO_WritePin(VALVE_[type_sulphite].Port, VALVE_[type_sulphite].Pin, SET);
		while (u16_time_fill > 0)
		{
			u16_time_fill--;
			HAL_Delay(1);
		}
		HAL_GPIO_WritePin(VALVE_[type_sulphite].Port, VALVE_[type_sulphite].Pin, RESET);
	}
}

void chemical_fill(uint8_t type, uint32_t u32_time_fill)
{
	if (type >= 0 && type < 17) {
		HAL_GPIO_WritePin(VALVE_[type].Port, VALVE_[type].Pin, SET);
		//DWT_Delay_ms(u32_time_fill);
		while (u32_time_fill > 0)
		{
			u32_time_fill--;
			HAL_Delay(1);
		}
		HAL_GPIO_WritePin(VALVE_[type].Port, VALVE_[type].Pin, RESET);
	}

}

void openValve(uint8_t type)
{
	HAL_GPIO_WritePin(VALVE_[type].Port, VALVE_[type].Pin, SET);
}

