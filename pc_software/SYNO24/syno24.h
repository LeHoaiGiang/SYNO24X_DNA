#ifndef SYNO24_H
#define SYNO24_H

#include <QMainWindow>
#include "QSerialPort"
#include "struct.h"
#include "QStandardItemModel"
#include "QMessageBox"
#include "function.h"
#include "struct.h"
#include "delay.h"
#include "qevent.h"
#include <QJsonObject>
#include <QJsonDocument>
#include "qradiobutton.h"
#include <QtWidgets>
#include "xlsxdocument.h"
#include "xlsxchartsheet.h"
#include "xlsxcellrange.h"
#include "xlsxchart.h"
#include "xlsxrichstring.h"
#include "xlsxworkbook.h"
#include "filemanager.h"
#include "amidite.h"
#include "progressbar_timer.h"
#include "Loghistory.h"
#include "trityl.h"
QT_BEGIN_NAMESPACE
namespace Ui { class SYNO24; }
QT_END_NAMESPACE

class SYNO24 : public QMainWindow
{
    Q_OBJECT

public:

    SYNO24(QWidget *parent = nullptr);
    ~SYNO24();
    ProgressBarTimer *m_progressPush;
    ProgressBarTimer *m_progressWait;
    QSerialPort *serialPort;
    QStandardItemModel* model_table_well;
    //path_file_manager_t path_file_manager;
    amidite amidite_;
    data_ui_t data_ui;
    function fnc;
    delay delay_ui;
    Filemanager filemanager;
    trityl m_baseVacuumBox;
    //amidite amidite_;
    void load_protocol_to_ui(quint8 u8_current_sub, quint8 u8_current_step);

    // 29-04-2025 tính năng floatting amidite
    QStandardItemModel *modelCoupling;
     QStandardItemModel *modelAmiditeSubSpecial;
    CouplingGroup couplingData;
    QList<QPair<uint8_t, uint8_t>> dataList;
    QList<quint8> coupling1List;
    QList<quint8> coupling2List;
    QList<quint8> couplingSpecialBaseList;

    QMap<int, int> itemIndexToRow; // Dùng để ánh xạ index combobox -> hàng trong bảng

private slots:
    void onSerialPortError(QSerialPort::SerialPortError);
    void initUIVolumeMNG();
    void serialReceived();
    void fnc_openSerialPort();
    void on_btn_start_update_fw_released();
    void init_syno24();
    void on_btn_Primming_released();
    void on_checkbx_sellect_all_toggled(bool checked);

    void on_btn_calib_step1_released();

    void on_btn_calib_step2_released();
    bool save_str_amidite_toJson(global_var_t* global_var, QString Json_Path);
    void on_btn_save_sequence_released();

    void on_btn_new_protocol_released();

    void on_btn_open_protocol_released();

    void on_btn_clear_data_step_released();

    void on_btn_Calib_released();

    void on_btn_fill_50ul_released();

    void on_btn_scanport_released();

    void on_btn_savecurrent_step_released();

    void on_btn_save_history_released();

    void on_btn_start_synthetic_released();

    void on_btn_Run2HomeStep_released();

    void on_btn_pause_synthetic_released();

    void on_spbox_number_sub_valueChanged(int arg1);

    void on_pushButton_5_released();

    void on_btn_stop_synthetic_released();

    void on_btn_RunStepper_released();

    void closeEvent (QCloseEvent *event);
    void Setstyle_groupsub(quint8 grbx);
    void checkSelected_STEP();
    void on_select_sub_1_toggled(bool checked);

    void on_select_sub_2_toggled(bool checked);

    void on_select_sub_3_toggled(bool checked);

    void on_select_sub_4_released();

    void on_select_sub_5_toggled(bool checked);

    void on_select_sub_4_toggled(bool checked);
    void checkSelectChemical();

    void setUI_FirstChemical(quint8 u8_step_cycle);

    quint8 get_FirstChemical(quint8 u8_chemical);
    void on_btn_ManualRun_released();

    void on_btn_backAtuoRun_released();

    void on_checkbx_manual_Allwell_toggled(bool checked);
    void Run_Manual_fill_Chemical();
    void on_btn_StartManual_released();

