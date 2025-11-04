#ifndef VOLUME_MANAGER_H
#define VOLUME_MANAGER_H
#include "qglobal.h"
#include "struct.h"
#include "macro.h"
#include "QString"
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include "QDebug"
#include "QSpinBox"
#include "QPushButton"
#include <QtWidgets>
#define MAX_VALVE 17
#define MAX_INDEX_TABLE 4
class volume_manager: public QObject
{
public:
    /*
    AMD_a = 4,
    AMD_t = 5,
    AMD_g = 6,
    AMD_c = 7,
    */
    const int16_t MIXED_BASE[21][4]=
    {
        {AMD_A,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY}, //0
        {AMD_T,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//1
        {AMD_G, CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//2
        {AMD_C, CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//3
        {AMD_a,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY}, //4
        {AMD_t,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},// 5
        {AMD_g, CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//6
        {AMD_c, CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//7
        {AMD_I, CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//8 FLOATTING 1
        {AMD_U, CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY,CHEMICAL_SUBTANCE_EMPTY},//9 FLOATTING 2
        {AMD_C, AMD_T, CHEMICAL_SUBTANCE_EMPTY, CHEMICAL_SUBTANCE_EMPTY},//10 AMD_Y
        {AMD_G, AMD_A, CHEMICAL_SUBTANCE_EMPTY, CHEMICAL_SUBTANCE_EMPTY},//11 AMD_R
        {AMD_A, AMD_T, CHEMICAL_SUBTANCE_EMPTY, CHEMICAL_SUBTANCE_EMPTY},//12 AMD_W
        {AMD_G, AMD_C, CHEMICAL_SUBTANCE_EMPTY, CHEMICAL_SUBTANCE_EMPTY},//13 AMD_S
        {AMD_G, AMD_T, CHEMICAL_SUBTANCE_EMPTY, CHEMICAL_SUBTANCE_EMPTY},//14 AMD_K
        {AMD_A, AMD_C, CHEMICAL_SUBTANCE_EMPTY, CHEMICAL_SUBTANCE_EMPTY},//15 AMD_M
        {AMD_G, AMD_A, AMD_T, CHEMICAL_SUBTANCE_EMPTY},//16 AMD_D
        {AMD_G, AMD_A, AMD_C, CHEMICAL_SUBTANCE_EMPTY},//17 AMD_V
        {AMD_G, AMD_A, AMD_C, AMD_T},//18 AMD_N
        {AMD_A, AMD_C, AMD_T, CHEMICAL_SUBTANCE_EMPTY},//19 AMD_H
        {AMD_G, AMD_C, AMD_T, CHEMICAL_SUBTANCE_EMPTY},//20 AMD_B
    };
    enum INDEX_COLUMN_TABLE
    {
        NAME_CHEMICAL_IDX = 0,
        CURRENT_VOLUME_IDX =1,
        CALCULATOR_VOLUME_IDX =2,
        REMAIN_VOLUME_IDX = 3,
    };
    const QStringList NAME_CHEMICAL = {"A", "T", "G", "C", "a", "t", "g", "c", "Float 1","Float 2", "ACTIVATOR", "TCA", "WASH", "OX", "OX2", "CAP-B", "CAP-A"};
    //const int numValves = 12;  // Số lượng van
    QTableView *tableView;
    QStandardItemModel *model = new QStandardItemModel(MAX_VALVE, 4);
    // Khai báo mảng chứa con trỏ đến QSpinBox

    QSpinBox* spbxs_current[MAX_VALVE];
    //=========================================
    QSpinBox* spbxs_change[MAX_VALVE];
    // VAR
    Parameter_valve_t valve[MAX_VALVE];
    QString valveSetting_Path;
    // OBJECT
    volume_manager();

    void setPath(QString Path);
    void save_parameter_valve();
    void read_parameter_valve();
    void add_volume(int16_t type_chemical, uint16_t volume);
    void sub_volume(int16_t type_chemical, uint16_t volume);
    void sub_volume_running(uint8_t type_chemical, uint16_t volume);
    void sub_volume_amidite(uint8_t AMIDITE,  uint16_t volume);
    void add_volume_normal_cal(int16_t type_chemical, uint16_t volume);
    void add_volume_amidite_cal(uint8_t AMIDITE,  uint16_t volume);

    quint16 valve_calculator_timefill( uint8_t type_sulphite, uint16_t u16_Volume);
    void reset_volume_cal();
    void cal_remain_volume();
    void Calculator_Protocol();
    void ReloadUIVolumeMNG();
    void onButtonReleased_Add_Chemical(ORDINAL_VALVE Chemical);
    void onButtonReleased_Sub_Chemical(ORDINAL_VALVE Chemical);

    void tableView_init();
    void tableView_display_data();
    void setCellColor(int rowIndex, int columnIndex, QColor color);
};

#endif // VOLUME_MANAGER_H
