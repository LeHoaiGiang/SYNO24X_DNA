#ifndef STRUCT_H
#define STRUCT_H
#include <QString>
#include "qglobal.h"
#include "macro.h"
#include <QByteArray>
#include <vector>
typedef union
{
    quint32 Data;
    quint8 Byte[4];
}FourByte_to_u32;
typedef union
{
    float Data;
    char Byte[4];
}float_to_u8_t;
typedef union
{
    quint16 Data;
    quint8 Byte[2];
}TwoByte_to_u16;
//***********************================================ CALIBRATION VALVE =======================================================
typedef struct
{
    float a;
    float b;
    float_to_u8_t f_a;
    float_to_u8_t f_b;
    int32_t volume_remain;
    int32_t volume_curent;
    uint32_t volume_calculator_need;
}Parameter_valve_t;// struct nay chua thong tin thoi gian bom hoa chat sau khi da calib
typedef struct
{
    FourByte_to_u32 u32fb_time_primming_calib; // thoi gian de test bom hoa chat xuong gieng va calib
    TwoByte_to_u16 u16tb_V1; // chua gia tri the tich lan 1
    TwoByte_to_u16 u16tb_V2; // chua gia tri the tich lan 2
}Calibration_t;

typedef struct
{
    bool valve[MAX_NUMBER_VALVE];
    quint8 u8_time_primming_control;
}primming_control_t;
//============================================================= AMIDITE ===========================================================
typedef struct
{
    uint8_t u8_valve_index;
    char u8_type_chemical[4];
    TwoByte_to_u16 u16tb_Volume[4];
}data_mixed_base_t;
typedef struct
{
    quint8 u8_sequence[MAX_SEQUENCE_OF_WELL];
    quint8 linkages[MAX_SEQUENCE_OF_WELL];
    quint8 lastSequence;
    quint8 FirstSequence;
    int32_t sequenceLength;
    int32_t LastbaseinWellCount;// biến này chứa giá trị base cuối cùng của giếng khi nó == counterfinishbase thì có nghĩa là
    TwoByte_to_u16 u16_timefill_well[3];
    QString string_sequence;
    QString string_sequence_converted;
    QString string_name;
    // u16_max_sequence_amidite_setting;
}Data_amidite_run_t; // dữ liệu của 1 well

//============================================================= STEP RUN OLIGO ====================================================
//=================================================================================================================================
typedef struct  {
    bool Enablefeature;
    uint8_t En_WASH;
    uint8_t En_Deblock;
    uint8_t En_Coupling;
    uint8_t En_Cap;
    uint8_t En_Ox;
    TwoByte_to_u16 time;
}VacuumBox_t; // tinh nang fill hoa chat vao cac cot bi trong
typedef struct  {
    bool EnableFillWellDone;
    uint8_t typeReagent;
    uint8_t En_WASH;
    uint8_t En_Deblock;
    uint8_t En_Coupling;
    uint8_t En_Cap;
    uint8_t En_Ox;
    TwoByte_to_u16 volumeWASH;
    TwoByte_to_u16 volumeDeblock;
    TwoByte_to_u16 volumeCoupling;
    TwoByte_to_u16 volumeCap;
    TwoByte_to_u16 volumeOx;
}FillChemistryDone_t; // tinh nang fill hoa chat vao cac cot bi trong
//=================================================================================================================================
typedef struct
{
    bool flag_autocheck_pressure; // cờ kiểm tra áp suất
    bool flag_auto_clean_box; // cờ xả hóa chất
    bool flag_auto_primming_chemical;
    bool flag_vacumm_waste;
    bool flag_exhaustFan; // Trạng thái quạt (bật/tắt)
    TwoByte_to_u16 u16tb_time_auto_clean;
    TwoByte_to_u16 u16tb_autoPrim_volume_amidite;
    TwoByte_to_u16 u16tb_autoPrim_volume_Activator;
    TwoByte_to_u16 u16tb_timeExhaustFan;
    float f_pressure_setting;
    VacuumBox_t VacuumBox;
    FillChemistryDone_t FillChemistryDone;
}advanced_setting_t;

typedef struct
{
    char u8_type_chemical[3];
    TwoByte_to_u16 u16tb_Volume[3];
}mix_function_chemical_t;

typedef struct
{
    TwoByte_to_u16 u16tb_procs_time[10];
    TwoByte_to_u16 u16tb_waitting_after_time[10];
    quint8 u8_option_pressure[10];
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
    // khai bao state 2 đên state  11
    control_pressure_t control_pressure;
    fill_chemical_t fill_chemical;
}Step_process_parameter_t;
//============================================================= FILE PATH =========================================================
typedef struct
{
    QString applicationDirpath;
    QString amidite_sequence_path;
    QString protocol_path;
    QString calibration_path;
    QString setting_path;
}path_file_manager_t;