    void on_btn_StartManual_CtrlVacuum_released();
    void Run_Manual_CtrlVacuum();
    void Display_Protocol_to_user();
    void on_btn_save_protocol_released();
    void send_setting();
    bool ASK_VENDOR_ID();
    void get_sensor_humidity_tempareture();
    void wait_humidity();
    //void on_pushButton_released();
    void log(const QString& message);
    void readExcelSequence(QTableView *tableView, QString path);
    //void on_btn_ConnectPort_released();
    void on_btn_opeExcelSequence_released();
    void on_btn_sequence_released();
    void reload_table_sequence();
    void checkLineEditFormat(const QString &text);
    void onLineEditEditingFinished();
    void readSpecialBaseFromLineEdit(QLineEdit *lineEdit, uint16_t special_base[MAX_SEQUENCE_OF_WELL]);
    void printArray(const uint16_t special_base[MAX_SEQUENCE_OF_WELL]);
    bool isbaseSpecial(uint16_t number, const uint16_t special_base[MAX_SEQUENCE_OF_WELL]);
    void copy_sub_protocol_data(sub_protocol_t &dest, const sub_protocol_t &src);
    void copyAndInsertStep(int sourceSubIndex, int sourceStepIndex, int destSubIndex, int insertionIndex);
    void deleteStep(int subIndex, int stepIndex);
    void calculator_volume_and_process_UI();
    void onButtonReleased_Add_A();
    void onButtonReleased_Add_T();
    void onButtonReleased_Add_G();
    void onButtonReleased_Add_C();
    void onButtonReleased_Add_a();
    void onButtonReleased_Add_t();
    void onButtonReleased_Add_g();
    void onButtonReleased_Add_c();

    void onButtonReleased_Add_F1();
    void onButtonReleased_Add_F2();
    void onButtonReleased_Add_ACT();
    void onButtonReleased_Add_TCA();
    void onButtonReleased_Add_WASH();
    void onButtonReleased_Add_OX();
    void onButtonReleased_Add_CAPA();
    void onButtonReleased_Add_CAPB();

    void onButtonReleased_Sub_A();
    void onButtonReleased_Sub_T();
    void onButtonReleased_Sub_G();
    void onButtonReleased_Sub_C();
    void onButtonReleased_Sub_I(); // TẠM THỜI ĐANG DÙNG I - hoặc F1
    void onButtonReleased_Sub_U(); // TẠM THỜI DÙNG U - hoặc F2
    void onButtonReleased_Sub_ACT();
    void onButtonReleased_Sub_TCA();
    void onButtonReleased_Sub_WASH();
    void onButtonReleased_Sub_OX();
    void onButtonReleased_Sub_CAPA();
    void onButtonReleased_Sub_CAPB();
    //void on_btn_calculator_volume_released();

    //void on_btn_ConnectPort_released();

    void on_btn_AutoHome_Manual_released();

    void on_btn_save_edit_amidite_released();

    void on_btn_panel_connect_released();

    void on_btn_panel_control_machine_released();

    void on_btn_settingcfg_valve_released();

    void on_tabWidget_main_currentChanged(int index);

    void updateControlsState();
    void log_debug(const QString &message);
    void log_terminal_withTimestamp(const QString &message, int level = 0);
    void log_terminal(const QString &message, LogLevel level = Default);
    void log_terminal_append(const QString &message, LogLevel level, bool newLine = false);
    //  void log_terminal_append(const QString &message, LogLevel level = Default);

    void ManualControlSystem(); // dùng để thực hiện lệnh đóng mở tay các solenoid các valve và đèn
    void writeSettings();
    void readSettings();
    void scanAndSelectPort(const QString &OldPort);
    void on_btn_new_sequence_released();

    void on_btnHomeCalibration_released();
    //==============================================================================================
    void startCountdown(float interval_time);
    void updateCountdown();
    void stopCountdown();
    void pauseCountdown();
    void resumeCountdown();
    void CalEstimateTimeProtocol(quint32 *TimeEstimate);
    QString convertSecondsToHHMMSS(quint32 TimeEstimate) ;
    void on_btnCopySub_released();
    //void copy_sub_protocol_data(sub_protocol_t &dest, const sub_protocol_t &src);
    void on_btn_delsub_released();

    void on_btn_addsub_released();
    void setCellColor(QTableWidget* tableWidget, int row, int col, const QColor& color) ;
    void MonitorPlateUpdateUI(uint16_t baseFinished);
    void on_btn_InsertStep_released();

    void on_btn_delStep_released();

    void on_btnAddCoupling1_released();

    void on_btnAddCoupling2_released();

    void on_btndelGrCoupling_released();

    void on_btnSaveGrCoupling_released();
    void updateTableWithCouplingLists();
    // Các hàm tiện ích kiểm tra
    bool isInCoupling1(quint8 index) const;
    bool isInCoupling2(quint8 index) const;
    bool isInCouplingLastBase(quint8 index) const;
    void onCountdownFinished();
    void UpdateUISTTRun( uint8_t state);
    void on_btn_saveReagentFillCoupling2_released();

    void on_btnDeleteSub_released();

