#ifndef MACRO_H
#define MACRO_H
#include "qstring.h"
#include <QStringList>
#define LENGTH_COMMAND_RECEIVED     512
#define LENGTH_COMMAND_SEND         512
#define BAUDRATE_UART               115200
//#define UART_LENGTH_COMMAND 		256
//#define UART_LENGTH_DATA_OLIGO 		256
#define CMD_CONNECT_LINK			0x01
#define CMD_RUNSTEPPER				0x03
#define CMD_RUN2HOME				0x02
#define CMD_PRIMMING				0x04
#define CMD_START_OLIGO_STEP		0x05
#define CMD_DATA_OLIGO				0x06
#define CMD_FIRMWARE_END_OLIGO_STEP 0x0A
#define CMD_FIRMWARE_UPDATE_STATUS_AND_SENSOR	0x0B
#define CMD_CONTROL_AIR_START       0x0C
#define CMD_CONTROL_MIXED_AIR		0x0D
#define CMD_RECIVED_SETTING			0x10
#define CMD_OX_SENQUENCE            0x11 // gui command Ox, gửi thông tin Ox2 cho stm32
#define CMD_FIRMWARE_UPDATE_RUNNING_STATE		0x12
#define CMD_FIRMWARE_REQUEST_DATA_AGAIN			0x13
#define CMD_ASK_VENDOR_ID			0x07
#define CMD_SIGNAL_START_OLIGO      0x08
#define CMD_SEND_STOPED				0x09
#define CMD_CALIBRATION_VALVE   	0x64
#define CMD_EXHAUSTED_CHEMICAL		0x65
#define CMD_MANUAL_RUN          	0x66
#define CMD_PRESURE_TESTING     	0x19
#define CMD_STOP_SYSTHETIC_OLIGO    0x99
#define CMD_FW_FEEDBACK_STATUS_RUNNING  0x27
#define CMD_FEEDBACK_STATUS_RUN         0x67
// synchronize input output control
#define CMD_CONTROL_SYNCHRONIZE_IO	0x23
#define CMD_SEUQENCE_AND_KILL       0x25
//============================================================= AMIDITE & VALVE MACRO ==========================================
#define CHEMICAL_SUBTANCE_EMPTY    	0x7F
#define CHEMICAL_SUBTANCE_HAVE      0x02
#define MAX_WELL_AMIDITE			24
#define MAX_NUMBER_VALVE			17
#define MAX_SEQUENCE_OF_WELL        127
#define MAX_SUB_OF_PROTOCOL         10
#define MAX_STEP_OF_SUB             20

#define INTERVAL_TIMEFILL_CALIB_1ST         800
#define INTERVAL_TIMEFILL_CALIB_2ND         2000
#define VOLUME_FILL_50UL                    50
//==============================================================================================================================
//#define START_PROCESS_SYNTHETIC_OLIGO                   1
//#define STOP_PROCESS_SYNTHETIC_OLIGO                    0
//#define MANUAL_PROCESS_SYNTHETIC_OLIGO                  1
//#define STOP_MANUAL_PROCESS_SYNTHETIC_OLIGO             0

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
// AMIDITE FORMATTING
enum MIX_AMIDITE
{
    AMD_A = 0, // valve 1
    AMD_T = 1,
    AMD_G = 2,
    AMD_C = 3,
    AMD_a = 4,
    AMD_t = 5,
    AMD_g = 6,
    AMD_c = 7,
    AMD_I = 8, // I là 8
    AMD_U = 9, // U là 9
    AMD_Y = 10,
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
    LOW_PUSH = 1,
    HIGH_PUSH = 0,
    HIGH_VACUUM = 2,
    LOW_VACUUM = 3,
};

enum OPTION_CHEMICAL_STEP
{
    DEBLOCK_FNC = 0,
    WASH_FNC = 1,
    COUPLING_FNC = 2,
    OXICATION_FNC = 3,
    CAP_FNC = 4,
    OXICATION_FNC_2 = 5,
    COUPLING_FNC_2 = 6
};
enum OPTION_DOUPLE_COUPLING
{
    NONE_DOUBLE_COUPLING = 0,
    DOUBLE_COUPLING_FIRSTBASE = 1,
    DOUBLE_COUPLING_FIRST_SECOND_BASE = 2,
    DOUBLE_COUPLING_ALL_BASE = 3
};

QString const PATH_SAVE_AMIDITE = "C:/software/00_Data/Amidite.json";

enum TAB_UI
{
     SYSTEM_TAB = 0,
     SEQUENCE_TAB= 1,
     VOLUME_MANAGER_TAB = 2,
     PROTOCOL_TAB = 3,
     RUN_TAB = 4,
     UPDATE_FW_TAB  = 5
};


enum LogLevel {
    Normal = 0,   // Black
    Error,        // Red
    Success,      // Green
    Warning,      // Yellow
    Info,         // Blue
    Debug,        // Magenta
    Trace,        // Cyan
    Default       // White
};

enum SUBFUNCTION_STT_FB
{
    WAIT_AFTERFILL =0,
    PUSHDOWN_FNC = 1,
    WAIT_FNC = 2,
};



// ===== ERROR HANDLING SYSTEM =====
enum class SyntheticResult : int {
    SUCCESS = 0,
    INVALID_STATE,
    CONNECTION_ERROR,
    FIRMWARE_ERROR,
    HUMIDITY_TIMEOUT,
    USER_STOPPED,
    SEQUENCE_COMPLETE,
    PROTOCOL_ERROR,
    INVALID_PARAMETER,
    TIMEOUT_ERROR,
    CRC_ERROR,
    UNKNOWN_ERROR
};



// ===== SYNTHETIC OPERATION STATE =====
struct SyntheticOperationState {
    uint8_t current_sub = 0;      // Current sub-protocol index
    uint8_t current_step = 0;     // Current step index
    uint8_t current_base = 0;     // Current base index
    uint16_t bases_completed = 0; // Total bases completed
    uint16_t total_bases = 0;     // Total bases to process
    bool is_paused = false;       // Pause state
    bool is_stopped = false;      // Stop state

    void reset() {
        current_sub = current_step = current_base = 0;
        bases_completed = total_bases = 0;
        is_paused = is_stopped = false;
    }
};


// ===== CONFIGURATION CONSTANTS =====
namespace SyntheticConfig {
    constexpr int HUMIDITY_CHECK_INTERVAL_MS = 500;
    constexpr int DELAY_BETWEEN_STEPS_MS = 60;
    constexpr int DELAY_BETWEEN_BASES_MS = 10;
    constexpr int DELAY_BETWEEN_SUBS_MS = 20;
    constexpr int COMMAND_TIMEOUT_MS = 10000;
    constexpr uint32_t STEP_TIMEOUT_BASE_MS = 100000;
    constexpr double OXIDATION_TIME_MULTIPLIER = 2.5;
    constexpr int SENSOR_UPDATE_INTERVAL_MS = 5000;
}



#endif // MACRO_H