typedef struct
{
    bool b_flag_connect_link;
    quint16 u16_counter_base_finished;
    quint16 u16_counter_current_base;
    quint16 u16_max_sequence_amidite_setting;
}signal_status_oligo_t;


typedef struct
{
    Step_process_parameter_t step[MAX_STEP_OF_SUB];
    quint8 u8_number_base_on_sub;
    uint8_t u8_number_step_on_base;
    quint8 douple_coupling_option;
}sub_protocol_t;
typedef struct
{
    sub_protocol_t sub[MAX_SUB_OF_PROTOCOL];
    quint8 u8_number_sub;
    quint8 u8_step_cycle;
    uint16_t speacial_base[MAX_SEQUENCE_OF_WELL];
    TwoByte_to_u16 u16_scale_volume; // TÍNH NĂNG TỰ ĐỘNG TĂNG VOLUME HÓA CHẤT TÍNH THEO %
    TwoByte_to_u16 u16_scale_time; // TÍNH NĂNG TỰ ĐỘNG TĂNG VOLUME HÓA CHẤT TÍNH THEO %
}protocol_t;


typedef struct
{
    uint8_t flag_have_feedback_value;
    TwoByte_to_u16 u16_temperature;
    TwoByte_to_u16 u16_humidity;
    double f_temperature;
    double f_humidity;
    QString str_humidity;
    QString str_temperature;
    bool flag_enable_auto_control_air_Nito;
    TwoByte_to_u16  u16tb_humidity_Preset;
    TwoByte_to_u16 Pos_X;
    TwoByte_to_u16 Pos_Y;
    TwoByte_to_u16 Pos_Z1;
    float fPosX;
    float fPosY;
    float fPosZ1;
}status_and_sensor_t;
/*************************************
 * Param chạy 2 trục XY
*/
typedef struct
{
    uint8_t u8_checked_well[MAX_WELL_AMIDITE];
    uint8_t u8_typeof_chemical;
    TwoByte_to_u16 u16_volume;
    quint8 u8_option_pressure[4];
    TwoByte_to_u16 u16tb_procs_time[4];
    TwoByte_to_u16 u16tb_waitting_after_time[4];
}manual_run_t;
typedef struct
{
    TwoByte_to_u16 u16tb_X_Distance;
    TwoByte_to_u16 u16tb_Y_Distance;
    TwoByte_to_u16 u16tb_Z1_Distance;
}Control_stepper_t;
typedef struct
{
    quint8 well_index[MAX_WELL_AMIDITE];
}signal_kill_t;
typedef struct
{
    quint8 u8_function_count;
    quint8 u8_subfunction; // =1 là đang chạy ép pushdown // =2 là đang đợi
    TwoByte_to_u16 u16tb_procs_time;
    TwoByte_to_u16 u16tb_waitting_after_time;
    QString log_control_pressure;
    uint16_t currentStep;
    uint16_t currentSub;
}updateSTTRun2UI_t;
typedef struct  {
    bool EnableFillWellDone;
    uint8_t typeReagent;
    TwoByte_to_u16 volume;
}Coupling2Setting_t;
typedef struct
{
    Control_stepper_t control_stepper;
    Parameter_valve_t valve_setting[MAX_NUMBER_VALVE];
    Data_amidite_run_t amidite_well[MAX_WELL_AMIDITE];
    primming_control_t primming_control;
    signal_status_oligo_t signal_status_oligo;
    status_and_sensor_t status_and_sensor;
    manual_run_t manual_run;
    Step_process_parameter_t dataCurrentStepOligo;
    advanced_setting_t advanced_setting;
    signal_kill_t signal_kill;
    updateSTTRun2UI_t updateSTTRun2UI;
    Coupling2Setting_t Coupling2Setting;
}global_var_t;

typedef struct
{
    uint8_t u8_current_SUB_edit_ui;
    uint8_t u8_current_STEP_edit_ui;
}data_ui_t;

//==========================================================================================================================================================================================
struct CouplingGroup {
    QList<QString> coupling1;
    QList<QString> coupling2;
};
// 03-11-2025
struct TableRowData {
    int from;
    int to;
    int multiplier;
};

struct TableData {
    std::vector<TableRowData> rows;
    int multiplier;
    int totalInitialWells;
    int rowCount() const {
        return rows.size();
    }
};

#endif // STRUCT_H