    void on_select_sub_6_toggled(bool checked);
    void on_select_sub_7_toggled(bool checked);
    void on_select_sub_8_toggled(bool checked);
    void on_select_sub_9_toggled(bool checked);
    void on_select_sub_10_toggled(bool checked);
    void on_btn_AddAmiditeSubSpecial_released();
    void updateTableWithSpecialList();
    void FindSignalLastedSequence();
    void on_btn_tabKillSequenceRun_released();
    //============================================================================================================================================
    // 03-08-2025 cap nhat ky thuat quan ly khi run
    void START_OLIGO_TASK();
    //LogHistory initializeAndSetup();
    //void waitForHumidityIfNeeded();
    //quint8 prepareSynthesis(LogHistory& LogHistoryRun);
    //void runSynthesisLoop(quint8 u8_number_sub_run, LogHistory& LogHistoryRun);
    //void runBaseSteps(quint8 u8_counter_sub_run, quint8 u8_counter_base_on_sub, LogHistory& LogHistoryRun);
    ///void sendOxidationSequence();
    //void processCouplingCappingSequence(quint8 u8_counter_sub_run, quint8 u8_counter_step, quint8 u8_first_chemical_temp);
    //void processCouplingSequenceAndTimefill(quint8 u8_counter_sub_run, quint8 u8_counter_step, quint8 couplingType);
    //bool sendStepDataToSTM32(quint8 u8_counter_sub_run, quint8 u8_counter_step, quint8 u8_first_chemical_temp, LogHistory& LogHistoryRun);
    //quint32 calculateProcessTime(quint8 u8_counter_sub_run, quint8 u8_counter_step, quint8 u8_first_chemical_temp);
    //void handleDoubleCoupling(quint8 u8_counter_sub_run, quint8 u8_counter_base_on_sub, quint8 u8_counter_step, quint8 u8_first_chemical_temp);
    //void handlePostBaseTasks();
    //void finalizeSynthesis(LogHistory& LogHistoryRun);
    //void handleAlreadyRunningOrDisconnected(LogHistory& LogHistoryRun);
    //void runSpecificSubBase(quint8 subIndex, quint8 baseIndex, LogHistory& logHistoryRun);
    void on_chkbox_Allbase_toggled(bool checked);
    void GetFeatureVacuumBox();
    void on_btn_FanVacuumBox_released();
    bool checkSyntheticSystemState();

    SyntheticResult initializeSyntheticOperation(LogHistory& logHistory) ;
    SyntheticResult waitForSyntheticHumidity();
    SyntheticResult calculateSyntheticTotalBases(uint8_t& u8_number_sub_run, uint8_t& u8_lastest_sub, uint16_t& u16_max_base_setting_protocol);
    SyntheticResult processAllSyntheticSubProtocols(uint8_t u8_number_sub_run, LogHistory& logHistory);
    SyntheticResult processSyntheticSubProtocol(uint8_t u8_counter_sub_run, LogHistory& logHistory);
    SyntheticResult processSyntheticBase(uint8_t u8_counter_sub_run, uint8_t u8_counter_base_on_sub, LogHistory& logHistory);
    void handleSyntheticPauseState();
    SyntheticResult processSyntheticStep(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, LogHistory& logHistory);
    SyntheticResult sendOxidationSequenceupdate();
    SyntheticResult processSyntheticStepChemical(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp);
    SyntheticResult processSyntheticCouplingStep(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp);
    SyntheticResult processSyntheticNonCouplingStep(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp);
    SyntheticResult completeCouplingCommand(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp, QByteArray& Command_send, uint32_t& u32_time_oligo_process_step);

    SyntheticResult handleDoubleCoupling(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp, uint32_t process_time);
    SyntheticResult finalizeSyntheticOperation(LogHistory& logHistory);
    SyntheticResult on_btn_start_synthetic_released_optimized();
    // 20-08-2025
    SyntheticResult processFinalSpecialCouplingStep(uint8_t template_sub_index, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp);
    SyntheticResult processFinalSpecialStep(uint8_t template_sub_index, uint8_t u8_counter_step, LogHistory& logHistory);
    SyntheticResult processFinalSpecialSubProtocol(LogHistory& logHistory);
    SyntheticResult sendFinalSpecialOxidationSequenceupdate();
    SyntheticResult processFinalSpecialSyntheticNonCouplingStep(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp);
    uint32_t calculateProcessTime(uint8_t u8_counter_sub_run, uint8_t u8_counter_step);
    void blinkLabel(QLabel* label, int remainingBlinks, const QString& originalStyleSheet);
    void pulseLabel(QLabel* label, const QString& text, const QString& color = "#FF4500");
    void on_chkbx_EnaSpecialLastBase_toggled(bool checked);

    int findMultiplierByPercentage(double percentage);
    void on_btn_AddStateMultiplier_released();
    void updateTableMultiplierView();
    void on_btn_DeleteMultiplier_released();

private:
    TableData m_tableDataMultiplier;
    QStandardItemModel *m_tableModel;
    Ui::SYNO24 *ui;
};
#endif // SYNO24_H
