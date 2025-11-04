/*
 * macro.h
 *
 *  Created on: Sep 26, 2022
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

#ifndef INC_MACRO_H_
#define INC_MACRO_H_
//#define SYNO24_PNA
#ifdef SYNO24_PNA
#define UART_LENGTH_COMMAND 		256
#define UART_LENGTH_DATA_OLIGO 		256
#define UART_LENGTH_COMMAND_TX 		40
#else
#define UART_LENGTH_COMMAND 		512
#define UART_LENGTH_DATA_OLIGO 		512
#define UART_LENGTH_COMMAND_TX 		512
#endif


#define CMD_CONNECT_LINK						0x01
#define CMD_RUNSTEPPER							0x03
#define CMD_RUN2HOME							0x02
#define CMD_PRIMMING							0x04
#define CMD_START_OLIGO_STEP					0x05
#define CMD_DATA_OLIGO							0x06
#define CMD_ASK_VENDOR_ID						0x07
#define CMD_SIGNAL_START_OLIGO      			0x08
#define CMD_SEND_STOPED_OLIGO					0x09
#define CMD_FIRMWARE_END_OLIGO_STEP 			0x0A
#define CMD_FIRMWARE_UPDATE_STATUS_AND_SENSOR	0x0B
#define CMD_CONTROL_AIR_START					0x0C
#define CMD_CONTROL_MIXED_AIR					0x0D  // command ket hop day khi xuong va thoi khi len lam sach vien frit
#define CMD_REQUEST_PAUSE                       0x0F
#define CMD_RECIVED_SETTING						0x10
#define CMD_OX_SENQUENCE						0x11
#define CMD_FIRMWARE_UPDATE_RUNNING_STATE		0x12
#define CMD_FIRMWARE_REQUEST_DATA_AGAIN			0x13

#define CMD_CALIBRATION_VALVE   				0x64
#define CMD_EXHAUSTED_CHEMICAL					0x65
#define CHEMICAL_SUBTANCE_EMPTY    				0x7F
#define CMD_MANUAL_RUN          				0x66
#define CMD_PRESURE_TESTING     				0x19
// synchronize input output control
#define CMD_CONTROL_SYNCHRONIZE_IO				0x23


#define CMD_FEEDBACK_STATUS_RUN					0x67
#define CMD_STOP_SYSTHETIC_OLIGO                0x99
#define CMD_FW_FEEDBACK_STATUS_RUNNING    		0x27



//#define	X_PRIMMING_POS				60
//#define	Y_PRIMMING_POS				65
//===========================================
//#define UART_DEBUG
//===========================================
#define MAX_WELL_AMIDITE			24
#define MAX_NUMBER_VALVE			17
//enum ORDINAL_VALVE
//{
//    A = 0, // valve 1
//    T = 1,
//    G = 2,
//    C = 3,
//    I = 4,
//    Activator = 5,
//    TCA_in_DCM = 6,
//    WASH_ACN_DCM = 7,
//    OXIDATION_IODINE = 8,
//    CAPPING_CAPA = 9,
//    CAPPING_CAPB = 10,
//    N = 11,
//    COUPLING = 12,
//    FUNTION_MIXED = 13,
//	CAPPING = 14
//};
// dung de chua vi tri valve và function cua step se return ra gia tri nay
//
enum ORDINAL_VALVE // VỊ TRÍ ĐỊNH VỊ VALVE
{
    A = 0, // valve 1
    T = 1,
    G = 2,
    C = 3,
    a = 4,
    t = 5,
    g = 6,
    c = 7,
    I = 8,
    U = 9,
    Activator = 10,
    TCA_in_DCM = 11,
    WASH_ACN_DCM = 12,
    OXIDATION_IODINE = 13,
    OXIDATION_IODINE2 = 14,
    CAPPING_CAPB = 15,
    CAPPING_CAPA = 16,

    COUPLING = 17,
    FUNTION_MIXED = 18,
    CAPPING = 19,
    COUPLING2 = 20
};

enum COUPLING_FNC
{
	CLG_A_ = 0, // valve 1
	CLG_T_ = 1,
	CLG_G_ = 2,
	CLG_X_ = 3,
	CLG_a = 4,
	CLG_t = 5,
	CLG_g = 6,
	CLG_c = 7,
	CLG_I_ = 8,
	CLG_U_ = 9,
	CLG_Activator_ = 10,
	CLG_TCA_in_DCM_ = 11,
	CLG_WASH_ACN_DCM_ = 12,
	CLG_OXIDATION_IODINE_ = 13,
	CLG_OXIDATION_IODINE2_= 14,
	CLG_CAPPING_CAPA_ = 15,
	CLG_CAPPING_CAPB_ = 16,
	CLG_AMIDITE = 17,
};
enum OPTION_CONTROL_PRESSURE
{
	HIGH_PUSH = 0,
	LOW_PUSH = 1,
	HIGH_VACUUM = 2,
	LOW_VACUUM = 3,
};
enum MIX_AMIDITE
{
	AMD_A = 0,
	AMD_T = 1,
	AMD_G = 2,
	AMD_C = 3,
	AMD_a = 4,
	AMD_t = 5,
	AMD_g = 6,
	AMD_c = 7,
	AMD_I = 8,
	AMD_U = 9, // floatting 1
	AMD_Y = 10, // floatting 2
	AMD_R = 11,
	AMD_W = 12,
	AMD_S = 13,
	AMD_K = 14,
	AMD_M = 15,
	AMD_D = 16,
	AMD_V = 17,
	AMD_N = 18,
	AMD_H = 19,
	AMD_B = 20,
};

//enum SOLENOID_NAME
//{
//	FAN_SV = 0, // 33 xa khi nito giam do am
//	LED_RED_SV  = 1, // 34
//	LED_GREEN_SV  = 2, // 35
//	V26_EMPTY_SV  = 3, // 36  high push
//	OPEN_NITOR_SV  = 4,
//	HIGH_PUSH_SV = 5,
//	MEDIUM_PUSH_SV = 6,
//	LOW_PUSH_SV = 7,
//};

enum SOLENOID_NAME
{
	FAN_SV = 0, // 33 xa khi nito giam do am
	LED_RED_SV  = 1, // 34
	LED_GREEN_SV  = 2, // 35
	FAN_VACUUM_BOX  = 3, // 36  high push
	LOW_PUSH_SV  = 4,// V37
	HIGH_PUSH_SV = 5,
	V39_EMPTY = 6,
	OPEN_NITOR_SV  = 7,
};

enum SUBFUNCTION_STT_FB
{
	WAIT_AFTERFILL =0,
	PUSHDOWN_FNC = 1,
	WAIT_FNC = 2,
};

#endif /* INC_MACRO_H_ */
