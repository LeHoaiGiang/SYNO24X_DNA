#include "syno24.h"
#include "ui_syno24.h"
#include "serial_custom.h"
#include "QMessageBox"
#include "struct.h"
#include <QCoreApplication>
#include <QFileInfo>
#include <QCoreApplication>
#include <QFileInfo>
#include <QtCore>
#include <QAxObject>
#include "state_machine.h"
#include <QRegularExpression>
#include <QMessageBox>
#include <QStringList>
#include "volume_manager.h"
#include "amidite.h"

#include "killsequence.h"

/*
 * Created on: Dec 3, 2023
 * Author: LeHoaiGiang
 * 01-10-2023 khoi tao du an bat dau thiet ke cac tinh nang SYNO24X dựa trên Resource SYNO2400  GiangLH22
 * 24-10-2024 phát hiện primming bị ngược 14 15 nếu có bị sai thứ tự thì kiểm tra lại chỗ này trên file ui
 * 12-11-2024 da hoat dong cac tinh nang SYNO24X = Phiên bản BETA mọi tính năng đã hoạt động và các phản ứng tổng hợp Gen đã thành công rất tốt
 *
 * BẮT ĐẦU HOÀN THIỆN DỰ ÁN TỪ GIAI ĐOẠN NÀY *********************************************** CỐ LÊN NHOAAA
 *
 * 10-02-2025 chua kiem tra dong  1525 chua tesst chuc nang nay auto prrimming amidite
 * 26-02-2025 update tính toán thời gian ước tính của software + đồng hồ đếm số
 * 21/03/2025 Giang Them flag và thời gian exhaustFan
 * 10-04-2025 tính năng insert Step bất kì vị trí nào trong protocol 70% AI
 * 16-04-2025 sửa thời gian đợi trên màn hình Backend lấy số trên ui / 100 rồi gửi cho FW
 * 22/04-2025 cập nhật tính năng xóa insert Step bất kì vị trí nào trong protocol
 * 05-05-2025 thêm tính năng mới coupling 2 amidite xử lý tương tự như OX2.
 * 05-05-2025 tính năng floatting amidite very flexible... người dùng tùy chọn cài đặt
 * 20-05-2025 tính năng coupling 2, xử lý sequence khi gặp coupling 1 và coupling 2
 * 22-05-2025 table chọn group amidite người dùng chọn amidite thuộc coupling 1 hay coupling 2
 * 22-05-2025 xóa một sub bất kì, tăng số sub lên 10 , tằng số step tối đa lên 20
 * 22-05-2025 monitor status run, hiển thị realtime và đếm thời gian ép thời gian chờ của máy
 * 06-06-2025 cap nhat xu ly amidite mixed base
 * 16-09-2025 cap nhat xu lý Run last base amidte đặc biệt, người dùng cấu hình last base, sẽ được giữ lại để chạy với sub đặc biệt
 *
 *
*/

#define SCALE_LEVEL_WAITTIME 100
#define MAIN_WINDOW_DEBUG
#define MAIN_WINDOW_DEBUG_SYNC_OLIGO
#define DEBUG_SOFTWARE
QString widget_stylesheet_enable = "QWidget{ border: 3px solid; 	border-color: #00FF00;}";
QString widget_stylesheet_disable = "QWidget { background-color: #FFFFFF;}";
QStringList STEP_NAME = { "DEBLOCK", "WASHING", "COUPLING", "OXIDATION", "CAPPING", "OXIDATION2", "COUPLING 2" };
QStringList NAME_OPTION_PRESSURE = {"HP", "LP", "HV", "LV"};
QStringList NAME_MIX_FUNCTION = {"A", "T", "G", "C","a", "t", "g", "c", "U","I", "ACTIVATOR", "TCA", "WASH", "OX1", "OX2", "CAP-B", "CAP-A", "AMIDITE"};
QString name_well_asign ="ABCDEFGH";
serial_custom STM32_COM;
QByteArray data_uart_received(LENGTH_COMMAND_RECEIVED, 0);
global_var_t global_var;
Calibration_t cablib_valve;
protocol_t protocol_oligo;
QTimer timer_update_humidity_tempareture;
Filemanager filemanager;
volume_manager volume;
using namespace QXlsx;
state_machine syno24_machine;
QTimer m_timerConnection;
QTimer *timerEstimate;
int secondsRemaining;
bool isPausedEstimate;
QRadioButton *radioSubsellect[MAX_SUB_OF_PROTOCOL];
QSpinBox *spinbxNumberBaseOnSub;
QSpinBox *spinbxNumberStepOnBase;
QComboBox *cbx_option_coupling_sub;


class ChemicalWells {
public:
    // 1. Tính tổng giếng chứa hóa chất ban đầu
    static int calculateTotalChemicalWells(const uint8_t u8_sequence[MAX_WELL_AMIDITE]) {
        int totalWells = 0;
        for (int i = 0; i < MAX_WELL_AMIDITE; ++i) {
            if (u8_sequence[i] != 127) {
                totalWells++;
            }
        }
        return totalWells;
    }

    // 2. Hàm kiểm tra số giếng còn lại và tính tỉ lệ %
    static QPair<int, double> checkRemainingChemicalWells(const uint8_t current_sequence[96], int initialTotalWells) {
        int remainingWells = 0;
        int emptyWellsAtIndex = 0; // Đếm số phần tử có giá trị 127

        for (int i = 0; i < MAX_WELL_AMIDITE; ++i) {
            if (current_sequence[i] != 127) {
                remainingWells++;
            } else {
                emptyWellsAtIndex++;
            }
        }

        double percentage = 0.0;
        if (initialTotalWells > 0) {
            percentage = static_cast<double>(remainingWells) / initialTotalWells * 100.0;
        }

        return qMakePair(remainingWells, percentage);
    }
};


SYNO24::SYNO24(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SYNO24)
{
    ui->setupUi(this);
    qputenv("QT_IM_MODULE", QByteArray("qtvirtualkeyboard"));
    //  QTableView *tableView = ui->tableView;
    //  readExcelAndPopulateTableView(ui->tableView);
    serialPort = new QSerialPort(this);
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos)
    {
        ui->cbx_comport->addItem(info.portName());
    }
    radioSubsellect[0] = ui->select_sub_1;
    radioSubsellect[1] = ui->select_sub_2;
    radioSubsellect[2] = ui->select_sub_3;
    radioSubsellect[3] = ui->select_sub_4;
    radioSubsellect[4] = ui->select_sub_5;

    radioSubsellect[5] = ui->select_sub_6;
    radioSubsellect[6] = ui->select_sub_7;
    radioSubsellect[7] = ui->select_sub_8;
    radioSubsellect[8] = ui->select_sub_9;
    radioSubsellect[9] = ui->select_sub_10;
    spinbxNumberBaseOnSub = ui->spbox_numbase_sub_1;
    spinbxNumberStepOnBase = ui->spbox_num_step_sub_1;
    cbx_option_coupling_sub = ui->cbx_option_coupling_sub_1;
    // setWindowIcon(QIcon(":/image/icon/SYNO24_256PPI.png"));
    STM32_COM.flag_connecttion = false;

    connect(ui->btn_ConnectPort, SIGNAL(released()), this, SLOT(fnc_openSerialPort())); // OPEN SERIAL
    connect(serialPort, SIGNAL(error(QSerialPort::SerialPortError)), this, SLOT(onSerialPortError(QSerialPort::SerialPortError)));
    connect(serialPort, SIGNAL(readyRead()), this, SLOT(serialReceived())); // ham xu ly doc du lieu UART
    QObject::connect(&timer_update_humidity_tempareture, SIGNAL(timeout()), this, SLOT(get_sensor_humidity_tempareture()));
    // Kết nối sự kiện editingFinished với hàm onLineEditEditingFinished
    //connect(ui->lineEdit_special_base, &QLineEdit::editingFinished, this, SLOT(onLineEditEditingFinished));
    timer_update_humidity_tempareture.start(5000);
    init_syno24();
    initUIVolumeMNG(); // 23.08.24 hàm tính volume có vấn đề khi dùng với syno24X số valve tăng lên nên khác rồi cần tính lại
    //  state machine process ==============================================================================================
    syno24_machine.setBtnStartAuto(ui->btn_start_synthetic);
    syno24_machine.setBtnStopAuto(ui->btn_stop_synthetic);
    syno24_machine.setBtnPauseAuto(ui->btn_pause_synthetic);

    syno24_machine.setBtnStartFillChemicalManual(ui->btn_StartManual);
    syno24_machine.setBtnStartPushDownManual(ui->btn_StartManual_CtrlVacuum);
    syno24_machine.setBtnHomeManual(ui->btn_AutoHome_Manual);

    syno24_machine.state_machine_init();
    //ui->stackedWidget_home->setCurrentIndex(0);
    ui->tabWidget_main->setCurrentIndex(0);
    ui->stackedWidget_Run->setCurrentIndex(0);

    //====================================== MULTIPLIER============================================

    // Khởi tạo model cho QTableView
    m_tableModel = new QStandardItemModel(0, 3, this); // 0 rows, 3 columns
    m_tableModel->setHorizontalHeaderLabels({"From", "To", "Multiplier"});
    ui->tableView_2->setModel(m_tableModel);

    readSettings();
    // Timer uoc tính thời gian chạy
    timerEstimate = new QTimer(this);
    //float secondsRemainingEstimate;
    //bool isPausedEstimate;
    connect(timerEstimate, SIGNAL(timeout()), this, SLOT(updateCountdown()));
    //
    m_progressPush = new ProgressBarTimer(ui->prgBar_Push, this);
    m_progressWait = new ProgressBarTimer(ui->prgBar_Wait, this);
    connect(m_progressWait, &ProgressBarTimer::finished, this, &SYNO24::onCountdownFinished);
    // **********************************************************************************************************************

}

SYNO24::~SYNO24()
{
    delete ui;
}
void SYNO24:: initUIVolumeMNG()
{

    volume.tableView = ui->tableView_calculator_volume;
    volume.tableView_init();
    volume.spbxs_current[A] = ui->spbx_current_vl_A;
    volume.spbxs_current[T] = ui->spbx_current_vl_T;
    volume.spbxs_current[G] = ui->spbx_current_vl_G;
    volume.spbxs_current[C] = ui->spbx_current_vl_C;

    volume.spbxs_current[a] = ui->spbx_current_vl_a;
    volume.spbxs_current[t] = ui->spbx_current_vl_t;
    volume.spbxs_current[g] = ui->spbx_current_vl_g;
    volume.spbxs_current[c] = ui->spbx_current_vl_c;

    volume.spbxs_current[I] = ui->spbx_current_vl_F1;
    volume.spbxs_current[U] = ui->spbx_current_vl_F2;
    volume.spbxs_current[Activator] = ui->spbx_current_vl_ACT;
    volume.spbxs_current[TCA_in_DCM] = ui->spbx_current_vl_TCA;
    volume.spbxs_current[WASH_ACN_DCM] = ui->spbx_current_vl_WASH;
    volume.spbxs_current[CAPPING_CAPA] = ui->spbx_current_vl_CAPA;
    volume.spbxs_current[CAPPING_CAPB] = ui->spbx_current_vl_CAPB;
    volume.spbxs_current[OXIDATION_IODINE] = ui->spbx_current_vl_OX;

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    volume.spbxs_change[A] = ui->spbx_change_vl_A;
    volume.spbxs_change[T] = ui->spbx_change_vl_T;
    volume.spbxs_change[G] = ui->spbx_change_vl_G;
    volume.spbxs_change[C] = ui->spbx_change_vl_C;
    volume.spbxs_change[a] = ui->spbx_change_vl_a;
    volume.spbxs_change[t] = ui->spbx_change_vl_t;
    volume.spbxs_change[g] = ui->spbx_change_vl_g;
    volume.spbxs_change[c] = ui->spbx_change_vl_c;

    volume.spbxs_change[I] = ui->spbx_change_vl_F1;
    volume.spbxs_change[U] = ui->spbx_change_vl_F2;
    volume.spbxs_change[Activator] = ui->spbx_change_vl_ACT;
    volume.spbxs_change[TCA_in_DCM] = ui->spbx_change_vl_TCA;
    volume.spbxs_change[WASH_ACN_DCM] = ui->spbx_change_vl_WASH;
    volume.spbxs_change[CAPPING_CAPA] = ui->spbx_change_vl_CAPA;
    volume.spbxs_change[CAPPING_CAPB] = ui->spbx_change_VL_CAPB;
    volume.spbxs_change[OXIDATION_IODINE] = ui->spbx_change_vl_OX;
    calculator_volume_and_process_UI();
    connect(ui->btn_add_A,SIGNAL(released()),this,SLOT(onButtonReleased_Add_A())); //
    connect(ui->btn_add_T,SIGNAL(released()),this,SLOT(onButtonReleased_Add_T())); //
    connect(ui->btn_add_G,SIGNAL(released()),this,SLOT(onButtonReleased_Add_G())); //
    connect(ui->btn_add_C,SIGNAL(released()),this,SLOT(onButtonReleased_Add_C())); //
    connect(ui->btn_add_a,SIGNAL(released()),this,SLOT(onButtonReleased_Add_a())); //
    connect(ui->btn_add_t,SIGNAL(released()),this,SLOT(onButtonReleased_Add_t())); //
    connect(ui->btn_add_g,SIGNAL(released()),this,SLOT(onButtonReleased_Add_g())); //
    connect(ui->btn_add_c,SIGNAL(released()),this,SLOT(onButtonReleased_Add_c())); //

    connect(ui->btn_add_F1,SIGNAL(released()),this,SLOT(onButtonReleased_Add_F1())); //
    connect(ui->btn_add_F2,SIGNAL(released()),this,SLOT(onButtonReleased_Add_F2())); //
    connect(ui->btn_add_ACT,SIGNAL(released()),this,SLOT(onButtonReleased_Add_ACT())); //
    connect(ui->btn_add_TCA,SIGNAL(released()),this,SLOT(onButtonReleased_Add_TCA())); //
    connect(ui->btn_add_Wash,SIGNAL(released()),this,SLOT(onButtonReleased_Add_WASH())); //
    connect(ui->btn_add_Ox,SIGNAL(released()),this,SLOT(onButtonReleased_Add_OX())); //
    connect(ui->btn_add_CapA,SIGNAL(released()),this,SLOT(onButtonReleased_Add_CAPA())); //
    connect(ui->btn_add_CapB,SIGNAL(released()),this,SLOT(onButtonReleased_Add_CAPB())); //

    connect(ui->btn_sub_A,SIGNAL(released()),this,SLOT(onButtonReleased_Sub_A())); //
    connect(ui->btn_sub_T,SIGNAL(released()),this,SLOT(onButtonReleased_Sub_T())); //
    connect(ui->btn_sub_G,SIGNAL(released()),this,SLOT(onButtonReleased_Sub_G())); //
    connect(ui->btn_sub_C,SIGNAL(released()),this,SLOT(onButtonReleased_Sub_C())); //
    connect(ui->btn_sub_F1,SIGNAL(released()),this,SLOT(onButtonReleased_Sub_I())); //
    connect(ui->btn_sub_F2,SIGNAL(released()),this,SLOT(onButtonReleased_Sub_U())); //
    connect(ui->btn_sub_ACT,SIGNAL(released()),this,SLOT(onButtonReleased_Sub_ACT())); //
    connect(ui->btn_sub_TCA,SIGNAL(released()),this,SLOT(onButtonReleased_Sub_TCA())); //
    connect(ui->btn_sub_Wash,SIGNAL(released()),this,SLOT(onButtonReleased_Sub_WASH())); //
    connect(ui->btn_sub_Ox,SIGNAL(released()),this,SLOT(onButtonReleased_Sub_OX())); //
    connect(ui->btn_sub_CapA,SIGNAL(released()),this,SLOT(onButtonReleased_Sub_CAPA())); //
    connect(ui->btn_sub_CapB,SIGNAL(released()),this,SLOT(onButtonReleased_Sub_CAPB())); //
    volume.read_parameter_valve(); // hiển thị số lượng hoá chất
    volume.ReloadUIVolumeMNG();
}

void SYNO24::closeEvent (QCloseEvent *event)
{
    // Hiển thị hộp thoại xác nhận
    // 09-01-2025 them chuc nang nhac nho nguoi dung khi thoat
    if (QMessageBox::question(this,
                              "Quit Application",
                              "Do you want to quit the application? This will stop all system operations.",
                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
    {
        // Xử lý logic trước khi thoát
        if (serialPort) {
            serialPort->close();
            delete serialPort;
            serialPort = nullptr;
        }

        volume.save_parameter_valve();
        filemanager.save();
        writeSettings();
        // Chấp nhận sự kiện đóng
        event->accept();
        QCoreApplication::quit();
    }
    else
    {
        // Người dùng chọn "No", hủy sự kiện đóng
        event->ignore();
    }
}

void SYNO24:: init_syno24()
{
    global_var.status_and_sensor.flag_have_feedback_value = false;
    filemanager.load();
    fnc.read_protocol_fromJson(&protocol_oligo,filemanager.protocol_Path);
    ui->lineEdit_float_1->setText(amidite_.sequence_floatting_1);
    ui->lineEdit_float_2->setText(amidite_.sequence_floatting_2);
    /* Create the ComboBox elements list (here we use QString)
     * 31-10-24 GiangLH add list item Text for valve, support display Floatting amidite
    */
    //ui->cbx_type_chemical_mix_1->addItems(stringsList_chemical_valve);
    //ui->cbx_type_chemical_mix_2->addItems(stringsList_chemical_valve);
    //ui->cbx_type_chemical_mix_3->addItems(stringsList_chemical_valve);
    volume.setPath(filemanager.valveSetting_Path);
    ui->lineEdit_path_sequence->setText(filemanager.amidite_sequence_Path);
    // Process TABLE AMIDITE
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    // Đặt độ rộng tối thiểu cho cột 1 và cột 2
    ui->tableView->horizontalHeader()->setMinimumSectionSize(50);
    ui->tableView->horizontalHeader()->resizeSection(0, 50);
    ui->tableView->horizontalHeader()->setMinimumSectionSize(150);
    ui->tableView->horizontalHeader()->resizeSection(1, 150);
    //ui->tableView->setColumnWidth(1,500);

    model_table_well = new QStandardItemModel(MAX_WELL_AMIDITE, 4, this);
    QStringList verticalHeader;
    ui->tableView->setModel(model_table_well);
    //fprintf(stdout, "NAME");
    QStringList horizontalHeader = QStringList() << "Position" << "Name"<< "Sequence Length"<<"Sequence Oligo (5' 3')";
    model_table_well->setHorizontalHeaderLabels(horizontalHeader);
    model_table_well->setVerticalHeaderLabels(verticalHeader);
    //fnc.getData2AmiditeProcess(&global_var);
    // Generate data to tableview AMIDITE SET CONNECTTION UI
    amidite_.bottleNameInputs[0] = ui->lineEdit_float_1;
    amidite_.bottleNameInputs[1] = ui->lineEdit_float_2;
    amidite_.bottleNameInputs[2] = ui->lineEdit_float_3;
    amidite_.bottleNameInputs[3] = ui->lineEdit_float_4;
    amidite_.bottleNameInputs[4] = ui->lineEdit_float_5;
    amidite_.bottleNameInputs[5] = ui->lineEdit_float_6;

    amidite_.setpath(filemanager.amidtie_floatting_path);
    amidite_.readAmiditeFloatting(amidite_.sequence_floatting_1, amidite_.sequence_floatting_2);
    //amidite_.amiditeSyno24XGetSequence(&global_var);
    fnc.read_str_amidite_fromJson(&global_var, filemanager.amidite_sequence_Path); // READING FILE SEQUENCE TO RUN
    //fnc.getData2AmiditeProcess(&global_var);
    amidite_.amiditeSyno24XGetSequence(&global_var);

    //ui->lineEdit_float_1->setText(amidite_.sequence_floatting_1);
    //ui->lineEdit_float_2->setText(amidite_.sequence_floatting_2);

    // load floatting amidite lên ui

    ui->cbx_valve_selected->clear();
    ui->cbx_type_chemical_manual_run->clear();
    ui->cbx_type_chemical_mix_1->clear();
    ui->cbx_type_chemical_mix_2->clear();
    ui->cbx_type_chemical_mix_3->clear();
    ui->cbx_coupling2Option->clear();
    ui->cbx_type_reagentDelivery->clear();
    ui->cbx_valve_selected->addItems(amidite_.bottleNamesListFull);
    ui->cbx_type_chemical_manual_run->addItems(amidite_.bottleNamesListFull);
    amidite_.bottleNamesListFull.append("AMIDITE"); // thêm từ floatting amidte
    ui->cbx_type_chemical_mix_1->addItems(amidite_.bottleNamesListFull);
    ui->cbx_type_chemical_mix_2->addItems(amidite_.bottleNamesListFull);
    ui->cbx_type_chemical_mix_3->addItems(amidite_.bottleNamesListFull);
    ui->cbx_type_reagentDelivery->addItems(amidite_.bottleNamesListFull);
    ui->cbx_coupling2Option->addItems(amidite_.bottleNamesListAmidite);
    // load xong

    reload_table_sequence();
    ui->lineEdit_path_protocol->setText(filemanager.protocol_Path);
    data_ui.u8_current_SUB_edit_ui = 0;
    data_ui.u8_current_STEP_edit_ui = 0;
    load_protocol_to_ui(0,0);
    // xu ly chon sub nao
    //ui->selected_oxidation2->hide();
    ui->select_sub_1->setChecked(true);
    Setstyle_groupsub(0);
    Display_Protocol_to_user();

    connect(ui->sub1_step_1, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_2, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_3, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_4, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_5, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_6, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_7, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_8, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_9, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_10, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_11, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_12, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_13, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_14, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_15, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_15, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_16, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_17, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_18, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_19, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));
    connect(ui->sub1_step_20, SIGNAL(clicked(bool)), this, SLOT(checkSelected_STEP()));

    connect(ui->selected_deblock, SIGNAL(clicked(bool)), this, SLOT(checkSelectChemical()));
    connect(ui->selected_washing, SIGNAL(clicked(bool)), this, SLOT(checkSelectChemical()));
    connect(ui->selected_capping, SIGNAL(clicked(bool)), this, SLOT(checkSelectChemical()));
    connect(ui->selected_oxidation, SIGNAL(clicked(bool)), this, SLOT(checkSelectChemical()));
    connect(ui->selected_coupling, SIGNAL(clicked(bool)), this, SLOT(checkSelectChemical()));
    connect(ui->selected_oxidation2,SIGNAL(clicked(bool)), this, SLOT(checkSelectChemical()));
    connect(ui->selected_coupling_2,SIGNAL(clicked(bool)), this, SLOT(checkSelectChemical()));

    // control system manual IO FIRMWARE
    connect(ui->checkbx_control_V_1, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_2, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_3, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_4, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_5, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_6, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_7, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_8, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_9, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_10, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_11, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_12, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_13, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_14, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_15, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_16, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->checkbx_control_V_17, &QCheckBox::toggled, this, &SYNO24::ManualControlSystem);
    ui->btn_HighPushSV->setCheckable(true); // Đảm bảo nút có thể bật/tắt
    ui->btnLowPushSV->setCheckable(true); // Đảm bảo nút có thể bật/tắt
    ui->btnMediumPushSV->setCheckable(true); // Đảm bảo nút có thể bật/tắt
    ui->btnFAN->setCheckable(true); // Đảm bảo nút có thể bật/tắt
    ui->btnOpenAirNitor->setCheckable(true); // Đảm bảo nút có thể bật/tắt
    ui->btn_FanVacuumBox->setCheckable(true); // Đảm bảo nút có thể bật/tắt
    connect(ui->btn_HighPushSV, &QPushButton::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->btnLowPushSV, &QPushButton::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->btnMediumPushSV, &QPushButton::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->btnFAN, &QPushButton::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->btnOpenAirNitor, &QPushButton::toggled, this, &SYNO24::ManualControlSystem);
    connect(ui->btn_FanVacuumBox,&QPushButton::toggled, this, &SYNO24::ManualControlSystem );
    //ui->btn_Start->setChecked(true);
    // UPDATE LENGTH TO TABLE
    for(int row = 0; row < MAX_WELL_AMIDITE; row++)
    {
        QModelIndex index = model_table_well->index(row, 2,QModelIndex());
        model_table_well->setData(index,global_var.amidite_well[row].sequenceLength);
    }
    MonitorPlateUpdateUI(0);
    // tính nắng custom coupling2
    modelCoupling = new QStandardItemModel(0, 2, this);
    modelCoupling->setHorizontalHeaderLabels({"Coupling1", "Coupling2"});
    ui->tableView_GroupCoupling->setModel(modelCoupling);

    modelAmiditeSubSpecial = new QStandardItemModel(0, 1, this);
    ui->tableView_GroupSpecial->setModel(modelAmiditeSubSpecial);
    modelAmiditeSubSpecial->setHorizontalHeaderLabels({"Amidite in Special Sub"});
    //ui->tableView_GroupSpecial->setColumnWidth(1, 300);
    // Đặt độ rộng tự động cho tất cả các cột theo chiều rộng của bảng
    int totalWidth = ui->tableView_GroupSpecial->width();
    int columnCount = modelCoupling->columnCount();

    for (int i = 0; i < columnCount; ++i) {
        ui->tableView_GroupSpecial->setColumnWidth(i, totalWidth / columnCount);
    }

    // Hoặc đặt cột cuối cùng giãn ra để lấp đầy phần còn lại
    ui->tableView_GroupSpecial->horizontalHeader()->setStretchLastSection(true);
    //tableView_GroupSpecial

}
void SYNO24:: serialReceived()
{
    if (serialPort->isOpen()) // PORT OPEN ???
    {
        data_uart_received = serialPort->readAll();
        STM32_COM.u16_length_command_fw_rx = data_uart_received.size();
        if(STM32_COM.u16_length_command_fw_rx == LENGTH_COMMAND_RECEIVED) // check length data
        {
            STM32_COM.flag_process_command = true;// flag nay dung de check command da dung length hay chua - kiem tra ki tu cuoi cung
#ifdef MAIN_WINDOW_DEBUG
            QByteArray ba_as_hex_string = data_uart_received.toHex();
            //qDebug() << "Headder command Rx Receive : "<<ba_as_hex_string[0]<<ba_as_hex_string[1];
            // qDebug() << "CMD TX" <<STM32_COM.header_commmand << "CMD Rx : "<<ba_as_hex_string[0]<<ba_as_hex_string[1];
#endif
            if(STM32_COM.header_commmand == data_uart_received[0])
            {
                STM32_COM.flag_waitResponse_from_FW = true; // firmware phan hoi
                log_debug("FW Response OK");
            }
        }
        else
        {

#ifdef MAIN_WINDOW_DEBUG
            qDebug() << "lENGTH DATA ERROR: "<<STM32_COM.u16_length_command_fw_rx;
            ui->textEdit_status_update_fw->append("lENGTH DATA ERROR: " + QString::number(STM32_COM.u16_length_command_fw_rx));
#endif
        }
    }
    else
    {
#ifdef MAIN_WINDOW_DEBUG
        qDebug() << "PORT ERROR, PORT IS CLOSED";
#endif
    } // END PORT OPEN
    if(STM32_COM.flag_process_command) // CHECK FLAG PROCESS COMMAND
    {
        switch (data_uart_received[0])
        {
        case CMD_ASK_VENDOR_ID:
        {
#ifdef MAIN_WINDOW_DEBUG
            qDebug() << "FW Command CMD_ASK_VENDOR_ID";
#endif
            break;
        }
        case CMD_RECIVED_SETTING:
        {
            break;
        }
        case CMD_PRESURE_TESTING:
        {
            break;
        }
        case CMD_CONNECT_LINK:
        {
#ifdef MAIN_WINDOW_DEBUG
            qDebug() << "FW Command 0x01 CMD_CONNECT_LINK";
#endif
            break;
        }
        case CMD_START_OLIGO_STEP:
        {
#ifdef MAIN_WINDOW_DEBUG
            qDebug() << "FW Command 0x05 CMD_START_OLIGO_STEP";
#endif
            break;
        }
        case 0x03:
        {
#ifdef MAIN_WINDOW_DEBUG
            qDebug() << "FW Command 0x03 ";
#endif

            break;
        }
        case CMD_DATA_OLIGO:
        {
#ifdef MAIN_WINDOW_DEBUG
            qDebug() << "FW Command CMD_DATA_OLIGO 0x06 ";
#endif
            break;
        }
        case CMD_PRIMMING:
        {
#ifdef MAIN_WINDOW_DEBUG
            qDebug() << "FW  Command CMD_PRIMMING ";
#endif
            break;
        }
        case CMD_CALIBRATION_VALVE:
        {
#ifdef MAIN_WINDOW_DEBUG
            qDebug() << "FW  Command 0x64 - CMD_CALIBRATION_VALVE";
#endif
            break;
        }
        case CMD_MANUAL_RUN:
        {
#ifdef MAIN_WINDOW_DEBUG
            qDebug() << "FW  Command 0x66 - CMD_MANUAL_RUN";
#endif
            break;
        }
        case CMD_EXHAUSTED_CHEMICAL:
        {
            qDebug() << "FW  Command 0x66 - CMD_EXHAUSTED_CHEMICAL";
            break;
        }
        case CMD_RUN2HOME:
        {
            qDebug() << "FW  Command 0x02 - CMD_RUN2HOME";
            break;
        }
        case CMD_FIRMWARE_UPDATE_STATUS_AND_SENSOR:
        {
            global_var.status_and_sensor.u16_temperature.Byte[0] = data_uart_received[1];
            global_var.status_and_sensor.u16_temperature.Byte[1] = data_uart_received[2];
            global_var.status_and_sensor.u16_humidity.Byte[0] = data_uart_received[3];
            global_var.status_and_sensor.u16_humidity.Byte[1] = data_uart_received[4];
            global_var.status_and_sensor.f_humidity = global_var.status_and_sensor.u16_humidity.Data ;
            global_var.status_and_sensor.f_temperature = global_var.status_and_sensor.u16_temperature.Data;
            global_var.status_and_sensor.str_humidity.setNum(global_var.status_and_sensor.f_humidity / 100);
            global_var.status_and_sensor.str_temperature.setNum(global_var.status_and_sensor.f_temperature / 100);
            //global_var.status_and_sensor.str_humidity.setNum(2531 / 100);
            //global_var.status_and_sensor.str_temperature.setNum(2670 / 100);
            global_var.status_and_sensor.Pos_X.Byte[0] = data_uart_received[5];
            global_var.status_and_sensor.Pos_X.Byte[1] = data_uart_received[6];
            global_var.status_and_sensor.Pos_Y.Byte[0] = data_uart_received[7];
            global_var.status_and_sensor.Pos_Y.Byte[1] = data_uart_received[8];
            global_var.status_and_sensor.Pos_Z1.Byte[0] = data_uart_received[9];
            global_var.status_and_sensor.Pos_Z1.Byte[1] = data_uart_received[10];

            global_var.status_and_sensor.fPosX = (float)global_var.status_and_sensor.Pos_X.Data / 10;
            global_var.status_and_sensor.fPosY = (float)global_var.status_and_sensor.Pos_Y.Data / 10;
            global_var.status_and_sensor.fPosZ1 = (float)global_var.status_and_sensor.Pos_Z1.Data / 10;

            ui->spbx_x_stepper_work->setValue(global_var.status_and_sensor.fPosX);
            ui->spbx_y_stepper_work->setValue(global_var.status_and_sensor.fPosY);
            ui->spbx_z1_stepper_work->setValue(global_var.status_and_sensor.fPosZ1);

            ui->lbl_humidity->setText(global_var.status_and_sensor.str_humidity);
            ui->lbl_temperature->setText(global_var.status_and_sensor.str_temperature);
            global_var.status_and_sensor.flag_have_feedback_value = true;
            //qDebug() << " f_humidity" <<global_var.status_and_sensor.f_humidity;
            break;
        }
        case CMD_FEEDBACK_STATUS_RUN:
        {


            global_var.updateSTTRun2UI.u8_function_count = data_uart_received[1];
            global_var.updateSTTRun2UI.u8_subfunction =  data_uart_received[2];
            global_var.updateSTTRun2UI.u16tb_procs_time.Byte[0] = data_uart_received[3];
            global_var.updateSTTRun2UI.u16tb_procs_time.Byte[1] = data_uart_received[4];
            global_var.updateSTTRun2UI.u16tb_waitting_after_time.Byte[0] = data_uart_received[5];
            global_var.updateSTTRun2UI.u16tb_waitting_after_time.Byte[1] = data_uart_received[6];
            //            qDebug() << "CMD_FEEDBACK_STATUS_RUN" << "functionProgress" << global_var.updateSTTRun2UI.u8_function_count <<
            //                        "functionProgress sub" <<  global_var.updateSTTRun2UI.u8_subfunction <<
            //                        global_var.updateSTTRun2UI.u16tb_procs_time.Data << " : "<< global_var.updateSTTRun2UI.u16tb_waitting_after_time.Data * SCALE_LEVEL_WAITTIME;
            if( global_var.updateSTTRun2UI.u8_subfunction == WAIT_AFTERFILL)
            {
                UpdateUISTTRun(global_var.updateSTTRun2UI.u8_function_count);
            }
            else
            {
                UpdateUISTTRun(global_var.updateSTTRun2UI.u8_function_count + 1);
                global_var.updateSTTRun2UI.log_control_pressure = NAME_OPTION_PRESSURE[protocol_oligo.sub[global_var.updateSTTRun2UI.currentSub].step[global_var.updateSTTRun2UI.currentStep].control_pressure.u8_option_pressure[global_var.updateSTTRun2UI.u8_function_count]] +  " - "+
                        QString::number(protocol_oligo.sub[global_var.updateSTTRun2UI.currentSub].step[global_var.updateSTTRun2UI.currentStep].control_pressure.u16tb_procs_time[global_var.updateSTTRun2UI.u8_function_count].Data) + " | "
                        + QString::number(protocol_oligo.sub[global_var.updateSTTRun2UI.currentSub].step[global_var.updateSTTRun2UI.currentStep].control_pressure.u16tb_waitting_after_time[global_var.updateSTTRun2UI.u8_function_count].Data * SCALE_LEVEL_WAITTIME);
                pulseLabel(ui->lbl_stt_fw_fb, global_var.updateSTTRun2UI.log_control_pressure, "#00FF00");
            }

            if(global_var.updateSTTRun2UI.u8_subfunction == PUSHDOWN_FNC) // progress
            {
                m_progressPush->startCountdown(global_var.updateSTTRun2UI.u16tb_procs_time.Data);
            }
            else
            {
                if(global_var.updateSTTRun2UI.u8_subfunction == WAIT_FNC) // waitting proress
                {
                    m_progressWait->startCountdown(global_var.updateSTTRun2UI.u16tb_waitting_after_time.Data * SCALE_LEVEL_WAITTIME);
                }
                else // waiting after fill
                {
                    m_progressWait->startCountdown(global_var.updateSTTRun2UI.u16tb_waitting_after_time.Data);
                }
            }

            break;
        }
        case CMD_CONTROL_AIR_START:
        {
            qDebug() << "CMD_CONTROL_AIR_START";
            break;
        }
        case CMD_OX_SENQUENCE:
        {
            qDebug() << "CMD_OX_SENQUENCE";
            break;
        }
        case CMD_CONTROL_SYNCHRONIZE_IO:
        {
            qDebug() << "CMD_CONTROL_SYNCHRONIZE_IO";
            break;
        }
        default:
        {
            qDebug() << "FW  Command not find on table";
            ui->textEdit_status_update_fw->append("ERROR COMMAND NOT FIND");
            break;
        }
        }
        STM32_COM.flag_process_command = false;
    }
    //data_uart_received.clear();
}
/*
 *Scan Serial Port
*/
void SYNO24::on_btn_scanport_released()
{
    ui->cbx_comport->clear();
    const auto infos = QSerialPortInfo::availablePorts();
    // Scan serial Port
    // Add Name combobox COMPORT
    for (const QSerialPortInfo &info : infos)
    {
        ui->cbx_comport->addItem(info.portName());
    }
}

/**
 *
 *
 * */
void SYNO24:: scanAndSelectPort(const QString &OldPort) {
    ui->cbx_comport->clear(); // Xóa các mục trong combobox
    const auto infos = QSerialPortInfo::availablePorts(); // Lấy danh sách các cổng COM
    int matchingIndex = -1; // Khởi tạo chỉ số cho cổng khớp với OldPort
    int currentIndex = 0;   // Biến đếm chỉ số cho combobox

    for (const QSerialPortInfo &info : infos) {
        QString portName = info.portName();
        ui->cbx_comport->addItem(portName); // Thêm cổng vào combobox
        // Debug thông tin cổng COM
        qDebug() << "Port Name: " << info.portName();
        qDebug() << "Description: " << info.description();
        qDebug() << "Manufacturer: " << info.manufacturer();
        qDebug() << "Serial Number: " << info.serialNumber();
        qDebug() << "Vendor Identifier: " << info.vendorIdentifier();
        qDebug() << "Product Identifier: " << info.productIdentifier();

        // Kiểm tra nếu port khớp với OldPort
        if (portName == OldPort) {
            matchingIndex = currentIndex; // Lưu lại chỉ số của port khớp
        }
        currentIndex++; // Tăng chỉ số
    }

    // Nếu tìm thấy cổng khớp, chọn nó trong combobox
    if (matchingIndex != -1) {
        ui->cbx_comport->setCurrentIndex(matchingIndex);
        qDebug() << "[INFO] OldPort found: " << OldPort << " at index: " << matchingIndex;
    } else {
        qDebug() << "[WARNING] OldPort not found: " << OldPort;
    }
}
/*
 * Function Open Serial port
*/
void SYNO24:: fnc_openSerialPort()
{
    //QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    //Command_send[0] = CMD_ASK_VENDOR_ID; // FILL to well
    if(ui->cbx_comport->currentText().isEmpty())
    {
        QMessageBox::warning(this,"Warning", "Don't have ComPort, Check your connection!");
    }
    else
    {
        serialPort->setPortName(ui->cbx_comport->currentText());
        serialPort->setBaudRate(BAUDRATE_UART);
        serialPort->setDataBits(QSerialPort::Data8);
        serialPort->setParity(QSerialPort::NoParity);
        serialPort->setStopBits(QSerialPort::OneStop);
        serialPort->setFlowControl(QSerialPort::NoFlowControl);
        if(STM32_COM.flag_connecttion == false)
        {
            serialPort->setReadBufferSize(LENGTH_COMMAND_RECEIVED);
            if (serialPort->open(QIODevice::ReadWrite))
            {
                ui->btn_ConnectPort->setText("Connected");
                ui->btn_ConnectPort->setCheckable(true);
                ui->btn_ConnectPort->setChecked(true);
                //ui->btn_ConnectPort->setStyleSheet("background-color: rgb(255, 0, 0)");
                if(ASK_VENDOR_ID())
                {
                    STM32_COM.flag_connecttion = true;
                    send_setting();
                    STM32_COM.currentPort = ui->cbx_comport->currentText();
                    //writeSettings();
                    delay_ui.delay_ms(500);
                }
                else
                {
                    // 29-07-2023 thêm dòng này bắt đầu giao tiếp với firmware
                    // fimmrware gửi lệnh thành công thì mới tiếp tục chạy được
                    serialPort->close();
                    //ui->btn_ConnectPort->setCheckable(false);
                    ui->btn_ConnectPort->setChecked(false);
                    ui->btn_ConnectPort->setCheckable(false);
                    QMessageBox::critical(this, tr("Error"), "NO DEVICE SYNO24X CONNECT");
                }
#ifdef MAIN_WINDOW_DEBUG
                qDebug()<< "Serial Connected";
#endif
            }
            else
            {
                ui->btn_ConnectPort->setCheckable(false);
                ui->btn_ConnectPort->setChecked(false);
                ui->btn_ConnectPort->setText("Disconnect");
                QMessageBox::critical(this, tr("Error"), "CAN'T CONNECT PORT");
            }
        }
        else
        {
            serialPort->close();
            ui->btn_ConnectPort->setText("Disconnect");
            ui->btn_ConnectPort->setCheckable(false);
            ui->btn_ConnectPort->setChecked(false);
            STM32_COM.flag_connecttion = false;
        }
    }
}

void SYNO24::onSerialPortError(QSerialPort::SerialPortError error)
{
    static QSerialPort::SerialPortError previousError;
    if (error != QSerialPort::NoError && error != previousError) {
        previousError = error;
        //ui->txtConsole->appendPlainText(tr("Serial port error ") + QString::number(error) + ": " + m_serialPort.errorString());
        if (serialPort->isOpen()) {
            serialPort->close();
            ui->btn_ConnectPort->setChecked(false);
            ui->btn_ConnectPort->setCheckable(false);
            STM32_COM.flag_connecttion = false;
            updateControlsState(); // update control state update ui
            QMessageBox::critical(this, tr("Error"), serialPort->errorString());
        }
    }
}

void SYNO24:: on_btn_start_update_fw_released()
{

}

/*
 * Primming control START
*/
void SYNO24::on_btn_Primming_released()
{
#ifdef MAIN_WINDOW_DEBUG
    qDebug()<< "primming_control" ;
#endif
    // Start Primming control
    double db_time_primming = ui->db_spbox_time_primming->value();
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    Command_send[0] = CMD_PRIMMING; // FILL to well
    global_var.primming_control.valve[0] = ui->checkbx_valve_1->isChecked();
    global_var.primming_control.valve[1] = ui->checkbx_valve_2->isChecked();
    global_var.primming_control.valve[2] = ui->checkbx_valve_3->isChecked();
    global_var.primming_control.valve[3] = ui->checkbx_valve_4->isChecked();
    global_var.primming_control.valve[4] = ui->checkbx_valve_5->isChecked();
    global_var.primming_control.valve[5] = ui->checkbx_valve_6->isChecked();
    global_var.primming_control.valve[6] = ui->checkbx_valve_7->isChecked();
    global_var.primming_control.valve[7] = ui->checkbx_valve_8->isChecked();
    global_var.primming_control.valve[8] = ui->checkbx_valve_9->isChecked();
    global_var.primming_control.valve[9] = ui->checkbx_valve_10->isChecked();
    global_var.primming_control.valve[10] = ui->checkbx_valve_11->isChecked();
    global_var.primming_control.valve[11] = ui->checkbx_valve_12->isChecked();
    global_var.primming_control.valve[12] = ui->checkbx_valve_13->isChecked();
    global_var.primming_control.valve[13] = ui->checkbx_valve_14->isChecked();
    global_var.primming_control.valve[14] = ui->checkbx_valve_15->isChecked();
    global_var.primming_control.valve[15] = ui->checkbx_valve_16->isChecked();
    global_var.primming_control.valve[16] = ui->checkbx_valve_17->isChecked();
    //double db_volume2sub = 0;
    //uint16_t u16_volume2sub;
    //    for(uint8_t u8_idx = 0; u8_idx < MAX_NUMBER_VALVE; u8_idx++)
    //    {
    //        if(global_var.primming_control.valve[u8_idx] == true)
    //        {
    //            db_volume2sub = (double)(db_time_primming * 10000 - volume.valve[u8_idx].b) /  volume.valve[u8_idx].a;
    //            u16_volume2sub = static_cast<uint16_t>(db_volume2sub);
    //            volume.sub_volume(u8_idx, u16_volume2sub);
    //#ifdef MAIN_WINDOW_DEBUG
    //            qDebug()<< "valve " << u8_idx << "volume :"<< u16_volume2sub;
    //#endif
    //        }
    //    }
    global_var.primming_control.u8_time_primming_control = db_time_primming * 10;
    for(uint8_t idx_valve = 0; idx_valve < MAX_NUMBER_VALVE; idx_valve++)
    {
        Command_send[idx_valve + 1] = global_var.primming_control.valve[idx_valve];
#ifdef MAIN_WINDOW_DEBUG
        qDebug()<< "primming" << idx_valve << Command_send[idx_valve + 1];
#endif
    }
    Command_send[20] = global_var.primming_control.u8_time_primming_control;
    STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 5000);
}

/*
 * check all valve for primming
*/

void SYNO24::on_checkbx_sellect_all_toggled(bool checked)
{
    ui->checkbx_valve_1->setChecked(checked);
    ui->checkbx_valve_2->setChecked(checked);
    ui->checkbx_valve_3->setChecked(checked);
    ui->checkbx_valve_4->setChecked(checked);
    ui->checkbx_valve_5->setChecked(checked);
    ui->checkbx_valve_6->setChecked(checked);
    ui->checkbx_valve_7->setChecked(checked);
    ui->checkbx_valve_8->setChecked(checked);
    ui->checkbx_valve_9->setChecked(checked);
    ui->checkbx_valve_10->setChecked(checked);
    ui->checkbx_valve_11->setChecked(checked);
    ui->checkbx_valve_12->setChecked(checked);
    ui->checkbx_valve_13->setChecked(checked);
    ui->checkbx_valve_14->setChecked(checked);
    ui->checkbx_valve_15->setChecked(checked);
    ui->checkbx_valve_16->setChecked(checked);
    ui->checkbx_valve_17->setChecked(checked);
}


void SYNO24::on_btn_calib_step1_released()
{
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    cablib_valve.u32fb_time_primming_calib.Data = ui->db_Spbox_calib_time_1->value() * 1000;
    Command_send[0] = CMD_CALIBRATION_VALVE; // FILL to well
    Command_send[1] = ui->cbx_valve_selected->currentIndex(); // CHON VALVE
    Command_send[2] = cablib_valve.u32fb_time_primming_calib.Byte[0];
    Command_send[3] = cablib_valve.u32fb_time_primming_calib.Byte[1];
    Command_send[4] = cablib_valve.u32fb_time_primming_calib.Byte[2];
    Command_send[5] = cablib_valve.u32fb_time_primming_calib.Byte[3];
    STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 5000);
}


void SYNO24::on_btn_calib_step2_released()
{
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    cablib_valve.u32fb_time_primming_calib.Data = ui->db_Spbox_calib_time_2->value()* 1000;
    Command_send[0] = CMD_CALIBRATION_VALVE; // FILL to well
    Command_send[1] = ui->cbx_valve_selected->currentIndex();// CHON VALVE
    Command_send[2] = cablib_valve.u32fb_time_primming_calib.Byte[0];
    Command_send[3] = cablib_valve.u32fb_time_primming_calib.Byte[1];
    Command_send[4] = cablib_valve.u32fb_time_primming_calib.Byte[2];
    Command_send[5] = cablib_valve.u32fb_time_primming_calib.Byte[3];
    STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 5000);
}
void SYNO24::on_btn_Calib_released()
{
    int16_t a1 = 0, b1 = 0 , c1 = 0, a2 = 0, b2 = 0, c2 = 0;
    quint8 u8_current_valve_selected = 0;
    c1 = ui->db_Spbox_calib_time_1->value() * 1000; // lay data tu ui
    b1 = 1;
    a1 = ui->spbox_calib_V1->value();
    c2 = ui->db_Spbox_calib_time_2->value()* 1000; // lay data tu ui
    b2 = 1;
    a2 = ui->spbox_calib_V2->value();
    float Delta, Delta_t, Delta_v;
    Delta = a1 * b2 - a2 * b1;
    Delta_t = c1 * b2 - c2 * b1;
    Delta_v = a1 * c2 - a2 * c1;
    if(Delta == 0)
    {
        QMessageBox::warning(this,"Error Calibration", "Please check your valve!");
#ifdef MAIN_WINDOW_DEBUG
        qDebug()<< "Error Calibration";
#endif
    }
    else
    {
        u8_current_valve_selected = ui->cbx_valve_selected->currentIndex();
        //global_var.valve_setting[u8_current_valve_selected].a = Delta_t / Delta;
        //global_var.valve_setting[u8_current_valve_selected].b = Delta_v / Delta;
        volume.valve[u8_current_valve_selected].a = Delta_t / Delta;
        volume.valve[u8_current_valve_selected].b = Delta_v / Delta;

        qDebug()<< "current index" << u8_current_valve_selected<< "a" << volume.valve[u8_current_valve_selected].a<< "b" << volume.valve[u8_current_valve_selected].b;
#ifdef MAIN_WINDOW_DEBUG
        qDebug()<< "current index" << u8_current_valve_selected<< "a" << volume.valve[u8_current_valve_selected].a<< "b" << volume.valve[u8_current_valve_selected].b;
#endif
        //fnc.save_parameter_valve_calib(&global_var, filemanager.valveSetting_Path);
        volume.save_parameter_valve();
        send_setting();
    }
}


void SYNO24::on_btn_fill_50ul_released()
{
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    quint8 u8_current_valve_selected = ui->cbx_valve_selected->currentIndex();
    cablib_valve.u32fb_time_primming_calib.Data = VOLUME_FILL_50UL * volume.valve[u8_current_valve_selected].a + volume.valve[u8_current_valve_selected].b;
    qDebug()<< "Time fill" << cablib_valve.u32fb_time_primming_calib.Data;
    Command_send[0] = CMD_CALIBRATION_VALVE; // FILL
    Command_send[1] = ui->cbx_valve_selected->currentIndex();
    Command_send[2] = cablib_valve.u32fb_time_primming_calib.Byte[0];
    Command_send[3] = cablib_valve.u32fb_time_primming_calib.Byte[1];
    Command_send[4] = cablib_valve.u32fb_time_primming_calib.Byte[2];
    Command_send[5] = cablib_valve.u32fb_time_primming_calib.Byte[3];
    //fnc.save_parameter_valve_calib(&global_var, filemanager.valveSetting_Path);
    volume.save_parameter_valve();
    STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 5000);
}
//  ==========================================================================================================================================================
bool SYNO24::save_str_amidite_toJson(global_var_t* global_var, QString Json_Path)
{
    QFile file(Json_Path); // path file
    if (!(file.open(QIODevice::WriteOnly)))
    {
#ifdef PRINT_DEBUG
        qDebug()<<"Error open file"<< file.errorString();
#endif
    }
    QTextStream config_file(&file);
    QModelIndex index ;
    for(int i = 0; i < MAX_WELL_AMIDITE; i++)
    {
        /*
        index = model_table_well->index(i, 3, QModelIndex());
        global_var->amidite_well[i].string_sequence = ui->tableView->model()->data(index).toString();
        */
        // GiangLH 05-11-2024 xóa khoảng trắng từ chuỗi amidite
        // nếu không cần xóa kí tự thì mở comment dòng trên
        // Lấy dữ liệu từ model_table_well
        QModelIndex index = model_table_well->index(i, 3, QModelIndex());
        QString sequence = ui->tableView->model()->data(index).toString();

        // Xóa khoảng trắng trong chuỗi
        sequence.replace(" ", ""); // Loại bỏ tất cả khoảng trắng

        // Cập nhật lại chuỗi đã loại bỏ khoảng trắng vào biến global
        global_var->amidite_well[i].string_sequence = sequence;

        // Set lại chuỗi đã loại bỏ khoảng trắng vào tableView
        model_table_well->setData(index, sequence, Qt::EditRole);
        // kết thúc xóa kí tự khoảng trắng
        // ================================================================
        //  lấy tên của giếng
        index = model_table_well->index(i, 1, QModelIndex());
        global_var->amidite_well[i].string_name =  ui->tableView->model()->data(index).toString();
        //std::reverse(global_var->data_amidite.strcode_well[i].begin(), global_var->data_amidite.strcode_well[i].end());
#ifdef MAIN_WINDOW_DEBUG
        //qDebug()<< global_var->amidite_well[i].string_sequence;
        //qDebug()<< "Length Data" << global_var->amidite_well[i].string_sequence.length();
#endif
    }
    QJsonObject jsonOb_well[MAX_WELL_AMIDITE];
    // Valve
    for(int element = 0; element < MAX_WELL_AMIDITE ; element++)
    {
        jsonOb_well[element].insert("Sequence",global_var->amidite_well[element].string_sequence );
        jsonOb_well[element].insert("Name",  global_var->amidite_well[element].string_name);
    }
    QJsonObject content;
    QString  str2  = "well_";
    for(int i = 0; i < MAX_WELL_AMIDITE ; i++)
    {
        QString  str_name_obj = str2 +  QString::number(i);
        content.insert( str_name_obj, jsonOb_well[i]);
    }
    QJsonDocument document;
    document.setObject( content );
    QByteArray bytes = document.toJson( QJsonDocument::Indented );
    QFile File_Save(Json_Path); // path file
    if( File_Save.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        QTextStream iStream( &File_Save );
        iStream.setCodec( "utf-8" );
        iStream << bytes;
        File_Save.close();
    }
    else
    {
#ifdef MAIN_WINDOW_DEBUG
        qDebug()<< " open file save data well";
#endif
    }
    return true;
}

void SYNO24::on_btn_save_sequence_released()
{
    save_str_amidite_toJson(&global_var, filemanager.amidite_sequence_Path);
    //fnc.getData2AmiditeProcess(&global_var);
    // Gọi hàm validateAllWells
    bool allValid = amidite_.validateAllWells(&global_var);
    // kiểm tra sequence 25-04-2025
    if (allValid) {
        qDebug() << "All wells are valid.";
    } else {
        qDebug() << "Some wells have errors.";
        return;
    }
    amidite_.amiditeSyno24XGetSequence(&global_var);
    // UPDATE LENGTH TO TABLE
    for(int row = 0; row < MAX_WELL_AMIDITE; row++)
    {
        QModelIndex index = model_table_well->index(row, 2,QModelIndex());
        model_table_well->setData(index,global_var.amidite_well[row].sequenceLength);
    }
    Display_Protocol_to_user();
    // UPDATE LENGTH TO TABLE
    // syno 24X cần tính lại length sequence
    // tạm thời khoá lại -tính lại đi
    //    for(int row = 0; row < MAX_WELL_AMIDITE; row++)
    //    {
    //        QModelIndex index = model_table_well->index(row, 2,QModelIndex());
    //        model_table_well->setData(index,global_var.amidite_well[row].string_sequence.length());
    //    }
    calculator_volume_and_process_UI(); // KHOÁ ĐẾM HOÁ CHẤT 23.08 // 28-02 mở lại để tính
    ///     // Giả sử số sequence được truyền vào từ người dùng
    // 10-02-2025 hộp thoại thông báo max sequence amidite
    int number = global_var.signal_status_oligo.u16_max_sequence_amidite_setting; // Bạn có thể thay đổi giá trị này hoặc lấy từ input

    //=========================================================
    MonitorPlateUpdateUI(0);
    // Tạo thông báo với nội dung "max sequence amidite: number"
    QString message = QString("Max sequence Amidite: %1").arg(number);

    // Hiển thị hộp thoại thông báo
    QMessageBox::information(nullptr, "information", message);
}


void SYNO24::on_btn_new_protocol_released()
{
    QString saveFileName = QFileDialog::getSaveFileName(this,
                                                        tr("Save Json File"),
                                                        QString(),
                                                        tr("JSON (*.json)"));
    QFile File_Save(saveFileName);
    if( File_Save.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        File_Save.close();
        filemanager.protocol_Path = saveFileName;
        ui->lineEdit_path_protocol->setText(saveFileName);
    }
    else
    {
        QMessageBox::warning(this, "Warning", " Can't Create or Read this file, Please try again!!!");
    }
}


void SYNO24::on_btn_open_protocol_released()
{
    filemanager.protocol_Path = QFileDialog::getOpenFileName(this, "Open file", filemanager.protocol_Dir,"json(*.json)");
    if(filemanager.protocol_Dir.isEmpty())
    {
        QMessageBox::warning(this, "Warning", " File Path protocol Empty, Please try choise again!!!");
    }
    else
    {
        ui->lineEdit_path_protocol->setText(filemanager.protocol_Path);
        // 14/03/2023 mở comment này để chạy load giá trị lên ui
        fnc.read_protocol_fromJson(&protocol_oligo, filemanager.protocol_Path);
        //fnc.getData2AmiditeProcess(&global_var);
        amidite_.amiditeSyno24XGetSequence(&global_var);
        load_protocol_to_ui(0, 0); // load ui khi thay co thay doi
        ui->select_sub_1->setChecked(true);
        Setstyle_groupsub(0);
    }
    Display_Protocol_to_user();
    calculator_volume_and_process_UI();
}


void SYNO24::on_btn_clear_data_step_released()
{
    //-------------------------------------------------------------------------------------------
    //ui->spbox_loop_page->setValue()
    ui->spbox_volume->setValue(0);
    ui->selected_deblock->setChecked(true);
    ui->spbox_wait_after_fill->setValue(0);
    // fast vacuum
    //ui->spbx_time_fast_vacuum->setValue(0);
    // STATE 2 START
    ui->cbx_option_pressure_state_2->setCurrentIndex(0);
    ui->spbx_time_process_state_2->setValue( 0);
    ui->spbox_wait_state_2->setValue(0);

    ui->cbx_option_pressure_state_3->setCurrentIndex(0);
    ui->spbx_time_process_state_3->setValue(0);
    ui->spbox_wait_state_3->setValue(0);

    ui->cbx_option_pressure_state_4->setCurrentIndex(0);
    ui->spbx_time_process_state_4->setValue( 0);
    ui->spbox_wait_state_4->setValue(0);

    ui->cbx_option_pressure_state_5->setCurrentIndex(0);
    ui->spbx_time_process_state_5->setValue( 0);
    ui->spbox_wait_state_5->setValue(0);

    ui->cbx_option_pressure_state_6->setCurrentIndex(0);
    ui->spbx_time_process_state_6->setValue(0);
    ui->spbox_wait_state_6->setValue(0);

    ui->cbx_option_pressure_state_7->setCurrentIndex(0);
    ui->spbx_time_process_state_7->setValue(0);
    ui->spbox_wait_state_7->setValue(0);

    ui->cbx_option_pressure_state_8->setCurrentIndex(0);
    ui->spbx_time_process_state_8->setValue(0);
    ui->spbox_wait_state_8->setValue(0);

    ui->cbx_option_pressure_state_9->setCurrentIndex(0);
    ui->spbx_time_process_state_9->setValue(0);
    ui->spbox_wait_state_9->setValue(0);

    ui->cbx_option_pressure_state_10->setCurrentIndex(0);

    ui->spbx_time_process_state_10->setValue(0);
    ui->spbox_wait_state_10->setValue(0);

    ui->cbx_option_pressure_state_11->setCurrentIndex(0);
    ui->spbx_time_process_state_11->setValue(0);
    ui->spbox_wait_state_11->setValue(0);
}




void SYNO24::on_btn_savecurrent_step_released()
{
    quint8 u8_current_sub = data_ui.u8_current_SUB_edit_ui;
    quint8 u8_current_step = data_ui.u8_current_STEP_edit_ui;

    protocol_oligo.sub[u8_current_sub].u8_number_base_on_sub = spinbxNumberBaseOnSub->value();
    protocol_oligo.sub[u8_current_sub].u8_number_step_on_base = spinbxNumberStepOnBase->value();
    checkSelectChemical();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.u8_first_type_chemical = protocol_oligo.u8_step_cycle;
    protocol_oligo.sub[u8_current_sub].douple_coupling_option = cbx_option_coupling_sub->currentIndex();

    // ==========================================================================================
    //protocol_oligo.u16_scale_volume.Data = ui->spbx_scale_volume->value();
    //protocol_oligo.u16_scale_time.Data = ui->spbx_scale_time->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.u16tb_Volume.Data = ui->spbox_volume->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.u16tb_wait_after_fill.Data = ui->spbox_wait_after_fill->value();

    // function 1 bom hoa chat
    checkSelectChemical();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.u8_first_type_chemical = protocol_oligo.u8_step_cycle;
    // ==========================================================================================
    //protocol_oligo.u16_scale_volume.Data = ui->spbx_scale_volume->value();
    //protocol_oligo.u16_scale_time.Data = ui->spbx_scale_time->value();
    qDebug()<< " u8_current_step" << u8_current_step;
    qDebug()<< " step sellected" << protocol_oligo.u8_step_cycle;
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.u16tb_Volume.Data = ui->spbox_volume->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.u16tb_wait_after_fill.Data = ui->spbox_wait_after_fill->value();
    // funtion mix chemical
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.mix_funtion.u8_type_chemical[0] = ui->cbx_type_chemical_mix_1->currentIndex();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.mix_funtion.u8_type_chemical[1] = ui->cbx_type_chemical_mix_2->currentIndex();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.mix_funtion.u8_type_chemical[2] = ui->cbx_type_chemical_mix_3->currentIndex();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.mix_funtion.u16tb_Volume[0].Data = ui->spbx_volume_mix_1->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.mix_funtion.u16tb_Volume[1].Data = ui->spbx_volume_mix_2->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.mix_funtion.u16tb_Volume[2].Data = ui->spbx_volume_mix_3->value();
    // Funtion 2
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[0] = ui->cbx_option_pressure_state_2->currentIndex();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[1] = ui->cbx_option_pressure_state_3->currentIndex();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[2] = ui->cbx_option_pressure_state_4->currentIndex();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[3] = ui->cbx_option_pressure_state_5->currentIndex();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[4] = ui->cbx_option_pressure_state_6->currentIndex();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[5] = ui->cbx_option_pressure_state_7->currentIndex();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[6] = ui->cbx_option_pressure_state_8->currentIndex();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[7] = ui->cbx_option_pressure_state_9->currentIndex();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[8] = ui->cbx_option_pressure_state_10->currentIndex();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[9] = ui->cbx_option_pressure_state_11->currentIndex();

    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[0].Data = ui->spbox_wait_state_2->value() / SCALE_LEVEL_WAITTIME;
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[1].Data = ui->spbox_wait_state_3->value() / SCALE_LEVEL_WAITTIME;
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[2].Data  = ui->spbox_wait_state_4->value() / SCALE_LEVEL_WAITTIME;
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[3].Data  = ui->spbox_wait_state_5->value() / SCALE_LEVEL_WAITTIME;
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[4].Data  = ui->spbox_wait_state_6->value() / SCALE_LEVEL_WAITTIME;
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[5].Data = ui->spbox_wait_state_7->value() / SCALE_LEVEL_WAITTIME;
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[6].Data  = ui->spbox_wait_state_8->value() / SCALE_LEVEL_WAITTIME;
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[7].Data  = ui->spbox_wait_state_9->value() / SCALE_LEVEL_WAITTIME;
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[8].Data = ui->spbox_wait_state_10->value() / SCALE_LEVEL_WAITTIME;
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[9].Data  = ui->spbox_wait_state_11->value() / SCALE_LEVEL_WAITTIME;

    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[0].Data = ui->spbx_time_process_state_2->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[1].Data= ui->spbx_time_process_state_3->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[2].Data = ui->spbx_time_process_state_4->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[3].Data = ui->spbx_time_process_state_5->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[4].Data = ui->spbx_time_process_state_6->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[5].Data = ui->spbx_time_process_state_7->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[6].Data = ui->spbx_time_process_state_8->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[7].Data= ui->spbx_time_process_state_9->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[8].Data = ui->spbx_time_process_state_10->value();
    protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[9].Data = ui->spbx_time_process_state_11->value();
    //fnc.save_protocol_toJson(&protocol_oligo, path_file_manager.protocol_path);
    Display_Protocol_to_user();
    calculator_volume_and_process_UI(); // KHOÁ ĐẾM HOÁ CHẤT 23.08
    // Hiển thị thông báo lưu thành công
    QMessageBox::information(nullptr, "Save to Word", "Save successful.");
}



void SYNO24::on_btn_save_history_released()
{
    // Khởi tạo một đối tượng Word
    QAxObject word("Word.Application");
    // Tạo một tài liệu mới
    QAxObject *documents = word.querySubObject("Documents");
    QAxObject *document = documents->querySubObject("Add()");

    // Lấy văn bản từ QLineEdit
    QString text = ui->textEdit_oligo_history_log->toPlainText();

    // Chèn văn bản vào tài liệu Word
    QAxObject *selection = word.querySubObject("Selection");
    selection->dynamicCall("TypeText(QString)", text);

    // Mở hộp thoại lưu tệp tin
    QString fileName = QFileDialog::getSaveFileName(nullptr, "Save to Word", "", "*.docx");

    // Lưu tài liệu vào tệp tin
    document->dynamicCall("SaveAs(const QString&)", fileName);

    // Đóng tài liệu và Word
    document->dynamicCall("Close()");
    word.dynamicCall("Quit()");

    // Hiển thị thông báo lưu thành công
    QMessageBox::information(nullptr, "Save to Word", "Save successful.");
}

void SYNO24::load_protocol_to_ui(quint8 u8_current_sub, quint8 u8_current_step)
{
    //  ui->spbox_loop_page->setValue()
    spinbxNumberBaseOnSub->setValue(protocol_oligo.sub[u8_current_sub].u8_number_base_on_sub);
    spinbxNumberStepOnBase->setValue(protocol_oligo.sub[u8_current_sub].u8_number_step_on_base);
    cbx_option_coupling_sub->setCurrentIndex(protocol_oligo.sub[u8_current_sub].douple_coupling_option);
    //-------------------------------------------------------------------------------------------
    ui->spbox_number_sub->setValue(protocol_oligo.u8_number_sub);
    ui->spbox_volume->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.u16tb_Volume.Data);
    setUI_FirstChemical(protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.u8_first_type_chemical);
    //ui->cbx_type_sulphite->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.u8_first_type_chemical);
    //on_cbx_type_sulphite_currentIndexChanged(protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.u8_first_type_chemical);

    ui->spbox_wait_after_fill->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.u16tb_wait_after_fill.Data);
    // mix function
    ui->spbx_volume_mix_1->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.mix_funtion.u16tb_Volume[0].Data);
    ui->spbx_volume_mix_2->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.mix_funtion.u16tb_Volume[1].Data);
    ui->spbx_volume_mix_3->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.mix_funtion.u16tb_Volume[2].Data);
    ui->cbx_type_chemical_mix_1->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.mix_funtion.u8_type_chemical[0]);
    ui->cbx_type_chemical_mix_2->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.mix_funtion.u8_type_chemical[1]);
    ui->cbx_type_chemical_mix_3->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].fill_chemical.mix_funtion.u8_type_chemical[2]);
    // CONTROL PRESSURE
    ui->cbx_option_pressure_state_2->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[0]);
    ui->spbx_time_process_state_2->setValue( protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[0].Data);
    ui->spbox_wait_state_2->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[0].Data * SCALE_LEVEL_WAITTIME); // 14-04-2025 sua thanh *10 vi da chia cho 10

    ui->cbx_option_pressure_state_3->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[1]);
    ui->spbx_time_process_state_3->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[1].Data);
    ui->spbox_wait_state_3->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[1].Data * SCALE_LEVEL_WAITTIME);

    ui->cbx_option_pressure_state_4->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[2]);
    ui->spbx_time_process_state_4->setValue( protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[2].Data);
    ui->spbox_wait_state_4->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[2].Data * SCALE_LEVEL_WAITTIME);// 14-04-2025

    ui->cbx_option_pressure_state_5->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[3]);
    ui->spbx_time_process_state_5->setValue( protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[3].Data);
    ui->spbox_wait_state_5->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[3].Data * SCALE_LEVEL_WAITTIME);// 14-04-2025

    ui->cbx_option_pressure_state_6->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[4]);
    ui->spbx_time_process_state_6->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[4].Data);
    ui->spbox_wait_state_6->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[4].Data * SCALE_LEVEL_WAITTIME);

    ui->cbx_option_pressure_state_7->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[5]);
    ui->spbx_time_process_state_7->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[5].Data);
    ui->spbox_wait_state_7->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[5].Data * SCALE_LEVEL_WAITTIME);

    ui->cbx_option_pressure_state_8->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[6]);
    ui->spbx_time_process_state_8->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[6].Data);
    ui->spbox_wait_state_8->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[6].Data * SCALE_LEVEL_WAITTIME);

    ui->cbx_option_pressure_state_9->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[7]);
    ui->spbx_time_process_state_9->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[7].Data);
    ui->spbox_wait_state_9->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[7].Data * SCALE_LEVEL_WAITTIME);

    ui->cbx_option_pressure_state_10->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[8]);
    ui->spbx_time_process_state_10->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[8].Data);
    ui->spbox_wait_state_10->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[8].Data * SCALE_LEVEL_WAITTIME);

    ui->cbx_option_pressure_state_11->setCurrentIndex(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u8_option_pressure[9]);
    ui->spbx_time_process_state_11->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_procs_time[9].Data);
    ui->spbox_wait_state_11->setValue(protocol_oligo.sub[u8_current_sub].step[u8_current_step].control_pressure.u16tb_waitting_after_time[9].Data * SCALE_LEVEL_WAITTIME);
}

void SYNO24::on_btn_start_synthetic_released()
{
    on_btn_start_synthetic_released_optimized();
    /*
    // tạo tên cho file history file
    QFileInfo fileInfo(filemanager.protocol_Path);
    // Lấy tên file không bao gồm phần mở rộng history log add on 02-04-2025
    QString baseName = fileInfo.baseName();
    qDebug() << "Log history Run Path:" << baseName;
    LogHistory LogHistoryRun(baseName);

    timer_update_humidity_tempareture.stop();
    if( syno24_machine.getAutoState() == State::STOPED)
    {
        ui->textEdit_oligo_history_log->clear();
        //.signal_status_oligo.u8_FLAG_RUN_OLIGO =  START_PROCESS_SYNTHETIC_OLIGO; // bat dau start chu trình oligo
        syno24_machine.setAutoState(State::RUNNING);
        global_var.status_and_sensor.u16tb_humidity_Preset.Data = ui->spinbx_humidity_value->value();
        float hummmidity = global_var.status_and_sensor.f_humidity / 100;
        global_var.status_and_sensor.flag_enable_auto_control_air_Nito = ui->checkbox_wait_humidity->isChecked();
        get_sensor_humidity_tempareture();
        if((global_var.status_and_sensor.flag_enable_auto_control_air_Nito == true) && (global_var.status_and_sensor.flag_have_feedback_value == true))
        {
            //ui->textEdit_oligo_history_log->append("Waitting for humidity to decrease");
            log_terminal("Waitting for humidity to decrease", Info);
            wait_humidity();
            //hummmidity = 30;
            while(hummmidity >= global_var.status_and_sensor.u16tb_humidity_Preset.Data)
            {
                hummmidity = global_var.status_and_sensor.f_humidity / 100;
                //hummmidity = 30;
                delay_ui.delay_ms(500);
                get_sensor_humidity_tempareture();
                qDebug()<< "hummmidity" << hummmidity;
                qDebug()<<"preset_value_hummmidity"<< global_var.status_and_sensor.u16tb_humidity_Preset.Data;
                if(syno24_machine.getAutoState() == State::STOPED)
                    break;
            }
            log_terminal("Finish Wait for humidity", Info);
            //ui->textEdit_oligo_history_log->append("Finish Wait for humidity");
        }
    }
    if( syno24_machine.getAutoState() == State::RUNNING && (STM32_COM.flag_connecttion == true))
    {
        LogHistoryRun.appendToLog(ui->textEdit_list_task_protocol->toPlainText());
        qDebug()<< "RUNNING";
        QTime currentTime = QTime::currentTime();
        QString formattedTime = currentTime.toString("hh:mm:ss");
        QByteArray Command_send(LENGTH_COMMAND_SEND,0);
        // CHỜ CẢM BIÊN ĐẠT ĐỦ ĐỘ ẨM
        quint32 timeEstimate = 0;
        CalEstimateTimeProtocol(&timeEstimate);
        startCountdown(timeEstimate);
        ui->textEdit_oligo_history_log->clear();
        //ui->textEdit_oligo_history_log->append("current Time : " + formattedTime);
        log_terminal_withTimestamp(" : SYSTEM START", Info); // log_terminal_withTimestamp
        Command_send[0] = CMD_CONTROL_AIR_START;
        Command_send[1] = 1;
        STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 10000);
        ui->btn_start_synthetic->setDisabled(true);
        quint8 u8_number_step_run =  0;
        quint8 u8_number_base_run =  0;
        quint32 u32_time_oligo_process_step = 0;
        quint8 u8_number_sub_run = ui->spbox_number_sub->value(); // lấy số sub cần Run
        //qDebug()<< "number sub run"<< u8_number_sub_run;
        quint8 u8_lastest_sub = 0;
        quint16 u16_max_base_setting_protocol = 0;
        // send các setting cần thiết 31/07 tạm thời chỗ này chỉ gửi giá trị calib valve sau này phát sinh thêm sẽ dùng command này
        // define index system data oligo
        const quint8 idx_start_opt_vaccum = 50;
        const quint8 idx_start_time_process = 60;
        const quint8 idx_start_time_wait = 80; // 20byte tuu 70 den 89
        const quint8 idx_start_time_fill_mixfunction_1 = 100; // 20byte tu 90 den 109
        const quint8 idx_start_time_fill_mixfunction_2 = 150; // 20byte tu 110 den 129
        const quint8 idx_start_time_fill_mixfunction_3 = 200; // 20byte tu 130 den 149
        const quint16 idx_start_sequence_amidite = 290;
        send_setting(); // send setting trước khi chu trình chạy
        global_var.advanced_setting.flag_auto_primming_chemical = ui->checkbox_autoPrim_amidite->isChecked();
        for(uint8_t ctn_sub = 0; ctn_sub < protocol_oligo.u8_number_sub; ctn_sub++)
        {
            if(syno24_machine.getAutoState() == State::STOPED)
                break;
            u16_max_base_setting_protocol = u16_max_base_setting_protocol + protocol_oligo.sub[ctn_sub].u8_number_base_on_sub;
            if(u16_max_base_setting_protocol < global_var.signal_status_oligo.u16_max_sequence_amidite_setting)
            {
                if(protocol_oligo.u8_number_sub == 1) // truong hop chi cai dat 1 sub thi sub cuoi cung chinh là sub 0
                {
                    u8_lastest_sub = 0;
                }
                else // trường hợp có nhiều sub phía sau thì cứ lấy sub tiếp theo mà chạy
                {
                    if(ctn_sub < (protocol_oligo.u8_number_sub -1))
                    {
                        u8_lastest_sub = ctn_sub + 1;
                    }
                    else
                    {
                        u8_lastest_sub = ctn_sub;
                    }
                }
            }
        }
        // kiểm tra xem sequence có dài hơn là protocol không
        int int_remain_base = global_var.signal_status_oligo.u16_max_sequence_amidite_setting - u16_max_base_setting_protocol;
        // THAY LOG HISTORY BẰNG LOG TERMINAL
        //ui->textEdit_oligo_history_log->append("Start Synthesis Oligo");
        //ui->textEdit_oligo_history_log->append("Protocol Included : " + QString::number(u8_number_sub_run) + " sub-protocol");
        //ui->textEdit_oligo_history_log->append("Protocol setting total: " + QString::number(u16_max_base_setting_protocol) + " base");
        //ui->textEdit_oligo_history_log->append("Table Sequence setting max: " + QString::number(global_var.signal_status_oligo.u16_max_sequence_amidite_setting) + " base");
        log_terminal("Start Synthesis Oligo" ,Info);
        log_terminal("Protocol Included : " + QString::number(u8_number_sub_run) + " sub-protocol",Info);
        log_terminal("Protocol setting total: " + QString::number(u16_max_base_setting_protocol) + " base",Info);
        log_terminal("Table Sequence setting max: " + QString::number(global_var.signal_status_oligo.u16_max_sequence_amidite_setting) + " base",Info);

        if(int_remain_base > 0) // sequence dài hơn protocol rồi // tự động tăng page cho sub
        {
            protocol_oligo.sub[u8_lastest_sub].u8_number_base_on_sub += int_remain_base;
            log_terminal("System auto continuous run "+ QString::number(int_remain_base)+ " last sequence with sub" +  QString::number(u8_lastest_sub+1), Info); // Success
            qDebug()<< "sub cuoi cung" << u8_lastest_sub;
            qDebug()<< "so base cua sub cuoi cung" << protocol_oligo.sub[u8_lastest_sub].u8_number_base_on_sub;
        }
        ui->lbl_status_system->setText("System Running Synthetic");
        TwoByte_to_u16 u16tb_Timefill_Volume_first_type;
        TwoByte_to_u16 u16tb_Timefill_Volume_function_mix[3];
        global_var.signal_status_oligo.u16_counter_base_finished = 0;
        ui->lbl_base_finished->setText(QString::number(global_var.signal_status_oligo.u16_counter_base_finished));

        // Chạy từng sub
        ui->textEdit_status_update_fw->clear();
        currentTime = QTime::currentTime();
        formattedTime = currentTime.toString("hh:mm:ss");
        //ui->textEdit_oligo_history_log->append("current Time : " + formattedTime);
        log_terminal_withTimestamp("", Info);
        for(uint8_t u8_counter_sub_run = 0; u8_counter_sub_run < u8_number_sub_run; u8_counter_sub_run++)
        {
            // lấy số sub của protocol
            global_var.updateSTTRun2UI.currentSub = u8_counter_sub_run;
            u8_number_base_run = protocol_oligo.sub[u8_counter_sub_run].u8_number_base_on_sub;
            log_terminal( "Running sub : "+ QString::number(u8_counter_sub_run + 1), Info);
            LogHistoryRun.appendToLog("Running sub : "+ QString::number(u8_counter_sub_run + 1));
            for(quint8 u8_counter_base_on_sub = 0; u8_counter_base_on_sub < u8_number_base_run; u8_counter_base_on_sub++)
            {
                global_var.signal_status_oligo.u16_counter_current_base = global_var.signal_status_oligo.u16_counter_base_finished + 1;
                // check xem có double coupling
                log_terminal("    - Base : " +  QString::number(global_var.signal_status_oligo.u16_counter_base_finished + 1), Info);
                LogHistoryRun.appendToLog("    - Base : " +  QString::number(global_var.signal_status_oligo.u16_counter_base_finished + 1));
                // start get data
                u8_number_step_run = protocol_oligo.sub[u8_counter_sub_run].u8_number_step_on_base;

                for(quint8 u8_counter_step = 0; u8_counter_step < u8_number_step_run; u8_counter_step++)
                {
                    global_var.updateSTTRun2UI.currentStep = u8_counter_step;
                    UpdateUISTTRun(0);
                    volume.save_parameter_valve();
                    volume.ReloadUIVolumeMNG();
                    // 11-08-2025 Tính năng hút khí trong hộp hóa chất
                    // get signal enable tính năng này
                    GetFeatureVacuumBox();
                    delay_ui.delay_ms(60);
                    log_terminal_append("        * step : " +  QString::number(u8_counter_step + 1) + STEP_NAME[protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u8_first_type_chemical], Normal);
                    // ghi vào file log 02-04-2025
                    LogHistoryRun.appendToLog("        * step : " +  QString::number(u8_counter_step + 1) + STEP_NAME[protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u8_first_type_chemical]);
                    //()<< "run sub "<< u8_counter_sub_run;
                    //qDebug()<< "    number base" << u8_counter_base_on_sub<< " run step"<< u8_counter_step;
                    quint8 u8_first_chemical_temp = 127;
                    // lấy function thật
                    u8_first_chemical_temp = get_FirstChemical(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u8_first_type_chemical);
                    ui->textEdit_status_update_fw->append("STEP RUN : " + QString::number(u8_counter_step));
                    //qDebug()<< " u8_first_type_chemical" << u8_first_chemical_temp;
                    // send sequence oxdation 06-09-2024
                    // Command_send.clear();
                    Command_send[0] = CMD_OX_SENQUENCE;
                    for(uint8_t u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                    {
                        if(global_var.amidite_well[u8_idx_well].linkages[global_var.signal_status_oligo.u16_counter_base_finished] == 2)
                        {
                            qDebug()<< "Well "<< u8_idx_well<< "OX2";
                        }
                        Command_send[u8_idx_well + 1] = global_var.amidite_well[u8_idx_well].linkages[global_var.signal_status_oligo.u16_counter_base_finished];
                        Command_send[u8_idx_well + 97] = global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];

                    }
                    Command_send[150] =  (global_var.signal_status_oligo.u16_counter_base_finished)&0xFF;
                    Command_send[151] =  (global_var.signal_status_oligo.u16_counter_base_finished << 8)&0xFF;
                    if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 10000))
                    {
#ifdef MAIN_WINDOW_DEBUG
                        qDebug()<<"Send Ox >>OK";
#endif
                    }
                    if((u8_first_chemical_temp == COUPLING) || u8_first_chemical_temp == CAPPING || u8_first_chemical_temp == COUPLING2) // neu la coupling amidite
                    {
                        for(quint8 idx_mix_fnc = 0; idx_mix_fnc < 3; idx_mix_fnc++)
                        {
                            u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data;
                            u16tb_Timefill_Volume_first_type.Data = 0; // neu la function coupling thi khong bom lan 1

                            //05-04-2025 các sequence amidite được chia thành 2 group coupling1 và coupling2
                            if(u8_first_chemical_temp == COUPLING) // nếu là coupling 1
                            {
                                for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                {
                                    // đây có phải là base cuối cùng của giếng đó chưa
                                    if(global_var.signal_status_oligo.u16_counter_base_finished >= global_var.amidite_well[u8_idx_well].LastbaseinWellCount)
                                    {
                                        qDebug()<< "Well "<< u8_idx_well<< " isLastBASE ";
                                        // kiểm tra xem sequence thuộc coupling lastbase  hay không
                                        if(isInCouplingLastBase(global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished]))
                                        {
                                            Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
                                            qDebug()<< "Thuộc base đặc biệt  "<< u8_idx_well<< " isLastBASE ";
                                        }
                                        else
                                        {
                                            // phải coupling 1 không
                                            if(isInCoupling1(global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished]))
                                            {
                                                Command_send[idx_start_sequence_amidite + u8_idx_well] = global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];
                                            }
                                            else
                                            {
                                                Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
                                            }
                                        }
                                    }
                                    else
                                    {
                                        if(isInCoupling1(global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished]))
                                        {
                                            Command_send[idx_start_sequence_amidite + u8_idx_well] = global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];
                                        }
                                        else
                                        {
                                            Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
                                        }
                                    }

#ifdef MAIN_WINDOW_DEBUG
                                    //qDebug()<< "u8_sequence"<<global_var.amidite_well[u8_idx_well].u8_sequence[u8_counter_base_on_sub];
#endif
                                    // 29/07/2023 lấy về giá trị gốc chưa qua convert firmware tự convert đối với function amidite
                                    global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data  = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data;
                                }
                            }
                            else
                            {
                                //05-04-2025 các sequence amidite được chia thành 2 group coupling1 và coupling2

                                if(u8_first_chemical_temp == COUPLING2)// nếu là coupling 2
                                {
                                    for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                    {
                                        // đây có phải là base cuối cùng của giếng đó chưa
                                        if(global_var.signal_status_oligo.u16_counter_base_finished >= global_var.amidite_well[u8_idx_well].LastbaseinWellCount)
                                        {
                                            qDebug()<< "Well "<< u8_idx_well<< " isLastBASE ";
                                            // kiểm tra xem sequence thuộc coupling 2 hay không
                                            if(isInCouplingLastBase(global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished]))
                                            {
                                                Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
                                            }
                                            else
                                            {
                                                // kiểm tra xem sequence thuộc coupling 2 hay không
                                                if(isInCoupling2(global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished]))
                                                {
                                                    Command_send[idx_start_sequence_amidite + u8_idx_well] = global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];
                                                }
                                                else
                                                {
                                                    Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
                                                }
                                            }
                                        }
                                        else
                                        {
                                            // kiểm tra xem sequence thuộc coupling 2 hay không
                                            if(isInCoupling2(global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished]))
                                            {
                                                Command_send[idx_start_sequence_amidite + u8_idx_well] = global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];
                                            }
                                            else
                                            {
                                                Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
                                            }
                                        }

#ifdef MAIN_WINDOW_DEBUG
                                        //qDebug()<< "u8_sequence"<<global_var.amidite_well[u8_idx_well].u8_sequence[u8_counter_base_on_sub];
#endif
                                        // 29/07/2023 lấy về giá trị gốc chưa qua convert firmware tự convert đối với function amidite
                                        global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data  = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data;
                                    }
                                }
                            }
                        } // end 3 mix function
                    }
                    else // khong phai FUNCTION MIXED
                    {
                        u16tb_Timefill_Volume_first_type.Data = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_Volume.Data;
                    }
                    // GET AUTO PRIMMING AMIDITE AND ACTIVATOR -GIANGLH 16-11-2024
                    global_var.advanced_setting.u16tb_autoPrim_volume_amidite.Data = ui->spinbx_volume_prim_amidite->value();
                    global_var.advanced_setting.u16tb_autoPrim_volume_Activator.Data = ui->spinbx_volume_prim_activator->value();
                    for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                    {
                        // 05-04-2025 đổi cách thực hiện chỗ này
                        // Command_send[idx_start_sequence_amidite + u8_idx_well] = global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];
                        qDebug()<< "u8_sequence"<< Command_send[idx_start_sequence_amidite + u8_idx_well];
                    }
                    Command_send[0] = CMD_DATA_OLIGO;
                    Command_send[1] = u8_first_chemical_temp;
                    Command_send[2] = u16tb_Timefill_Volume_first_type.Byte[0];
                    Command_send[3] = u16tb_Timefill_Volume_first_type.Byte[1];
                    Command_send[4] = protocol_oligo.sub[u8_counter_sub_run].douple_coupling_option;

                    Command_send[5] = u16tb_Timefill_Volume_function_mix[0].Byte[0];
                    Command_send[6] = u16tb_Timefill_Volume_function_mix[0].Byte[1];
                    Command_send[7] = u16tb_Timefill_Volume_function_mix[1].Byte[0];
                    Command_send[8] = u16tb_Timefill_Volume_function_mix[1].Byte[1];
                    Command_send[9] = u16tb_Timefill_Volume_function_mix[2].Byte[0];
                    Command_send[10] = u16tb_Timefill_Volume_function_mix[2].Byte[1];

                    Command_send[11] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[0];
                    Command_send[12] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[1];
                    Command_send[13] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[2];
                    Command_send[14] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_wait_after_fill.Byte[0];
                    Command_send[15] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_wait_after_fill.Byte[1];
                    // FLAG control enable auto control NITO
                    Command_send[16] = global_var.status_and_sensor.flag_enable_auto_control_air_Nito;
                    // ========= KIEM TRA BASE HIEN TAI CO PHAI LA BASE DAC BIET KHÔNG 10-02-2025 chua kiem tra
                    if(isbaseSpecial(global_var.signal_status_oligo.u16_counter_current_base, protocol_oligo.speacial_base))
                    {
                        Command_send[17] = true;
                        Command_send[18] = protocol_oligo.u16_scale_volume.Byte[0]; // nhan ty le volume
                        Command_send[19] = protocol_oligo.u16_scale_volume.Byte[1];
                        Command_send[20] = protocol_oligo.u16_scale_time.Byte[0]; // ty le thoi gian
                        Command_send[21] = protocol_oligo.u16_scale_time.Byte[1];
                    }
                    else
                    {
                        Command_send[17] = false;
                    }
                    //                    Command_send[22] =  (global_var.signal_status_oligo.u16_counter_base_finished)&0xFF;
                    //                    Command_send[23] =  (global_var.signal_status_oligo.u16_counter_base_finished << 8)&0xFF;

                    //                    Command_send[24] =  (global_var.signal_status_oligo.u16_counter_base_finished)&0xFF;
                    //                    Command_send[25] =  (global_var.signal_status_oligo.u16_counter_base_finished << 8)&0xFF;
                    Command_send[26] =  u8_counter_step;
                    Command_send[27] = global_var.advanced_setting.flag_auto_primming_chemical;
                    Command_send[28] = global_var.advanced_setting.u16tb_autoPrim_volume_amidite.Byte[0];
                    Command_send[29] = global_var.advanced_setting.u16tb_autoPrim_volume_amidite.Byte[1];
                    Command_send[30] = global_var.advanced_setting.u16tb_autoPrim_volume_Activator.Byte[0];
                    Command_send[31] = global_var.advanced_setting.u16tb_autoPrim_volume_Activator.Byte[1];

                    Command_send[32] = global_var.advanced_setting.FillChemistryDone.EnableFillWellDone;
                    Command_send[33] = global_var.advanced_setting.FillChemistryDone.En_WASH;
                    Command_send[34] = global_var.advanced_setting.FillChemistryDone.En_Deblock;
                    Command_send[35] = global_var.advanced_setting.FillChemistryDone.En_Coupling;
                    Command_send[36] = global_var.advanced_setting.FillChemistryDone.En_Deblock;
                    Command_send[37] = global_var.advanced_setting.FillChemistryDone.En_Coupling;
                    Command_send[38] = global_var.advanced_setting.FillChemistryDone.typeReagent;
                    Command_send[39] = global_var.advanced_setting.FillChemistryDone.volumeWASH.Byte[0];
                    Command_send[40] = global_var.advanced_setting.FillChemistryDone.volumeWASH.Byte[1];
                    Command_send[41] = global_var.advanced_setting.FillChemistryDone.volumeDeblock.Byte[0];
                    Command_send[43] = global_var.advanced_setting.FillChemistryDone.volumeDeblock.Byte[1];
                    Command_send[44] = global_var.advanced_setting.FillChemistryDone.volumeCoupling.Byte[0];
                    Command_send[45] = global_var.advanced_setting.FillChemistryDone.volumeCoupling.Byte[1];
                    Command_send[46] = global_var.advanced_setting.FillChemistryDone.volumeCap.Byte[0];
                    Command_send[47] = global_var.advanced_setting.FillChemistryDone.volumeCap.Byte[1];
                    Command_send[48] = global_var.advanced_setting.FillChemistryDone.volumeOx.Byte[0];
                    Command_send[49] = global_var.advanced_setting.FillChemistryDone.volumeOx.Byte[1];
                    u32_time_oligo_process_step = 0;
                    // idx_start_opt_vaccum = 50 bắt đầu byte 50
                    for(uint8_t idx_process = 0; idx_process < 10; idx_process++)
                    {
                        Command_send[idx_start_opt_vaccum + idx_process]            = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u8_option_pressure[idx_process];
                        Command_send[idx_start_time_process + idx_process*2]        = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_procs_time[idx_process].Byte[0];
                        Command_send[idx_start_time_process + idx_process*2 + 1]    = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_procs_time[idx_process].Byte[1];
                        Command_send[idx_start_time_wait + idx_process*2]           = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[idx_process].Byte[0];
                        Command_send[idx_start_time_wait + idx_process*2 +1]        = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[idx_process].Byte[1];
                        u32_time_oligo_process_step +=  protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_procs_time[idx_process].Data;
                        u32_time_oligo_process_step +=  protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[idx_process].Data * SCALE_LEVEL_WAITTIME; // x10 thoi gian do bi /10 ui
                    }
                    u32_time_oligo_process_step += 100000; // thoi gian doi command
                    if(u8_first_chemical_temp == OXIDATION_IODINE || u8_first_chemical_temp == OXIDATION_IODINE2 )
                    {
                        u32_time_oligo_process_step = u32_time_oligo_process_step * 2.5; // coi lại chỗ này sao phải nhân cho 2.5 ?? quên comment rồi
                    }
#ifdef MAIN_WINDOW_DEBUG
                    //qDebug()<<"u32_time_oligo_process_step" <<u32_time_oligo_process_step ;
#endif
                    // từ byte 100 đến 147
                    for(uint8_t u8_time_fill_idx = 0; u8_time_fill_idx < MAX_WELL_AMIDITE; u8_time_fill_idx++)
                    {
                        Command_send[idx_start_time_fill_mixfunction_1 + u8_time_fill_idx*2]       = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[0].Byte[0];
                        Command_send[idx_start_time_fill_mixfunction_1 + u8_time_fill_idx*2 +1]    = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[0].Byte[1];
                    }
                    // tu byte 150 den byte 197
                    for(uint8_t u8_time_fill_idx = 0; u8_time_fill_idx < MAX_WELL_AMIDITE; u8_time_fill_idx++)
                    {
                        Command_send[idx_start_time_fill_mixfunction_2 + u8_time_fill_idx*2]       = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[1].Byte[0];
                        Command_send[idx_start_time_fill_mixfunction_2 + u8_time_fill_idx*2 +1]    = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[1].Byte[1];
                    }
                    // tu byte 200 den byte 247
                    for(uint8_t u8_time_fill_idx = 0; u8_time_fill_idx < MAX_WELL_AMIDITE; u8_time_fill_idx++)
                    {
                        Command_send[idx_start_time_fill_mixfunction_3 + u8_time_fill_idx*2]       = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[2].Byte[0];
                        Command_send[idx_start_time_fill_mixfunction_3 + u8_time_fill_idx*2 +1]    = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[2].Byte[1];
                    }
                    // ====================================================================================================
                    const quint16 idx_VacuumBox = 350; // 20byte tu 90 den 109
                    if(isbaseSpecial((global_var.signal_status_oligo.u16_counter_base_finished +1), m_baseVacuumBox.speacial_base))
                    {
                        int currentIdx = idx_VacuumBox; // Biến theo dõi chỉ số hiện tại
                        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.Enablefeature;
                        currentIdx++;
                        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_WASH;
                        currentIdx++;
                        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Deblock;
                        currentIdx++;
                        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Coupling;
                        currentIdx++;
                        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Ox;
                        currentIdx++;
                        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Cap;
                        currentIdx++;
                        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.time.Byte[0];
                        currentIdx++;
                        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.time.Byte[1];
                        //currentIdx++;
                    }else
                    {
                        int currentIdx = idx_VacuumBox; // Biến theo dõi chỉ số hiện tại
                        Command_send[currentIdx]= false;
                    }
#ifdef MAIN_WINDOW_DEBUG
                    qDebug()<<"Send command START OLIGO";
#endif
                    if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 10000))
                    {
#ifdef MAIN_WINDOW_DEBUG
                        qDebug()<< QTime::currentTime();
                        qDebug()<<"FW receive successful"<<"START OLIGO";
#endif
                        Command_send[0] = CMD_START_OLIGO_STEP;
                        //global_var.signal_status_oligo.u8_FLAG_FINISHED_PROCESS = false;
                        if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, u32_time_oligo_process_step))
                        {
                            //global_var.signal_status_oligo.u8_FLAG_FINISHED_PROCESS = true; // da finished roi
                            //                            ui->textEdit_oligo_history_log->moveCursor(QTextCursor::End);
                            //                            ui->textEdit_oligo_history_log->insertPlainText (" >>> Completed");
                            //                            ui->textEdit_oligo_history_log->moveCursor(QTextCursor::End);
                            log_terminal(" >> Completed", Success);
                            // Double coupling process
                            delay_ui.delay_ms(60);
                            if(u8_first_chemical_temp == COUPLING)
                            {
                                switch (protocol_oligo.sub[u8_counter_sub_run].douple_coupling_option)
                                {
                                case DOUBLE_COUPLING_FIRSTBASE:
                                {
                                    if(u8_counter_base_on_sub == 0)
                                    {
                                        Command_send[0] = CMD_START_OLIGO_STEP;
                                        log_terminal_append("Run Double Coupling ", Info);
                                        // global_var.signal_status_oligo.u8_FLAG_FINISHED_PROCESS = false;
                                        if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, u32_time_oligo_process_step))
                                        {
                                            //global_var.signal_status_oligo.u8_FLAG_FINISHED_PROCESS = true; // da finished roi
                                            //                                            ui->textEdit_oligo_history_log->moveCursor(QTextCursor::End);
                                            //                                            ui->textEdit_oligo_history_log->insertPlainText (" = ok");
                                            //                                            ui->textEdit_oligo_history_log->moveCursor(QTextCursor::End);
                                            log_terminal("OK",Success );
                                        }
                                        else
                                        {
                                            //global_var.signal_status_oligo.u8_FLAG_FINISHED_PROCESS = true; // da finished roi
                                            // 29-07-2023 THÊM TÍNH NĂNG TỰ ĐỘNG NGỪNG KHI GIAO TIẾP VỚI FIRMWARE KHÔNG THÀNH CÔNG
                                            //global_var.signal_status_oligo.b_FLAG_PAUSE_OLIGO = true;
                                            syno24_machine.setAutoState(State::PAUSE);// 20-01 sua thanh statemachine
                                            log_terminal(" error", Error );
                                            log_terminal(" SYSTEM AUTO PAUSED", Error );
                                            log_terminal(" FIRMWARE NOT RESPONSE ", Error );
                                            //                                            ui->textEdit_oligo_history_log->moveCursor(QTextCursor::End);
                                            //                                            ui->textEdit_oligo_history_log->insertPlainText (" = error");
                                            //                                            ui->textEdit_oligo_history_log->moveCursor(QTextCursor::End);
                                            //                                            ui->textEdit_oligo_history_log->append("SYSTEM AUTO PAUSED");
                                            //                                            ui->textEdit_oligo_history_log->append("FIRMWARE NOT RESPONSE ==========================================");
                                        }
                                        // TÍNH TOÁN TRỪ HOÁ CHẤT tại bước DOUBLE COUPLING
                                        for(quint8 idx_mix_fnc = 0; idx_mix_fnc < 3; idx_mix_fnc++)
                                        {
                                            if(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc] == CLG_AMIDITE)
                                            {
                                                //                                                for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                                //                                                {
                                                //                                                    // 29/07/2023 lấy về giá trị gốc chưa qua convert firmware tự convert đối với function amidite
                                                //                                                    //global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data  = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data;
                                                //                                                    volume.sub_volume(global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished],
                                                //                                                            protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                                //                                                }

                                            }
                                            else // KHONG PHAI COUPLING AMIDITE
                                            {

                                                //                            u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data = fnc.valve_calculator_timefill(&global_var, protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],
                                                //                                                                                                                 protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                                //                                                qDebug()<< "KHONG PHAI COUPLING AMIDITE";
                                                //                                                for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                                //                                                {
                                                //                                                    if(global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished] != CHEMICAL_SUBTANCE_EMPTY)
                                                //                                                    {
                                                //                                                        //volume.sub_volume_amidite(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                                //                                                    }
                                                //                                                }
                                            }
                                        } // end 3 mix function
                                        // HẾT BƯỚC TRỪ HOÁ CHẤT
                                    }
                                    break;
                                }
                                case DOUBLE_COUPLING_FIRST_SECOND_BASE:
                                {
                                    // double 2 base
                                    if(u8_counter_base_on_sub == 0 || u8_counter_base_on_sub == 1)
                                    {
                                        Command_send[0] = CMD_START_OLIGO_STEP;
                                        //ui->textEdit_oligo_history_log->append("Run Double Coupling ");
                                        log_terminal_append("Run Double Coupling ", Info);
                                        //global_var.signal_status_oligo.u8_FLAG_FINISHED_PROCESS = false; // chua finished
                                        if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, u32_time_oligo_process_step))
                                        {
                                            //global_var.signal_status_oligo.u8_FLAG_FINISHED_PROCESS = true; // finished

                                            //                                            ui->textEdit_oligo_history_log->moveCursor(QTextCursor::End);
                                            //                                            ui->textEdit_oligo_history_log->insertPlainText (" >>> Completed");
                                            //                                            ui->textEdit_oligo_history_log->moveCursor(QTextCursor::End);
                                            log_terminal(" Completed",Success );
                                        }
                                        else
                                        {
                                            // 29-07-2023 THÊM TÍNH NĂNG TỰ ĐỘNG NGỪNG KHI GIAO TIẾP VỚI FIRMWARE KHÔNG THÀNH CÔNG
                                            syno24_machine.setAutoState( State::PAUSE);// 20-01 sua thanh statemachine
                                            log_terminal(" error", Error );
                                            log_terminal(" SYSTEM AUTO PAUSED", Error );
                                            log_terminal(" FIRMWARE NOT RESPONSE ", Error );
                                        }
                                        // TÍNH TOÁN TRỪ HOÁ CHẤT tại bước DOUBLE COUPLING
                                        for(quint8 idx_mix_fnc = 0; idx_mix_fnc < 3; idx_mix_fnc++)
                                        {
                                            if(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc] == CLG_AMIDITE)
                                            {
                                                for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                                {
                                                    // 29/07/2023 lấy về giá trị gốc chưa qua convert firmware tự convert đối với function amidite
                                                    //global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data  = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data;
                                                    //                                                    volume.sub_volume(global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished],
                                                    //                                                            protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                                }

                                            }
                                            else // KHONG PHAI COUPLING AMIDITE
                                            {

                                                //                            u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data = fnc.valve_calculator_timefill(&global_var, protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],
                                                //                                                                                                                 protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                                //                                                qDebug()<< "KHONG PHAI COUPLING AMIDITE";
                                                //                                                for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                                //                                                {
                                                //                                                    if(global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished] != CHEMICAL_SUBTANCE_EMPTY)
                                                //                                                    {
                                                //                                                        // volume.sub_volume_amidite(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                                //                                                    }
                                                //                                                }
                                            }
                                        } // end 3 mix function
                                        // HẾT BƯỚC TRỪ HOÁ CHẤT
                                    }
                                    break;
                                }
                                case DOUBLE_COUPLING_ALL_BASE:
                                {
                                    Command_send[0] = CMD_START_OLIGO_STEP;
                                    //global_var.signal_status_oligo.u8_FLAG_FINISHED_PROCESS = false; // chua finished
                                    //ui->textEdit_oligo_history_log->append("Run Double Coupling ");
                                    log_terminal_append("Run Double Coupling ", Info);
                                    if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, u32_time_oligo_process_step))
                                    {
                                        //global_var.signal_status_oligo.u8_FLAG_FINISHED_PROCESS = true; // da finished roi
                                        //                                        ui->textEdit_oligo_history_log->moveCursor(QTextCursor::End);
                                        //                                        ui->textEdit_oligo_history_log->insertPlainText (" >>> Completed");
                                        //                                        ui->textEdit_oligo_history_log->moveCursor(QTextCursor::End);
                                        log_terminal(" Completed",Success );
                                    }
                                    else
                                    {
                                        // 29-07-2023 THÊM TÍNH NĂNG TỰ ĐỘNG NGỪNG KHI GIAO TIẾP VỚI FIRMWARE KHÔNG THÀNH CÔNG
                                        //global_var.signal_status_oligo.b_FLAG_PAUSE_OLIGO = true;
                                        //ui->textEdit_oligo_history_log->append("SYSTEM AUTO PAUSED");
                                        //global_var.signal_status_oligo.u8_FLAG_FINISHED_PROCESS = true; // da finished roi
                                        // ui->textEdit_oligo_history_log->append("FIRMWARE NOT RESPONSE ==============");
                                        syno24_machine.setAutoState( State::PAUSE); // 20-01 sua thanh statemachine
                                        log_terminal(" error", Error );
                                        log_terminal(" SYSTEM AUTO PAUSED", Error );
                                        log_terminal(" FIRMWARE NOT RESPONSE ", Error );

                                    }
                                    // TÍNH TOÁN TRỪ HOÁ CHẤT tại bước DOUBLE COUPLING
                                    for(quint8 idx_mix_fnc = 0; idx_mix_fnc < 3; idx_mix_fnc++)
                                    {
                                        if(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc] == CLG_AMIDITE)
                                        {
                                            for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                            {
                                                // 29/07/2023 lấy về giá trị gốc chưa qua convert firmware tự convert đối với function amidite
                                                //global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data  = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data;
                                                //                                                volume.sub_volume(global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished],
                                                //                                                        protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                            }

                                        }
                                        else // KHONG PHAI COUPLING AMIDITE
                                        {

                                            //                            u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data = fnc.valve_calculator_timefill(&global_var, protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],
                                            //                                                                                                                 protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                            //                                            qDebug()<< "KHONG PHAI COUPLING AMIDITE";
                                            //                                            for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                            //                                            {
                                            //                                                if(global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished] != CHEMICAL_SUBTANCE_EMPTY)
                                            //                                                {
                                            //                                                    //volume.sub_volume_amidite(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                            //                                                }
                                            //                                            }
                                        }
                                    } // end 3 mix function
                                    // HẾT BƯỚC TRỪ HOÁ CHẤT
                                    break;
                                }
                                default:
                                {
                                    break;
                                }
                                } // end switch
                            } // END IF double coupling
                        }
                        else // start oligo khong thanh cong = PAUSE va bao loi
                        {
                            //global_var.signal_status_oligo.u8_FLAG_FINISHED_PROCESS = true; // da finished roi
                            // 29-07-2023 THÊM TÍNH NĂNG TỰ ĐỘNG NGỪNG KHI GIAO TIẾP VỚI FIRMWARE KHÔNG THÀNH CÔNG
                            //global_var.signal_status_oligo.b_FLAG_PAUSE_OLIGO = true;
                            // 20-01
                            // 20-01 sua thanh statemachine
                            syno24_machine.setAutoState( State::PAUSE); // 20-01 sua thanh statemachine
                            log_terminal(" error", Error );
                            log_terminal(" SYSTEM AUTO PAUSED", Error );
                            log_terminal(" FIRMWARE NOT RESPONSE ", Error );
                            break;
                        }
                    }
                    else // 20-01-24 gui data oligo khong thanh cong
                    {
                        // 29-07-2023 THÊM TÍNH NĂNG TỰ ĐỘNG NGỪNG KHI GIAO TIẾP VỚI FIRMWARE KHÔNG THÀNH CÔNG
                        //global_var.signal_status_oligo.b_FLAG_PAUSE_OLIGO = true;
                        syno24_machine.setAutoState( State::PAUSE); // 20-01 sua thanh statemachine
                        log_terminal(" error", Error );
                        log_terminal(" SYSTEM AUTO PAUSED", Error );
                        log_terminal(" FIRMWARE NOT RESPONSE - Please Restart SYSTEM", Error );

#ifdef MAIN_WINDOW_DEBUG
                        qDebug()<<"FW feadback error";
                        break;
#endif
                    }

                    if(syno24_machine.getAutoState() ==  State::STOPED)
                        break;
                }// run all step of base
                global_var.signal_status_oligo.u16_counter_base_finished++;
                // monitor plate lên màn hình
                MonitorPlateUpdateUI(global_var.signal_status_oligo.u16_counter_base_finished); //

                ui->lbl_base_finished->setText(QString::number(global_var.signal_status_oligo.u16_counter_base_finished));
                //xu ly truong hop hết sequence
                /// GET GIA TRI NHIET DO- DO AM FORM FIRMWARE ===================================================================================
                delay_ui.delay_ms(10);
                if((global_var.signal_status_oligo.u16_counter_base_finished % 1) == 0)
                {
                    //                    //ui->textEdit_oligo_history_log->append(">>>>EXHAUSTED_CHEMICAL");
                    //                    Command_send[0] = CMD_FIRMWARE_UPDATE_STATUS_AND_SENSOR;
                    //                    if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 10000))
                    //                    {
                    //                        //                        ui->textEdit_oligo_history_log->moveCursor(QTextCursor::End);
                    //                        //                        ui->textEdit_oligo_history_log->insertPlainText (" >>> Completed");
                    //                        //                        ui->textEdit_oligo_history_log->moveCursor(QTextCursor::End);
                    //                    }
                    get_sensor_humidity_tempareture();
                    log_terminal("Humidity: " + QString::number(global_var.status_and_sensor.f_humidity / 100), Trace);
                    log_terminal_withTimestamp("", Info);
                    //                    ui->textEdit_oligo_history_log->append("Humidity: " + QString::number(global_var.status_and_sensor.f_humidity / 100));
                    //                    currentTime = QTime::currentTime();
                    //                    formattedTime = currentTime.toString("hh:mm:ss");
                    //                    ui->textEdit_oligo_history_log->append("current Time : " + formattedTime);
                }
                if(syno24_machine.getAutoState() == State::PAUSE)
                {
                    log_terminal("SYSTEM PAUSED", Error);
                }
                // xu ly pause 11-01-2025
                while(syno24_machine.getAutoState() == State::PAUSE)
                {
                    delay_ui.delay_ms(100);
                    if(syno24_machine.getAutoState() == State::STOPED)
                        break;
                }
                if(syno24_machine.getAutoState() == State::STOPED)
                    break;
                // nếu số base dài hơn sequence
                if(global_var.signal_status_oligo.u16_counter_base_finished >= global_var.signal_status_oligo.u16_max_sequence_amidite_setting)
                {
                    log_terminal("THE SYSTEM HAS AUTOMATICALLY STOPPED, AND THE OLIGO SEQUENCE IS COMPLETE", Success);
                    syno24_machine.setAutoState(State::STOPED);
                    //global_var.signal_status_oligo.u8_FLAG_RUN_OLIGO = STOP_PROCESS_SYNTHETIC_OLIGO;
                }
                if(syno24_machine.getAutoState() == State::STOPED)
                    break;
            }// vòng lặp chạy các base

            if(syno24_machine.getAutoState() == State::STOPED)
                break;
            delay_ui.delay_ms(20);
        }// vòng lặp chạy hết các sub =================================================== end tất cả các Sub bình thương
        //global_var.signal_status_oligo.u8_FLAG_RUN_OLIGO = STOP_PROCESS_SYNTHETIC_OLIGO;
        syno24_machine.setAutoState(State::STOPED);
        // chay ve home cho nguoi dung lay frit ra khoi khay
        on_btn_Run2HomeStep_released();
        // 21/03/2025 Giang Them flag và thời gian exhaustFan
        global_var.advanced_setting.flag_exhaustFan = ui->checkbox_exhaustFan->isChecked();
        global_var.advanced_setting.u16tb_timeExhaustFan.Data = ui->spinbx_exhaustFan->value();
        //QByteArray Command_send(LENGTH_COMMAND_SEND,0);
        Command_send[0] = CMD_STOP_SYSTHETIC_OLIGO;
        Command_send[1] = global_var.advanced_setting.flag_exhaustFan;
        Command_send[2] = global_var.advanced_setting.u16tb_timeExhaustFan.Byte[0];
        Command_send[3] = global_var.advanced_setting.u16tb_timeExhaustFan.Byte[1];
        //global_var.signal_status_oligo.u8_FLAG_FINISHED_PROCESS = false;
        uint32_t timeExhaustFan = global_var.advanced_setting.u16tb_timeExhaustFan.Data * 60 * 1000;
        if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, timeExhaustFan))
        {
            log("STOP OK");
            qDebug()<<"STOP OK";
        }

        //global_var.signal_status_oligo.u8_FLAG_FINISHED_PROCESS = true;
        ui->btn_start_synthetic->setDisabled(false);
        log_terminal("Finish Synthesis Oligo", Success);
        volume.save_parameter_valve();
        volume.ReloadUIVolumeMNG();
    }// end IF FLAG == STOP == neu flag == stop thi moi START
    else // neu flag la START thi canh bao nguoi dung
    {
        log_terminal("Finish Synthesis Oligo", Success);
        syno24_machine.setAutoState(State::STOPED);
        syno24_machine.procsess_ui();
        //        if(syno24_machine.getAutoState() != State::STOPED)
        //        {
        //            QMessageBox::warning(this,"Warning", "System Oligo Running, Please Waitting or STOP and Restart");
        //        }
        //        if(STM32_COM.flag_connecttion == false)
        //        {
        //            QMessageBox::warning(this,"Warning", "System Don't connect to Device, Please Connect Device to Run");
        //        }

    }
    stopCountdown();
    timer_update_humidity_tempareture.start(5000);
    LogHistoryRun.appendToLog(ui->textEdit_oligo_history_log->toPlainText());
    LogHistoryRun.appendToLog("Finish Run Protocol");
    LogHistoryRun.setLogFileReadOnly();

    */
}


void SYNO24::on_btn_pause_synthetic_released()
{
    if(QMessageBox::question(this, "PAUSE to complete the current cycle.", "DO YOU WANT PAUSE PROCESS") == QMessageBox::Yes)
    {
        if(syno24_machine.getAutoState() == State::PAUSE)
        {
            ui->btn_pause_synthetic->setText("PAUSE");
            ui->lbl_status_system->setText("System Paused");
            ui->lbl_status_system->setText("System Running Synthetic");
            syno24_machine.setAutoState( State::RUNNING);
            //ui->btn_start_synthetic->setEnabled(true);
        }
        else
        {
            syno24_machine.setAutoState( State::PAUSE);
            ui->lbl_status_system->setText("System Paused");
            ui->btn_pause_synthetic->setText("RESUME");
            //ui->btn_start_synthetic->setDisabled(true);
        }
    }
}


void SYNO24::on_spbox_number_sub_valueChanged(int arg1)
{
}


void SYNO24::on_pushButton_5_released() // button save as protocol
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"), filemanager.protocol_Path, tr("JSON Files (*.json);;All Files (*)"));
    //QFileDialog::getOpenFileName(this, "Open file", path_file_manager.applicationDirpath,"json(*.json)");
    if (fileName.isEmpty())
    {
        return;
    }
    fnc.save_protocol_toJson(&protocol_oligo, fileName);// save dữ liệu vào path mới
}


void SYNO24::on_btn_stop_synthetic_released()
{
    if(QMessageBox::question(this, "STOP ALL PROCESS", "DO YOU WANT STOP PROCESS") == QMessageBox::Yes)
    {
        if(syno24_machine.getAutoState() == State::RUNNING || syno24_machine.getAutoState() == State::PAUSE)
        {
            //global_var.signal_status_oligo.u8_FLAG_RUN_OLIGO = STOP_PROCESS_SYNTHETIC_OLIGO;
            syno24_machine.setAutoState(State::STOPED);
            ui->btn_pause_synthetic->setText("PAUSE");
            ui->textEdit_oligo_history_log->append("Please Waitting for finish current Step!");
            ui->lbl_status_system->setText("System Stoped Synthetic");
#ifdef MAIN_WINDOW_DEBUG
            syno24_machine.printModeAndState();
#endif
        }
    }
    else
    {
        ui->lbl_status_system->setText("System Running Synthetic");
#ifdef MAIN_WINDOW_DEBUG
        qDebug()<<" DON'T STOP ALL PROCESS";
#endif
    }
}


void SYNO24::on_btn_RunStepper_released()
{
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    //global_var.control_stepper.u16tb_X_Distance.Data = ui->spbx_x_stepper->value();
    //global_var.control_stepper.u16tb_Y_Distance.Data = ui->spbx_y_stepper->value();
    //global_var.control_stepper.u16tb_Z1_Distance.Data = ui->spbx_z1_stepper->value();

    global_var.control_stepper.u16tb_X_Distance.Data = (ui->spbx_x_coordinates->value() * 10);
    global_var.control_stepper.u16tb_Y_Distance.Data = (ui->spbx_y_coordinates->value() * 10);
    global_var.control_stepper.u16tb_Z1_Distance.Data = (ui->spbx_z1_coordinates->value() * 10);

    Command_send[0] = CMD_RUNSTEPPER; // RUN STEPPER
    //-------------------------------------------------------------------------
    Command_send[1] = global_var.control_stepper.u16tb_X_Distance.Byte[0];
    Command_send[2] = global_var.control_stepper.u16tb_X_Distance.Byte[1];
    Command_send[3] = global_var.control_stepper.u16tb_Y_Distance.Byte[0];
    Command_send[4] = global_var.control_stepper.u16tb_Y_Distance.Byte[1];
    Command_send[5] = global_var.control_stepper.u16tb_Z1_Distance.Byte[0];
    Command_send[6] = global_var.control_stepper.u16tb_Z1_Distance.Byte[1];
    STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 5000);
}

void SYNO24::on_btn_Run2HomeStep_released()
{
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    Command_send[0] = CMD_RUN2HOME; // RUN STEPPER
    //Command_send[1] = (ui->db_spbox_time_primming->value() *10);
    STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 20000);
}

// Process UI/ UX từ chỗ này bắt đầu chỉ xử lý các lệnh software UI/UX
void SYNO24::Setstyle_groupsub(quint8 grbx)
{
    data_ui.u8_current_STEP_edit_ui = 0;
    data_ui.u8_current_SUB_edit_ui = grbx;
    load_protocol_to_ui(data_ui.u8_current_SUB_edit_ui, 0);
    /*
    switch(grbx)
    {
    case 0:
    {
        // select_sub_1 đang được chọn
        data_ui.u8_current_SUB_edit_ui = 0;

        //qDebug()<< " new ui select_sub_ "<< data_ui.u8_current_SUB_edit_ui;
        ui->sub1_step_1->setChecked(true);
        ui->widget_sub_1->setDisabled(false);
        ui->widget_sub_2->setDisabled(true);
        ui->widget_sub_3->setDisabled(true);
        ui->widget_sub_4->setDisabled(true);
        ui->widget_sub_5->setDisabled(true);

        ui->widget_sub_1->setStyleSheet(widget_stylesheet_enable);
        ui->widget_sub_2->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_3->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_4->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_5->setStyleSheet(widget_stylesheet_disable);
        break;
    }
    case 1:
    {
        data_ui.u8_current_SUB_edit_ui = 1;
        // select_sub_2 đang được chọn
        ui->sub2_step_1->setChecked(true);
        ui->widget_sub_1->setDisabled(true);
        ui->widget_sub_2->setDisabled(false);
        ui->widget_sub_3->setDisabled(true);
        ui->widget_sub_4->setDisabled(true);
        ui->widget_sub_5->setDisabled(true);
        ui->widget_sub_1->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_2->setStyleSheet(widget_stylesheet_enable);
        ui->widget_sub_3->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_4->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_5->setStyleSheet(widget_stylesheet_disable);
        //qDebug()<< "new ui  select_sub_ "<< data_ui.u8_current_SUB_edit_ui;
        break;
    }
    case 2:
    {
        data_ui.u8_current_SUB_edit_ui = 2;
        // select_sub_3 đang được chọn
        ui->sub3_step_1->setChecked(true);
        ui->widget_sub_1->setDisabled(true);
        ui->widget_sub_2->setDisabled(true);
        ui->widget_sub_3->setDisabled(false);
        ui->widget_sub_4->setDisabled(true);
        ui->widget_sub_5->setDisabled(true);
        ui->widget_sub_1->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_2->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_3->setStyleSheet(widget_stylesheet_enable);
        ui->widget_sub_4->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_5->setStyleSheet(widget_stylesheet_disable);
        //qDebug()<< "new ui  select_sub_ "<<data_ui.u8_current_SUB_edit_ui;
        break;
    }
    case 3:
    {
        data_ui.u8_current_SUB_edit_ui = 3;
        ui->sub4_step_1->setChecked(true);
        ui->widget_sub_1->setDisabled(true);
        ui->widget_sub_2->setDisabled(true);
        ui->widget_sub_3->setDisabled(true);
        ui->widget_sub_4->setDisabled(false);
        ui->widget_sub_5->setDisabled(true);
        ui->widget_sub_1->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_2->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_3->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_4->setStyleSheet(widget_stylesheet_enable);
        ui->widget_sub_5->setStyleSheet(widget_stylesheet_disable);
        // Không có select box nào được chọn
        //qDebug()<< "new ui select_sub_ " << data_ui.u8_current_SUB_edit_ui;
        break;
    }
    case 4:
    {
        data_ui.u8_current_SUB_edit_ui = 4;
        ui->sub5_step_1->setChecked(true);
        ui->widget_sub_1->setDisabled(true);
        ui->widget_sub_2->setDisabled(true);
        ui->widget_sub_3->setDisabled(true);
        ui->widget_sub_4->setDisabled(true);
        ui->widget_sub_5->setDisabled(false);
        ui->widget_sub_1->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_2->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_3->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_4->setStyleSheet(widget_stylesheet_disable);
        ui->widget_sub_5->setStyleSheet(widget_stylesheet_enable);
        //qDebug()<< "new ui select_sub_ "<< data_ui.u8_current_SUB_edit_ui;
        break;
    }
    default:
    {
        break;
    }
    }
    load_protocol_to_ui(data_ui.u8_current_SUB_edit_ui, 0);
    */
}

void SYNO24::checkSelected_STEP()
{
    static QRadioButton *sub_step[MAX_STEP_OF_SUB] = {
        ui->sub1_step_1,
        ui->sub1_step_2,
        ui->sub1_step_3,
        ui->sub1_step_4,
        ui->sub1_step_5,
        ui->sub1_step_6,
        ui->sub1_step_7,
        ui->sub1_step_8,
        ui->sub1_step_9,
        ui->sub1_step_10,
        ui->sub1_step_11,
        ui->sub1_step_12,
        ui->sub1_step_13,
        ui->sub1_step_14,
        ui->sub1_step_15,
        ui->sub1_step_16,
        ui->sub1_step_17,
        ui->sub1_step_18,
        ui->sub1_step_19,
        ui->sub1_step_20
    };
    for(int i = 0; i < MAX_STEP_OF_SUB; i++)
    {
        if(sub_step[i]->isChecked())
        {
            data_ui.u8_current_STEP_edit_ui = i;
            qDebug()<< "select_sub "<< data_ui.u8_current_SUB_edit_ui<< " >>step_ "<< data_ui.u8_current_STEP_edit_ui;
            break;
        }
    }
    load_protocol_to_ui(data_ui.u8_current_SUB_edit_ui,data_ui.u8_current_STEP_edit_ui);
}

void SYNO24::on_select_sub_1_toggled(bool checked)
{
    if(checked)
    {
        Setstyle_groupsub(0);
    }
}


void SYNO24::on_select_sub_2_toggled(bool checked)
{
    if(checked)
    {
        Setstyle_groupsub(1);
    }
}


void SYNO24::on_select_sub_3_toggled(bool checked)
{
    if(checked)
    {
        Setstyle_groupsub(2);
    }

}

void SYNO24::on_select_sub_4_toggled(bool checked)
{
    if(checked)
    {
        Setstyle_groupsub(3);
    }
}


void SYNO24::on_select_sub_4_released()
{

}


void SYNO24::on_select_sub_5_toggled(bool checked)
{
    if(checked)
    {
        Setstyle_groupsub(4);
    }
}
void SYNO24::on_select_sub_6_toggled(bool checked)
{
    if(checked)
    {
        Setstyle_groupsub(5);
    }
}
void SYNO24::on_select_sub_7_toggled(bool checked)
{
    if(checked)
    {
        Setstyle_groupsub(6);
    }
}
void SYNO24::on_select_sub_8_toggled(bool checked)
{
    if(checked)
    {
        Setstyle_groupsub(7);
    }
}
void SYNO24::on_select_sub_9_toggled(bool checked)
{
    if(checked)
    {
        Setstyle_groupsub(8);
    }
}
void SYNO24::on_select_sub_10_toggled(bool checked)
{
    if(checked)
    {
        Setstyle_groupsub(9);
    }
}

void SYNO24::checkSelectChemical()
{
    static quint8 u8_current_option_chemical;
    //static quint8 u8_chemical_odinal;
    static QRadioButton *optionChemical_step[7] =
    {
        ui->selected_deblock,
        ui->selected_washing,
        ui->selected_coupling,
        ui->selected_oxidation,
        ui->selected_capping,
        ui->selected_oxidation2,
        ui->selected_coupling_2,
    };
    u8_current_option_chemical = -1;
    for(int i = 0; i < 7; i++)
    {
        if(optionChemical_step[i]->isChecked())
        {
            u8_current_option_chemical = i;
            qDebug()<< "option "<< u8_current_option_chemical<< STEP_NAME[u8_current_option_chemical];
            // break;
        }
    }

    switch(u8_current_option_chemical)
    {
    case DEBLOCK_FNC:
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->lbl_step_name->setText(STEP_NAME[u8_current_option_chemical]);
        break;
    }
    case WASH_FNC:
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->lbl_step_name->setText(STEP_NAME[u8_current_option_chemical]);
        break;
    }
    case COUPLING_FNC:// mix function view
    {
        ui->stackedWidget->setCurrentIndex(1);// mix function view
        ui->lbl_step_name->setText(STEP_NAME[u8_current_option_chemical]);
        break;
    }
    case OXICATION_FNC:
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->lbl_step_name->setText(STEP_NAME[u8_current_option_chemical]);
        break;
    }
    case CAP_FNC:// mix function view
    {
        ui->stackedWidget->setCurrentIndex(1);// mix function view
        ui->lbl_step_name->setText(STEP_NAME[u8_current_option_chemical]);
        break;
    }
    case OXICATION_FNC_2:
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->lbl_step_name->setText(STEP_NAME[u8_current_option_chemical]);
        break;
    }
    case COUPLING_FNC_2:// mix function view
    {
        ui->stackedWidget->setCurrentIndex(1);// mix function view
        ui->lbl_step_name->setText(STEP_NAME[u8_current_option_chemical]);
        break;
    }
    default :
    {
        ui->lbl_step_name->setText("ERROR");
        QMessageBox::warning(this, tr("Error"), tr("Please choice Chemical"));
        break;
    }
    }
    protocol_oligo.u8_step_cycle = u8_current_option_chemical;
    qDebug()<< "choice STEP debug" << STEP_NAME[u8_current_option_chemical];
}

void SYNO24::setUI_FirstChemical(quint8 u8_step_cycle)
{
    switch(u8_step_cycle)
    {
    case DEBLOCK_FNC:
    {

        ui->stackedWidget->setCurrentIndex(0);
        ui->selected_deblock->setChecked(true);
        ui->lbl_step_name->setText(STEP_NAME[u8_step_cycle]);
        break;
    }
    case WASH_FNC:
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->selected_washing->setChecked(true);
        ui->lbl_step_name->setText(STEP_NAME[u8_step_cycle]);
        break;
    }
    case COUPLING_FNC:
    {
        ui->stackedWidget->setCurrentIndex(1);// mix function view
        ui->selected_coupling->setChecked(true);
        ui->lbl_step_name->setText(STEP_NAME[u8_step_cycle]);
        break;
    }
    case OXICATION_FNC:
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->selected_oxidation->setChecked(true);
        ui->lbl_step_name->setText(STEP_NAME[u8_step_cycle]);
        break;
    }
    case CAP_FNC:
    {
        ui->selected_capping->setChecked(true);
        ui->stackedWidget->setCurrentIndex(1); // mix function view
        ui->lbl_step_name->setText(STEP_NAME[u8_step_cycle]);
        break;
    }
    case OXICATION_FNC_2:
    {
        ui->selected_oxidation2->setChecked(true);
        ui->stackedWidget->setCurrentIndex(0);
        ui->lbl_step_name->setText(STEP_NAME[u8_step_cycle]);
        break;
    }
    case COUPLING_FNC_2:
    {
        ui->selected_coupling_2->setChecked(true);
        ui->stackedWidget->setCurrentIndex(1);// mix function view
        ui->lbl_step_name->setText(STEP_NAME[u8_step_cycle]);
        break;
    }
    default :
    {
        ui->stackedWidget->setCurrentIndex(0);
        ui->selected_deblock->setChecked(true);
        ui->lbl_step_name->setText(STEP_NAME[u8_step_cycle]);
        break;
    }
    }
}

quint8 SYNO24::get_FirstChemical(quint8 u8_chemical)
{
    quint8 u8_chemical_odinal = 0;
    switch(u8_chemical)
    {
    case DEBLOCK_FNC:
    {
        u8_chemical_odinal = TCA_in_DCM;
        break;
    }
    case WASH_FNC:
    {
        u8_chemical_odinal = WASH_ACN_DCM;
        break;
    }
    case COUPLING_FNC:
    {
        u8_chemical_odinal = COUPLING;

        break;
    }
    case OXICATION_FNC:
    {
        u8_chemical_odinal = OXIDATION_IODINE;//_IODINE
        break;
    }
    case CAP_FNC:
    {
        u8_chemical_odinal = CAPPING;
        break;
    }
    case OXICATION_FNC_2:
    {
        u8_chemical_odinal = OXIDATION_IODINE2;
        break;
    }
    case COUPLING_FNC_2:
    {
        u8_chemical_odinal = COUPLING2;
        break;
    }
    default :
    {
        break;
    }
    }
    return u8_chemical_odinal;
}

void SYNO24::on_btn_ManualRun_released() // ManualRun
{
    timer_update_humidity_tempareture.stop();
    if(syno24_machine.getAutoState() == State::STOPED)
    {
        if(syno24_machine.setManualMode())
        {
            ui->stackedWidget_Run->setCurrentIndex(1); // go to stack manualRun
        }
    }
    else
    {
        QMessageBox::warning(this,"Warning", "SYSTEM BUSY RUNNING AUTO MODE Wait and try again!");
    }
}


void SYNO24::on_btn_backAtuoRun_released() // backtoAutoRun
{
    if(syno24_machine.getManualState() == State::STOPED)
    {
        if(syno24_machine.setAutoMode())
        {
            timer_update_humidity_tempareture.start(5000);
            ui->stackedWidget_Run->setCurrentIndex(0);
        }
    }
    else
    {
        QMessageBox::warning(this,"Warning", "System Running MANUAL MODE, Please Waitting for STOP");
    }
}
/*
void SYNO24::on_checkbx_manual_column_1_toggled(bool checked)
{
    ui->well_1->setChecked(checked);
    ui->well_2->setChecked(checked);
    ui->well_3->setChecked(checked);
    ui->well_4->setChecked(checked);
    ui->well_5->setChecked(checked);
    ui->well_6->setChecked(checked);
    ui->well_7->setChecked(checked);
    ui->well_8->setChecked(checked);
}


void SYNO24::on_checkbx_manual_column_2_toggled(bool checked)
{
    ui->well_9->setChecked(checked);
    ui->well_10->setChecked(checked);
    ui->well_11->setChecked(checked);
    ui->well_12->setChecked(checked);
    ui->well_13->setChecked(checked);
    ui->well_14->setChecked(checked);
    ui->well_15->setChecked(checked);
    ui->well_16->setChecked(checked);
}


void SYNO24::on_checkbx_manual_column_3_toggled(bool checked)
{
    ui->well_17->setChecked(checked);
    ui->well_18->setChecked(checked);
    ui->well_19->setChecked(checked);
    ui->well_20->setChecked(checked);
    ui->well_21->setChecked(checked);
    ui->well_22->setChecked(checked);
    ui->well_23->setChecked(checked);
    ui->well_24->setChecked(checked);
}
*/

void SYNO24::on_checkbx_manual_Allwell_toggled(bool checked)
{
    ui->well_1->setChecked(checked);
    ui->well_2->setChecked(checked);
    ui->well_3->setChecked(checked);
    ui->well_4->setChecked(checked);
    ui->well_5->setChecked(checked);
    ui->well_6->setChecked(checked);
    ui->well_7->setChecked(checked);
    ui->well_8->setChecked(checked);
    ui->well_9->setChecked(checked);
    ui->well_10->setChecked(checked);
    ui->well_11->setChecked(checked);
    ui->well_12->setChecked(checked);
    ui->well_13->setChecked(checked);
    ui->well_14->setChecked(checked);
    ui->well_15->setChecked(checked);
    ui->well_16->setChecked(checked);
    ui->well_17->setChecked(checked);
    ui->well_18->setChecked(checked);
    ui->well_19->setChecked(checked);
    ui->well_20->setChecked(checked);
    ui->well_21->setChecked(checked);
    ui->well_22->setChecked(checked);
    ui->well_23->setChecked(checked);
    ui->well_24->setChecked(checked);
}

void SYNO24:: Run_Manual_fill_Chemical()
{
    qDebug()<< "Run Manual fill Chemical";
    syno24_machine.setManualState(State::RUNNING);
    // Start Primming control
    uint16_t u16_volume_temp = ui->spbx_volume_manual->value();
    global_var.manual_run.u8_typeof_chemical = ui->cbx_type_chemical_manual_run->currentIndex();
    // 17-05-2025 khong can tinh thoi gian, ben duoi fw tự tính
    global_var.manual_run.u16_volume.Data = u16_volume_temp;


    uint16_t u16_time_wait_firmware = 0;
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    Command_send[0] = CMD_MANUAL_RUN; // FILL to well
    Command_send[25] = 1;
    global_var.manual_run.u8_checked_well[0] = ui->well_1->isChecked();
    global_var.manual_run.u8_checked_well[1] = ui->well_2->isChecked();
    global_var.manual_run.u8_checked_well[2] = ui->well_3->isChecked();
    global_var.manual_run.u8_checked_well[3] = ui->well_4->isChecked();
    global_var.manual_run.u8_checked_well[4] = ui->well_5->isChecked();
    global_var.manual_run.u8_checked_well[5] = ui->well_6->isChecked();
    global_var.manual_run.u8_checked_well[6] = ui->well_7->isChecked();
    global_var.manual_run.u8_checked_well[7] = ui->well_8->isChecked();
    global_var.manual_run.u8_checked_well[8] = ui->well_9->isChecked();
    global_var.manual_run.u8_checked_well[9] = ui->well_10->isChecked();
    global_var.manual_run.u8_checked_well[10] = ui->well_11->isChecked();
    global_var.manual_run.u8_checked_well[11] = ui->well_12->isChecked();
    global_var.manual_run.u8_checked_well[12] = ui->well_13->isChecked();
    global_var.manual_run.u8_checked_well[13] = ui->well_14->isChecked();
    global_var.manual_run.u8_checked_well[14] = ui->well_15->isChecked();
    global_var.manual_run.u8_checked_well[15] = ui->well_16->isChecked();
    global_var.manual_run.u8_checked_well[16] = ui->well_17->isChecked();
    global_var.manual_run.u8_checked_well[17] = ui->well_18->isChecked();
    global_var.manual_run.u8_checked_well[18] = ui->well_19->isChecked();
    global_var.manual_run.u8_checked_well[19] = ui->well_20->isChecked();
    global_var.manual_run.u8_checked_well[20] = ui->well_21->isChecked();
    global_var.manual_run.u8_checked_well[21] = ui->well_22->isChecked();
    global_var.manual_run.u8_checked_well[22] = ui->well_23->isChecked();
    global_var.manual_run.u8_checked_well[23] = ui->well_24->isChecked();

    Command_send[26] = global_var.manual_run.u8_typeof_chemical;
    Command_send[27] = global_var.manual_run.u16_volume.Byte[0];
    Command_send[28] = global_var.manual_run.u16_volume.Byte[1];
    // trừ hoá chất trong quản lý -- 31-10 quan ly hoa chat chua hoan thien tren SYNO24X
    for(uint8_t idx_valve = 0; idx_valve < MAX_WELL_AMIDITE; idx_valve++)
    {
        if( global_var.manual_run.u8_checked_well[idx_valve] == true)
        {
            //volume.sub_volume(global_var.manual_run.u8_typeof_chemical, u16_volume_temp);
        }
    }
    // gửi command
    for(uint8_t idx_valve = 0; idx_valve < MAX_WELL_AMIDITE; idx_valve++)
    {
        Command_send[idx_valve + 1] = global_var.manual_run.u8_checked_well[idx_valve];
#ifdef MAIN_WINDOW_DEBUG
        //qDebug()<< "function manual run" << idx_valve << Command_send[idx_valve + 1] << " volume" << global_var.manual_run.u16_volume.Data;
#endif
    }
    u16_time_wait_firmware = 30000;
    if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, u16_time_wait_firmware))
    {
        syno24_machine.printModeAndState();
        syno24_machine.setManualState(State::STOPED);
    }
    else
    {
        QMessageBox::warning(this,"Warning", "System Error, RESTART ALL SYSTEM");
    }
    syno24_machine.printModeAndState();
    syno24_machine.setManualState(State::STOPED);

}
void SYNO24:: Run_Manual_CtrlVacuum()
{
    syno24_machine.setManualState(State::RUNNING);
    if(syno24_machine.getManualState() == State::STOPED)
    {
        ui->btn_StartManual->setEnabled(true);
        ui->btn_AutoHome_Manual->setEnabled(true);
        ui->btn_StartManual_CtrlVacuum->setEnabled(true);
    }
    else
    {
        ui->btn_StartManual->setEnabled(false);
        ui->btn_AutoHome_Manual->setEnabled(false);
        ui->btn_StartManual_CtrlVacuum->setEnabled(false);
    }
    // Start Primming control
    uint16_t u16_volume_temp = ui->spbx_volume_manual->value();
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    Command_send[0] = CMD_MANUAL_RUN; // FILL to well

    global_var.manual_run.u8_option_pressure[0] = ui->cbx_option_pressure_manual->currentIndex();
    global_var.manual_run.u16tb_procs_time[0].Data = ui->spbx_time_process_presure_mamual->value();
    global_var.manual_run.u16tb_waitting_after_time[0].Data = ui->spbox_wait_manual->value();

    global_var.manual_run.u8_option_pressure[1] = ui->cbx_option_pressure_manual_2->currentIndex();
    global_var.manual_run.u16tb_procs_time[1].Data = ui->spbx_time_process_presure_mamual_2->value();
    global_var.manual_run.u16tb_waitting_after_time[1].Data = ui->spbox_wait_manual_2->value();

    global_var.manual_run.u8_option_pressure[2] = ui->cbx_option_pressure_manual_3->currentIndex();
    global_var.manual_run.u16tb_procs_time[2].Data = ui->spbx_time_process_presure_mamual_3->value();
    global_var.manual_run.u16tb_waitting_after_time[2].Data = ui->spbox_wait_manual_3->value();

    global_var.manual_run.u8_option_pressure[3] = ui->cbx_option_pressure_manual_4->currentIndex();
    global_var.manual_run.u16tb_procs_time[3].Data = ui->spbx_time_process_presure_mamual_4->value();
    global_var.manual_run.u16tb_waitting_after_time[3].Data = ui->spbox_wait_manual_4->value();
    Command_send[25] = 2;

    Command_send[29] = global_var.manual_run.u8_option_pressure[0];
    Command_send[30] = global_var.manual_run.u16tb_procs_time[0].Byte[0];
    Command_send[31] = global_var.manual_run.u16tb_procs_time[0].Byte[1];
    Command_send[32] = global_var.manual_run.u16tb_waitting_after_time[0].Byte[0];
    Command_send[33] = global_var.manual_run.u16tb_waitting_after_time[0].Byte[1];
    Command_send[34] = global_var.manual_run.u8_option_pressure[1];
    Command_send[35] = global_var.manual_run.u16tb_procs_time[1].Byte[0];
    Command_send[36] = global_var.manual_run.u16tb_procs_time[1].Byte[1];
    Command_send[37] = global_var.manual_run.u16tb_waitting_after_time[1].Byte[0];
    Command_send[38] = global_var.manual_run.u16tb_waitting_after_time[1].Byte[1];
    Command_send[39] = global_var.manual_run.u8_option_pressure[2];
    Command_send[40] = global_var.manual_run.u16tb_procs_time[2].Byte[0];
    Command_send[41] = global_var.manual_run.u16tb_procs_time[2].Byte[1];
    Command_send[42] = global_var.manual_run.u16tb_waitting_after_time[2].Byte[0];
    Command_send[43] = global_var.manual_run.u16tb_waitting_after_time[2].Byte[1];
    Command_send[44] = global_var.manual_run.u8_option_pressure[3];
    Command_send[45] = global_var.manual_run.u16tb_procs_time[3].Byte[0];
    Command_send[46] = global_var.manual_run.u16tb_procs_time[3].Byte[1];
    Command_send[47] = global_var.manual_run.u16tb_waitting_after_time[3].Byte[0];
    Command_send[48] = global_var.manual_run.u16tb_waitting_after_time[3].Byte[1];
    uint32_t u16_time_wait_firmware = 60000;
    u16_time_wait_firmware = u16_time_wait_firmware + global_var.manual_run.u16tb_waitting_after_time[0].Data;
    u16_time_wait_firmware = u16_time_wait_firmware + global_var.manual_run.u16tb_waitting_after_time[1].Data;
    u16_time_wait_firmware = u16_time_wait_firmware + global_var.manual_run.u16tb_waitting_after_time[2].Data;
    u16_time_wait_firmware = u16_time_wait_firmware + global_var.manual_run.u16tb_procs_time[0].Data;
    u16_time_wait_firmware = u16_time_wait_firmware + global_var.manual_run.u16tb_procs_time[1].Data;
    u16_time_wait_firmware = u16_time_wait_firmware + global_var.manual_run.u16tb_procs_time[2].Data;

    qDebug()<<" wait time " << u16_time_wait_firmware;
    if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, u16_time_wait_firmware))
    {
        //global_var.signal_status_oligo.b_flag_MANUAL_RUN = STOP_MANUAL_PROCESS_SYNTHETIC_OLIGO;
    }
    else
    {
        QMessageBox::warning(this,"Warning", "System Error, RESTART ALL SYSTEM");
    }

    syno24_machine.setManualState(State::STOPED);
    syno24_machine.printModeAndState();
    if(syno24_machine.getManualState() == State::STOPED)
    {
        ui->btn_StartManual->setEnabled(true);
        ui->btn_AutoHome_Manual->setEnabled(true);
        ui->btn_StartManual_CtrlVacuum->setEnabled(true);
    }
    else
    {
        ui->btn_StartManual->setEnabled(false);
        ui->btn_AutoHome_Manual->setEnabled(false);
        ui->btn_StartManual_CtrlVacuum->setEnabled(false);
    }

}
void SYNO24::on_btn_StartManual_released() // fill chemical
{
    //syno24_machine.setManualState(State::RUNNING);
    Run_Manual_fill_Chemical();
    //syno24_machine.setManualState(State::STOPED);
}


void SYNO24::on_btn_StartManual_CtrlVacuum_released()
{
    //ui->btn_StartManual_CtrlVacuum->setDisabled(true);
    // ui->btn_StartManual->setDisabled(true);
    syno24_machine.setManualState(State::RUNNING);
    Run_Manual_CtrlVacuum();
    //ui->btn_StartManual_CtrlVacuum->setDisabled(false);
    //ui->btn_StartManual->setDisabled(false);
    syno24_machine.setManualState(State::STOPED);
}

void SYNO24:: Display_Protocol_to_user()
{
    quint8 u8_number_sub = protocol_oligo.u8_number_sub;
    ui->spbox_number_sub->setValue(protocol_oligo.u8_number_sub);
    quint8 u8_number_step_current_sub = 0;
    quint8 u8_number_base_current_sub = 0;
    QString log_str = "";
    QString log_mix_function = "";
    QString log_control_pressure = "";
    ui->textEdit_list_task_protocol->clear();
    ui->textEdit_list_task_protocol->append("Protocol will run " + QString::number(u8_number_sub)+ " sub");
    ui->textEdit_list_task_protocol->append("Max length Table sequence " + QString::number(global_var.signal_status_oligo.u16_max_sequence_amidite_setting)+ " mer");
    uint32_t TimeEstimate = 0;
    CalEstimateTimeProtocol(&TimeEstimate);
    QString timeString = convertSecondsToHHMMSS(TimeEstimate);
    ui->textEdit_list_task_protocol->append("Protocol Time Estimate : " + timeString);
    for(uint8_t u8_counter_sub = 0; u8_counter_sub < u8_number_sub; u8_counter_sub++)
    {
        u8_number_base_current_sub = protocol_oligo.sub[u8_counter_sub].u8_number_base_on_sub;
        u8_number_step_current_sub = protocol_oligo.sub[u8_counter_sub].u8_number_step_on_base;
        //qDebug()<<"u8_number_sub: "<< u8_number_sub;
        //qDebug()<<"u8_number_step_current_sub: "<< u8_number_step_current_sub;
        //qDebug()<<"u8_number_base_current_sub: "<< u8_number_base_current_sub;
        ui->textEdit_list_task_protocol->append("Sub "+ QString::number(u8_counter_sub + 1)+ ":" + " Have " + QString::number(u8_number_base_current_sub) + " Base");
        for(uint8_t u8_counter_step = 0; u8_counter_step < u8_number_step_current_sub; u8_counter_step++)
        {
            quint8 u8_first_chemical_temp = 127;
            u8_first_chemical_temp = get_FirstChemical(protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u8_first_type_chemical);

            //qDebug()<< "u8_first_type_chemical" << u8_first_chemical_temp;
            if((u8_first_chemical_temp == COUPLING) || u8_first_chemical_temp == CAPPING|| u8_first_chemical_temp == COUPLING2) // neu la coupling amidite
            {
                log_str = "         STEP " + QString::number(u8_counter_step + 1) + " "+STEP_NAME[protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u8_first_type_chemical];
                //qDebug()<< "log step name " << log_str;
                ui->textEdit_list_task_protocol->append(log_str);
                log_mix_function = "        ";
                for(quint8 idx_mix_fnc = 0; idx_mix_fnc < 3; idx_mix_fnc++)
                {
                    //quint16 u16tb_Timefill_Volume_first_type.Data = 0; // neu la function coupling thi khong bom lan 1
                    log_mix_function = log_mix_function + " | "+ NAME_MIX_FUNCTION[protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc]]+
                            " volume: "+QString::number(protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data) + "ul" " | ";
                }
                ui->textEdit_list_task_protocol->moveCursor(QTextCursor::End);
                ui->textEdit_list_task_protocol->insertPlainText (log_mix_function);
                ui->textEdit_list_task_protocol->moveCursor(QTextCursor::End);
                //ui->textEdit_list_task_protocol->append(log_mix_function);
                log_mix_function.clear();
            }
            else
            {
                log_str = "         STEP " + QString::number(u8_counter_step + 1) + " " + STEP_NAME[protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u8_first_type_chemical]+ "| volume: "+ QString::number(protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u16tb_Volume.Data) + + "ul";
                //qDebug()<<log_str;
                ui->textEdit_list_task_protocol->append(log_str);
            }
            log_control_pressure= "  ";
            for(uint8_t idx_process = 0; idx_process < 10; idx_process++)
            {
                if(protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[idx_process].Data != 0 &&
                        protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[idx_process].Data
                        )
                    log_control_pressure = log_control_pressure + NAME_OPTION_PRESSURE[protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[idx_process]] +  " - "+
                            QString::number(protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[idx_process].Data) + " - "
                            + QString::number(protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[idx_process].Data * SCALE_LEVEL_WAITTIME) + " | ";
            }
            ui->textEdit_list_task_protocol->moveCursor(QTextCursor::End);
            ui->textEdit_list_task_protocol->insertPlainText(log_control_pressure);
            ui->textEdit_list_task_protocol->moveCursor(QTextCursor::End);
            log_control_pressure = "";
            log_str = "";
        }
    }
    // 21-08-2025 Le Hoai Giang add Special Sub display
    if(ui->chkbx_EnaSpecialLastBase->isChecked())
    {
        uint32_t u8_counter_sub = 9;
        u8_number_base_current_sub = protocol_oligo.sub[u8_counter_sub].u8_number_base_on_sub;
        u8_number_step_current_sub = protocol_oligo.sub[u8_counter_sub].u8_number_step_on_base;
        ui->textEdit_list_task_protocol->append("Final Special Sub Have: " + QString::number(u8_number_base_current_sub) + " Base");
        for(uint8_t u8_counter_step = 0; u8_counter_step < u8_number_step_current_sub; u8_counter_step++)
        {
            quint8 u8_first_chemical_temp = 127;
            u8_first_chemical_temp = get_FirstChemical(protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u8_first_type_chemical);

            //qDebug()<< "u8_first_type_chemical" << u8_first_chemical_temp;
            if((u8_first_chemical_temp == COUPLING) || u8_first_chemical_temp == CAPPING|| u8_first_chemical_temp == COUPLING2) // neu la coupling amidite
            {
                log_str = "         STEP " + QString::number(u8_counter_step + 1) + " "+STEP_NAME[protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u8_first_type_chemical];
                //qDebug()<< "log step name " << log_str;
                ui->textEdit_list_task_protocol->append(log_str);
                log_mix_function = "        ";
                for(quint8 idx_mix_fnc = 0; idx_mix_fnc < 3; idx_mix_fnc++)
                {
                    //quint16 u16tb_Timefill_Volume_first_type.Data = 0; // neu la function coupling thi khong bom lan 1
                    log_mix_function = log_mix_function + " | "+ NAME_MIX_FUNCTION[protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc]]+
                            " volume: "+QString::number(protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data) + "ul" " | ";
                }
                ui->textEdit_list_task_protocol->moveCursor(QTextCursor::End);
                ui->textEdit_list_task_protocol->insertPlainText (log_mix_function);
                ui->textEdit_list_task_protocol->moveCursor(QTextCursor::End);
                //ui->textEdit_list_task_protocol->append(log_mix_function);
                log_mix_function.clear();
            }
            else
            {
                log_str = "         STEP " + QString::number(u8_counter_step + 1) + " " + STEP_NAME[protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u8_first_type_chemical]+ "| volume: "+ QString::number(protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].fill_chemical.u16tb_Volume.Data) + + "ul";
                //qDebug()<<log_str;
                ui->textEdit_list_task_protocol->append(log_str);
            }
            log_control_pressure= "  ";
            for(uint8_t idx_process = 0; idx_process < 10; idx_process++)
            {
                if(protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[idx_process].Data != 0 &&
                        protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[idx_process].Data
                        )
                    log_control_pressure = log_control_pressure + NAME_OPTION_PRESSURE[protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].control_pressure.u8_option_pressure[idx_process]] +  " - "+
                            QString::number(protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_procs_time[idx_process].Data) + " - "
                            + QString::number(protocol_oligo.sub[u8_counter_sub].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[idx_process].Data * SCALE_LEVEL_WAITTIME) + " | ";
            }
            ui->textEdit_list_task_protocol->moveCursor(QTextCursor::End);
            ui->textEdit_list_task_protocol->insertPlainText(log_control_pressure);
            ui->textEdit_list_task_protocol->moveCursor(QTextCursor::End);
            log_control_pressure = "";
            log_str = "";
        }

    }

}

void SYNO24::on_btn_save_protocol_released()
{
    fnc.save_protocol_toJson(&protocol_oligo, filemanager.protocol_Path);
    Display_Protocol_to_user();
}

void SYNO24::send_setting()
{
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    Command_send[0] = CMD_RECIVED_SETTING; // send setting

    for(uint8_t index_valve = 0; index_valve < MAX_NUMBER_VALVE; index_valve++)
    {
        volume.valve[index_valve].f_a.Data =  volume.valve[index_valve].a;
        volume.valve[index_valve].f_b.Data =  volume.valve[index_valve].b;
        // debug setting

        //qDebug()<< "valve " << index_valve  <<"f_a " <<  volume.valve[index_valve].f_a.Data << "f_b " << volume.valve[index_valve].f_b.Data;
        //qDebug()<< "f_b" << volume.valve[index_valve].f_b.Data;
        Command_send[index_valve*8 +1] = volume.valve[index_valve].f_a.Byte[0];
        Command_send[index_valve*8 +2] = volume.valve[index_valve].f_a.Byte[1];
        Command_send[index_valve*8 +3] = volume.valve[index_valve].f_a.Byte[2];
        Command_send[index_valve*8 +4] = volume.valve[index_valve].f_a.Byte[3];
        //====================================================================
        Command_send[index_valve*8 +5] = volume.valve[index_valve].f_b.Byte[0];
        Command_send[index_valve*8 +6] = volume.valve[index_valve].f_b.Byte[1];
        Command_send[index_valve*8 +7] = volume.valve[index_valve].f_b.Byte[2];
        Command_send[index_valve*8 +8] = volume.valve[index_valve].f_b.Byte[3];
    }
    if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 2000))
    {
        log("CMD SEND SETTING = OK");
    }
    else
    {
        log("CMD SEND SETTING = ERROR");
        QMessageBox::critical(this, tr("Error"), "NO DEVICE CONNECT, STARTUP DEVICE ERROR");
        //QMessageBox::critical(this, tr("Error: System not response, PLEASE RESTART SYSTEM! "), serialPort->errorString());
    }
}

bool SYNO24::ASK_VENDOR_ID()
{
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    Command_send[0] = CMD_ASK_VENDOR_ID; // send setting
    if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 2000))
    {
        log("CMD_ASK_VENDOR_ID = OK");
        return true;
    }
    else
    {
        return false;
    }
}
/*
void SYNO24::on_pushButton_released() // go HOME
{
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    Command_send[0] = CMD_RUN2HOME; // RUN STEPPER
    //Command_send[1] = (ui->db_spbox_time_primming->value() *10);
    STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 20000);
}
*/
void SYNO24:: get_sensor_humidity_tempareture()
{
    if(serialPort->isOpen())
    {
        QByteArray Command_send(LENGTH_COMMAND_SEND,0);
        Command_send[0] = CMD_FIRMWARE_UPDATE_STATUS_AND_SENSOR; // RUN STEPPER
        if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 20000))
        {
            log("sensor humidity tempareture = OK");
        }
    }
}
void SYNO24::wait_humidity()
{
    if( syno24_machine.getAutoState() == State::RUNNING && serialPort->isOpen())
    {
        QByteArray Command_send(LENGTH_COMMAND_SEND,0);
        Command_send[0] = CMD_FIRMWARE_UPDATE_STATUS_AND_SENSOR; // cmd get sensor
        Command_send[1] = CMD_FIRMWARE_UPDATE_STATUS_AND_SENSOR; // cmd control open air NITO
        Command_send[2] = global_var.status_and_sensor.u16tb_humidity_Preset.Byte[0]; // cmd get sensor
        Command_send[3] = global_var.status_and_sensor.u16tb_humidity_Preset.Byte[1]; // cmd control open air NITO
        if(STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 20000))
        {
            log("sensor humidity tempareture = OK");
        }
    }
}

// Hàm tương tự qDebug()
void SYNO24::log(const QString& message)
{
    ui->textEdit_status_update_fw->append(message);
}

void SYNO24:: readExcelSequence(QTableView *tableView, QString path)
{
    QXlsx::Document xlsx(path);
    if (!xlsx.isLoadPackage())
    {
        qDebug() << "Failed to open Excel file.";
        return;
    }
    for (int row = 2; row <= 25; ++row)
    {
        QString cellData = xlsx.read(row, 3).toString(); //POSITION
        //rowItems << new QStandardItem(cellData);
        cellData = xlsx.read(row, 4).toString(); // NAME
        //rowItems << new QStandardItem(cellData);
        cellData = xlsx.read(row, 6).toString(); // LENGTH
        //rowItems << new QStandardItem(cellData);
        cellData = xlsx.read(row, 5).toString();// SEQUENCE
        //rowItems << new QStandardItem(cellData);
        //rowData << rowItems;
        global_var.amidite_well[row - 2].string_sequence = xlsx.read(row, 5).toString(); // SEQUENCE
        global_var.amidite_well[row - 2].string_name = xlsx.read(row, 4).toString(); // NAME
    }
    reload_table_sequence();
}
void SYNO24:: reload_table_sequence()
{
    // Generate data to tableview
    for(int row = 0; row < MAX_WELL_AMIDITE; row++)
    {
        QModelIndex index = model_table_well->index(row, 3,QModelIndex());
        model_table_well->setData(index,global_var.amidite_well[row].string_sequence);
    }
    // UPDATE LENGTH TO TABLE
    for(int row = 0; row < MAX_WELL_AMIDITE; row++)
    {
        QModelIndex index = model_table_well->index(row, 2,QModelIndex());
        model_table_well->setData(index,global_var.amidite_well[row].sequenceLength);
    }
    // UPDATE LENGTH TO TABLE
    for(int row = 0; row < MAX_WELL_AMIDITE; row++)
    {
        QModelIndex index = model_table_well->index(row, 1,QModelIndex());
        model_table_well->setData(index,global_var.amidite_well[row].string_name);
    }
    for(quint8 col= 0; col < 4; col++)
    {
        for(int row = 0; row < 8; row++)
        {
            QModelIndex index = model_table_well->index(col*8 + row, 0,QModelIndex());
            model_table_well->setData(index,name_well_asign[row] +QString::number(col+1));
        }
    }
}
void SYNO24::on_btn_opeExcelSequence_released()
{
    QString sequenceExcel_path = QFileDialog::getOpenFileName(this, "Open file", filemanager.applicationDirpath,"Excel Files (*.xlsx)");
    if(sequenceExcel_path.isEmpty())
    {
        QMessageBox::warning(this, "Warning", " File Path protocol Empty, Please try choise again!!!");
    }
    else
    {
        readExcelSequence(ui->tableView, sequenceExcel_path);
        // calculator_volume_and_process_UI();
    }
}


void SYNO24::on_btn_sequence_released()
{
    filemanager.amidite_sequence_Path = QFileDialog::getOpenFileName(this, "Open file", filemanager.amidite_sequence_Dir,"json(*.json)");
    if(filemanager.amidite_sequence_Path.isEmpty())
    {
        QMessageBox::warning(this, "Warning", " File Path protocol Empty, Please try choise again!!!");
    }
    else
    {
        fnc.read_str_amidite_fromJson(&global_var, filemanager.amidite_sequence_Path); // READING FILE SEQUENCE TO RUN
        filemanager.save();
        amidite_.amiditeSyno24XGetSequence(&global_var);
        ui->lineEdit_path_sequence->setText(filemanager.amidite_sequence_Path);
        // 14/03/2023 mở comment này để chạy load giá trị lên ui
        // TÍNH TOÁN HOÁ CHÂT VÀ HIỂN THỊ SEQUENCE LÊN UI
        reload_table_sequence();
        calculator_volume_and_process_UI();
        //        // Generate data to tableview
        //        for(int row = 0; row < MAX_WELL_AMIDITE; row++)
        //        {
        //            QModelIndex index = model_table_well->index(row, 3,QModelIndex());
        //            model_table_well->setData(index,global_var.amidite_well[row].string_sequence);
        //        }
        //        // UPDATE LENGTH TO TABLE
        //        for(int row = 0; row < MAX_WELL_AMIDITE; row++)
        //        {
        //            QModelIndex index = model_table_well->index(row, 2,QModelIndex());
        //            model_table_well->setData(index,global_var.amidite_well[row].string_sequence.length());
        //        }
        //        // UPDATE LENGTH TO TABLE
        //        for(int row = 0; row < MAX_WELL_AMIDITE; row++)
        //        {
        //            QModelIndex index = model_table_well->index(row, 1,QModelIndex());
        //            model_table_well->setData(index,global_var.amidite_well[row].string_name);
        //        }
        //        for(quint8 col= 0; col < 4; col++)
        //        {
        //            for(int row = 0; row < 8; row++)
        //            {
        //                QModelIndex index = model_table_well->index(col*8 + row, 0,QModelIndex());
        //                model_table_well->setData(index,name_well_asign[row] +QString::number(col+1));
        //            }
        //        }
    }
}

// Hàm kiểm tra định dạng của văn bản trong QLineEdit
void SYNO24:: checkLineEditFormat(const QString &text) {
    // Biểu thức chính quy cho định dạng mong muốn
    QRegularExpression regex("^(\\d+,)*(\\d+)$");

    // Kiểm tra nếu văn bản không khớp với định dạng
    if (!regex.match(text).hasMatch()) {
        QMessageBox::warning(nullptr, "Lỗi", "Định dạng không hợp lệ. Xin vui lòng nhập lại theo định dạng");
        // Đặc điểm khác biệt tùy thuộc vào cách bạn muốn xử lý khi người dùng nhập sai định dạng.
    } else {
        // Định dạng hợp lệ, bạn có thể thực hiện các xử lý khác tại đây
    }
}
// Sử dụng hàm này khi người dùng kết thúc quá trình chỉnh sửa
void SYNO24:: onLineEditEditingFinished() {
    // Gọi hàm kiểm tra định dạng
    // checkLineEditFormat(u);
}

void SYNO24::readSpecialBaseFromLineEdit(QLineEdit *lineEdit, uint16_t special_base[MAX_SEQUENCE_OF_WELL]) {
    QString text = lineEdit->text();

    // Loại bỏ khoảng trắng và chia chuỗi thành danh sách các số
    QStringList numbers = text.split(QRegExp("\\s*,\\s*"), QString::SkipEmptyParts);

    // Chuyển đổi các số từ chuỗi sang uint16_t và lưu vào mảng
    int index = 0;
    for (const QString &number : numbers)
    {
        bool ok;
        uint value = number.toUInt(&ok);
        if (ok)
        {
            // Chỉ lưu giá trị nếu nó là số dương
            if (value <= MAX_SEQUENCE_OF_WELL)
            {
                special_base[index++] = static_cast<uint16_t>(value);
            }
        }
    }
    // Các phần tử còn lại của mảng được đặt thành giá trị mặc định hoặc 0
    for (; index < MAX_SEQUENCE_OF_WELL; ++index)
    {
        special_base[index] = 0;
    }
    printArray(special_base);
}

void SYNO24::printArray(const uint16_t special_base[MAX_SEQUENCE_OF_WELL])
{
    for (int i = 0; i < MAX_SEQUENCE_OF_WELL; ++i)
    {
        if(special_base[i] != 0)
        {
            qDebug() << "special_base[" << i << "] = " << special_base[i];
        }
    }
}


// Hàm kiểm tra số có trong mảng hay không
bool SYNO24:: isbaseSpecial(uint16_t number, const uint16_t special_base[MAX_SEQUENCE_OF_WELL])
{
    for (int i = 0; i < MAX_SEQUENCE_OF_WELL; ++i)
    {
        if (special_base[i] == number)
        {
            return true;
        }
    }
    return false;
}
//=========================================================== Volume process handle ui
void SYNO24::onButtonReleased_Add_A()
{
    volume.onButtonReleased_Add_Chemical(A);
}
void SYNO24::onButtonReleased_Add_T()
{
    volume.onButtonReleased_Add_Chemical(T);
}
void SYNO24::onButtonReleased_Add_G()
{
    volume.onButtonReleased_Add_Chemical(G);
}
void SYNO24::onButtonReleased_Add_C()
{
    volume.onButtonReleased_Add_Chemical(C);
}

void SYNO24::onButtonReleased_Add_a()
{
    volume.onButtonReleased_Add_Chemical(a);
}
void SYNO24::onButtonReleased_Add_t()
{
    volume.onButtonReleased_Add_Chemical(t);
}
void SYNO24::onButtonReleased_Add_g()
{
    volume.onButtonReleased_Add_Chemical(g);
}
void SYNO24::onButtonReleased_Add_c()
{
    volume.onButtonReleased_Add_Chemical(c);
}

void SYNO24::onButtonReleased_Add_F1()
{
    volume.onButtonReleased_Add_Chemical(I);
}
void SYNO24::onButtonReleased_Add_F2()
{
    volume.onButtonReleased_Add_Chemical(U);
}
void SYNO24::onButtonReleased_Add_ACT()
{
    volume.onButtonReleased_Add_Chemical(Activator);
}
void SYNO24::onButtonReleased_Add_TCA()
{
    volume.onButtonReleased_Add_Chemical(TCA_in_DCM);
}
void SYNO24::onButtonReleased_Add_WASH()
{
    volume.onButtonReleased_Add_Chemical(WASH_ACN_DCM);
}
void SYNO24::onButtonReleased_Add_OX()
{
    volume.onButtonReleased_Add_Chemical(OXIDATION_IODINE);
}
void SYNO24::onButtonReleased_Add_CAPA()
{
    volume.onButtonReleased_Add_Chemical(CAPPING_CAPA);
}
void SYNO24::onButtonReleased_Add_CAPB()
{
    volume.onButtonReleased_Add_Chemical(CAPPING_CAPB);
}
// ==================================================================
void SYNO24::onButtonReleased_Sub_A()
{
    volume.onButtonReleased_Sub_Chemical(A);
}
void SYNO24::onButtonReleased_Sub_T()
{
    volume.onButtonReleased_Sub_Chemical(T);
}
void SYNO24::onButtonReleased_Sub_G()
{
    volume.onButtonReleased_Sub_Chemical(G);
}

void SYNO24::onButtonReleased_Sub_C()
{
    volume.onButtonReleased_Sub_Chemical(C);
}
void SYNO24::onButtonReleased_Sub_I() // TẠM THỜI ĐANG DÙNG I - hoặc F1
{
    volume.onButtonReleased_Sub_Chemical(I);
}
void SYNO24::onButtonReleased_Sub_U() // TẠM THỜI DÙNG U - hoặc F2
{
    volume.onButtonReleased_Sub_Chemical(U);
}
void SYNO24::onButtonReleased_Sub_ACT()
{
    volume.onButtonReleased_Sub_Chemical(Activator);
}
void SYNO24::onButtonReleased_Sub_TCA()
{
    volume.onButtonReleased_Sub_Chemical(TCA_in_DCM);
}
void SYNO24::onButtonReleased_Sub_WASH()
{
    volume.onButtonReleased_Sub_Chemical(WASH_ACN_DCM);
}
void SYNO24::onButtonReleased_Sub_OX()
{
    volume.onButtonReleased_Sub_Chemical(OXIDATION_IODINE);
}
void SYNO24::onButtonReleased_Sub_CAPA()
{
    volume.onButtonReleased_Sub_Chemical(CAPPING_CAPA);
}
void SYNO24::onButtonReleased_Sub_CAPB()
{
    volume.onButtonReleased_Sub_Chemical(CAPPING_CAPB);
}
//==========================================================================
void SYNO24:: calculator_volume_and_process_UI()
{
    volume.reset_volume_cal();
    quint8 u8_number_step_run =  0;
    quint8 u8_number_base_run =  0;
    quint16 u16_counter_base_finished = 0;
    quint8 u8_lastest_sub = 0;
    quint16 u16_max_base_setting_protocol = 0;
    quint8 u8_first_chemical_temp = 127;
    quint8 u8_number_sub_run = ui->spbox_number_sub->value(); // lấy số sub cần Run
    protocol_oligo.u8_number_sub = u8_number_sub_run;
    for(uint8_t ctn_sub = 0; ctn_sub < protocol_oligo.u8_number_sub; ctn_sub++)
    {
        u16_max_base_setting_protocol = u16_max_base_setting_protocol + protocol_oligo.sub[ctn_sub].u8_number_base_on_sub;
        if(u16_max_base_setting_protocol < global_var.signal_status_oligo.u16_max_sequence_amidite_setting)
        {
            if(protocol_oligo.u8_number_sub == 1) // truong hop chi cai dat 1 sub thi sub cuoi cung chinh là sub 0
            {
                u8_lastest_sub = 0;
            }
            else // trường hợp có nhiều sub phía sau thì cứ lấy sub tiếp theo mà chạy
            {
                if(ctn_sub < (protocol_oligo.u8_number_sub -1))
                {
                    u8_lastest_sub = ctn_sub + 1;
                }
                else
                {
                    u8_lastest_sub = ctn_sub;
                }
            }
        }
    }
    // kiểm tra xem sequence có dài hơi là protocol không
    int int_remain_base = global_var.signal_status_oligo.u16_max_sequence_amidite_setting - u16_max_base_setting_protocol;
    if(int_remain_base > 0) // sequence dài hơn protocol rồi // tự động tăng page cho sub
    {
        protocol_oligo.sub[u8_lastest_sub].u8_number_base_on_sub += int_remain_base;
        //qDebug()<< "sub cuoi cung" << u8_lastest_sub;
        //qDebug()<< "so base cua sub cuoi cung" << protocol_oligo.sub[u8_lastest_sub].u8_number_base_on_sub;
    }
    for(uint8_t u8_counter_sub_run = 0; u8_counter_sub_run < u8_number_sub_run; u8_counter_sub_run++)
    {

        // lấy số base của sub-protocol
        u8_number_base_run = protocol_oligo.sub[u8_counter_sub_run].u8_number_base_on_sub;
        //qDebug()<< "u8_number_base_run" << u8_number_base_run;
        for(quint8 u8_counter_base_on_sub = 0; u8_counter_base_on_sub < u8_number_base_run; u8_counter_base_on_sub++)
        {
            //qDebug()<< "u16_counter_base_finished" << u16_counter_base_finished;
            u8_number_step_run = protocol_oligo.sub[u8_counter_sub_run].u8_number_step_on_base;
            for(quint8 u8_counter_step = 0; u8_counter_step < u8_number_step_run; u8_counter_step++)
            {
                u8_first_chemical_temp = get_FirstChemical(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u8_first_type_chemical);

                if((u8_first_chemical_temp == COUPLING ) || (u8_first_chemical_temp == CAPPING ) || (u8_first_chemical_temp == COUPLING2 )) // neu la coupling amidite
                {
                    for(quint8 idx_mix_fnc = 0; idx_mix_fnc < 3; idx_mix_fnc++)
                    {

                        if(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc] == CLG_AMIDITE)
                        {
                            for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                            {
                                // 29/07/2023 lấy về giá trị gốc chưa qua convert firmware tự convert đối với function amidite
                                //global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data  = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data;
                                volume.add_volume_amidite_cal(global_var.amidite_well[u8_idx_well].u8_sequence[u16_counter_base_finished],
                                                              protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                            }

                        }
                        else // KHONG PHAI COUPLING AMIDITE
                        {

                            //                            u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data = fnc.valve_calculator_timefill(&global_var, protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],
                            //                                                                                                                 protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                            //qDebug()<< "KHONG PHAI COUPLING AMIDITE";
                            for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                            {
                                if(global_var.amidite_well[u8_idx_well].u8_sequence[u16_counter_base_finished] != CHEMICAL_SUBTANCE_EMPTY)
                                {
                                    volume.add_volume_normal_cal(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                }
                            }
                        }
                    } // end 3 mix function
                }
                else // khong phai COUPLING ||  CAPPING
                {
                    //                    u16tb_Timefill_Volume_first_type.Data = fnc.valve_calculator_timefill(&global_var, u8_first_chemical_temp,
                    //                                                                                          protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_Volume.Data);

                    /*
                    for(uint8_t u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                    {
                        if(global_var.amidite_well[u8_idx_well].linkages[global_var.signal_status_oligo.u16_counter_base_finished] == 2)
                        {
                            qDebug()<< "Well "<< u8_idx_well<< "OX2";
                        }
                        Command_send[u8_idx_well +1] = global_var.amidite_well[u8_idx_well].linkages[global_var.signal_status_oligo.u16_counter_base_finished];
                    }
                    */
                    for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                    {
                        if(global_var.amidite_well[u8_idx_well].u8_sequence[u16_counter_base_finished] != CHEMICAL_SUBTANCE_EMPTY)
                        {
                            if(u8_first_chemical_temp == OXIDATION_IODINE)
                            {
                                if(global_var.amidite_well[u8_idx_well].linkages[u16_counter_base_finished]  != 2) // nếu không phải OX2 thì là ox1 cứ trừ đúng hóa chất đầu tiên
                                {
                                    volume.add_volume_normal_cal(u8_first_chemical_temp,  protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_Volume.Data);
                                }else
                                {
                                    volume.add_volume_normal_cal(OXIDATION_IODINE2,  protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_Volume.Data);
                                }

                            }
                            else
                            {
                                volume.add_volume_normal_cal(u8_first_chemical_temp,protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_Volume.Data);

                            }
                        }
                    }
                }
                if(u8_first_chemical_temp == COUPLING)
                {
                    switch (protocol_oligo.sub[u8_counter_sub_run].douple_coupling_option)
                    {
                    case DOUBLE_COUPLING_FIRSTBASE:
                    {
                        if(u8_counter_base_on_sub == 0)
                        {
                            for(quint8 idx_mix_fnc = 0; idx_mix_fnc < 3; idx_mix_fnc++)
                            {
                                if(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc] == CLG_AMIDITE)
                                {
                                    for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                    {
                                        // 29/07/2023 lấy về giá trị gốc chưa qua convert firmware tự convert đối với function amidite
                                        //global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data  = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data;
                                        volume.add_volume_amidite_cal(global_var.amidite_well[u8_idx_well].u8_sequence[u16_counter_base_finished],
                                                                      protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                    }
                                }
                                else // KHONG PHAI COUPLING AMIDITE
                                {

                                    //                            u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data = fnc.valve_calculator_timefill(&global_var, protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],
                                    //                                                                                                                 protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                    //                                    qDebug()<< "KHONG PHAI COUPLING AMIDITE";
                                    //                                    for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                    //                                    {
                                    //                                        if(global_var.amidite_well[u8_idx_well].u8_sequence[u16_counter_base_finished] != CHEMICAL_SUBTANCE_EMPTY)
                                    //                                        {
                                    //                                            // volume.add_volume_normal_cal(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                    //                                        }
                                    //                                    }
                                }
                            } // end 3 mix function
                        }
                        break;
                    }
                    case DOUBLE_COUPLING_FIRST_SECOND_BASE:
                    {
                        // double 2 base
                        if(u8_counter_base_on_sub == 0 || u8_counter_base_on_sub == 1)
                        {
                            for(quint8 idx_mix_fnc = 0; idx_mix_fnc < 3; idx_mix_fnc++)
                            {

                                if(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc] == CLG_AMIDITE)
                                {
                                    for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                    {
                                        // 29/07/2023 lấy về giá trị gốc chưa qua convert firmware tự convert đối với function amidite
                                        //global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data  = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data;
                                        volume.add_volume_amidite_cal(global_var.amidite_well[u8_idx_well].u8_sequence[u16_counter_base_finished],
                                                                      protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                    }
                                }
                                else // KHONG PHAI COUPLING AMIDITE
                                {

                                    //                            u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data = fnc.valve_calculator_timefill(&global_var, protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],
                                    //                                                                                                                 protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                    //                                    qDebug()<< "KHONG PHAI COUPLING AMIDITE";
                                    //                                    for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                    //                                    {
                                    //                                        if(global_var.amidite_well[u8_idx_well].u8_sequence[u16_counter_base_finished] != CHEMICAL_SUBTANCE_EMPTY)
                                    //                                        {
                                    //                                            //volume.add_volume_normal_cal(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                    //                                        }
                                    //                                    }
                                }
                            } // end 3 mix function
                        }
                        break;
                    }
                    case DOUBLE_COUPLING_ALL_BASE:
                    {
                        for(quint8 idx_mix_fnc = 0; idx_mix_fnc < 3; idx_mix_fnc++)
                        {
                            if(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc] == CLG_AMIDITE)
                            {
                                for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                {
                                    // 29/07/2023 lấy về giá trị gốc chưa qua convert firmware tự convert đối với function amidite
                                    //global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data  = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data;
                                    volume.add_volume_amidite_cal(global_var.amidite_well[u8_idx_well].u8_sequence[u16_counter_base_finished],
                                                                  protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                }
                            }
                            else // KHONG PHAI COUPLING AMIDITE
                            {

                                //                            u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data = fnc.valve_calculator_timefill(&global_var, protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],
                                //                                                                                                                 protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                for(quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++)
                                {
                                    if(global_var.amidite_well[u8_idx_well].u8_sequence[u16_counter_base_finished] != CHEMICAL_SUBTANCE_EMPTY)
                                    {
                                        volume.add_volume_normal_cal(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc],protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data);
                                    }
                                }
                            }
                        } // end 3 mix function
                        break;
                    }
                    }
                }
            }// chay so step
            u16_counter_base_finished++;
        }// chay so base
    }
    volume.cal_remain_volume();
    for(quint8 u8_idx_well = 0; u8_idx_well < 12; u8_idx_well++)
    {
        //qDebug()<< "Chemical need" << NAME_MIX_FUNCTION[u8_idx_well] << volume.valve[u8_idx_well].volume_calculator_need;
        //qDebug()<< "Chemical remain" << NAME_MIX_FUNCTION[u8_idx_well] << volume.valve[u8_idx_well].volume_remain;
    }
    volume.tableView_display_data();
}

//void SYNO24::on_btn_calculator_volume_released()
//{
//    calculator_volume_and_process_UI();
//}




void SYNO24::on_btn_AutoHome_Manual_released()
{
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    Command_send[0] = CMD_RUN2HOME; // RUN STEPPER
    STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 20000);
    syno24_machine.setManualState( State::STOPED);
}


void SYNO24::on_btn_save_edit_amidite_released()
{
    QString  Amidite_Floatting1 = ui->lineEdit_float_1->text();
    QString  Amidite_Floatting2 = ui->lineEdit_float_2->text();
    char amidite[19] = {'A', 'T', 'G', 'C', 'a', 't', 'g', 'c', 'Y', 'R', 'W', 'S', 'K', 'M', 'D', 'V', 'N', 'H', 'B'};  // Example array
    // Kiểm tra floatting_1
    if (Amidite_Floatting1.length() == 1) {
        QChar char1 = Amidite_Floatting1[0];
        for (char c : amidite) {
            if (char1 == c) {
                ui->lineEdit_float_1->setText("I");
                break;
            }
        }
    }

    // Kiểm tra floatting_2
    if (Amidite_Floatting2.length() == 1) {
        QChar char2 = Amidite_Floatting2[0];
        for (char c : amidite) {
            if (char2 == c) {
                ui->lineEdit_float_2->setText("U");
                break;
            }
        }
    }
    amidite_.sequence_floatting_1 = ui->lineEdit_float_1->text();
    amidite_.sequence_floatting_2 = ui->lineEdit_float_2->text();
    amidite_.saveAmiditeFloatting(amidite_.sequence_floatting_1 , amidite_.sequence_floatting_2);

    ui->cbx_valve_selected->clear();
    ui->cbx_type_chemical_manual_run->clear();
    ui->cbx_type_chemical_mix_1->clear();
    ui->cbx_type_chemical_mix_2->clear();
    ui->cbx_type_chemical_mix_3->clear();
    ui->cbx_type_reagentDelivery->clear();
    ui->cbx_coupling2Option->clear();

    ui->cbx_valve_selected->addItems(amidite_.bottleNamesListFull);
    ui->cbx_type_chemical_manual_run->addItems(amidite_.bottleNamesListFull);
    amidite_.bottleNamesListFull.append("AMIDITE"); // thêm từ floatting amidte
    ui->cbx_type_chemical_mix_1->addItems(amidite_.bottleNamesListFull);
    ui->cbx_type_chemical_mix_2->addItems(amidite_.bottleNamesListFull);
    ui->cbx_type_chemical_mix_3->addItems(amidite_.bottleNamesListFull);
    ui->cbx_type_reagentDelivery->addItems(amidite_.bottleNamesListFull);
    ui->cbx_coupling2Option->addItems(amidite_.bottleNamesListAmidite);
}

void SYNO24::on_btn_panel_connect_released()
{
#ifdef DEBUG_SOFTWARE
    //ui->stackedWidget_home->setCurrentIndex(0);
#else
    if(STM32_COM.flag_connecttion == true)
    {
        ui->stackedWidget_home->setCurrentIndex(0);
    }
#endif

}


void SYNO24::on_btn_panel_control_machine_released()
{
#ifdef DEBUG_SOFTWARE
    //ui->stackedWidget_home->setCurrentIndex(1);
#else
    if(STM32_COM.flag_connecttion == true)
    {
        ui->stackedWidget_home->setCurrentIndex(1);
    }
#endif

    // ui->stackedWidget_home->setCurrentIndex(1);

}


void SYNO24::on_btn_settingcfg_valve_released()
{
#ifdef DEBUG_SOFTWARE
    //ui->stackedWidget_home->setCurrentIndex(2);
#else
    if(STM32_COM.flag_connecttion == true)
    {
        ui->stackedWidget_home->setCurrentIndex(2);
    }
#endif
}


void SYNO24::on_tabWidget_main_currentChanged(int index)
{
#ifdef DEBUG_SOFTWARE

#else
    if(index == SYSTEM_TAB)
    {
        if(syno24_machine.getAutoState() == State::STOPED && syno24_machine.getManualState() == State::STOPED)
        {

            ui->tabWidget_main->setCurrentIndex(SYSTEM_TAB);
        }
        else
        {
            ui->tabWidget_main->setCurrentIndex(RUN_TAB);
        }
    }
    else
    {
        //ui->tabWidget_main->setCurrentIndex();
    }

    if(index == RUN_TAB && STM32_COM.flag_connecttion != true)
    {
        QMessageBox::critical(this, tr("Error"), "PLEASE CONNECT WITH SYNO24X");
        ui->tabWidget_main->setCurrentIndex(0);
    }
    // mở comment này để bật lại tính năng không cho chuyển  17-03-2025
    // phía trên cần xem lại nha
    //    if(index == RUN_TAB && STM32_COM.flag_connecttion != true)
    //    {
    //        QMessageBox::critical(this, tr("Error"), "PLEASE CONNECT WITH SYNO24X");
    //        ui->tabWidget_main->setCurrentIndex(0);
    //    }
#endif
}

void SYNO24:: updateControlsState()
{
    if(STM32_COM.flag_connecttion != true)
    {
        ui->tabWidget_main->setCurrentIndex(0);
    }
}


void SYNO24::log_debug(const QString &message)
{
    // Get the current timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    // Format the log message with the timestamp
    QString formattedMessage = QString("[%1] %2").arg(timestamp, message);

    // Append the log message to the textEdit
    ui->textEdit_status_update_fw->append(formattedMessage);
}


void SYNO24::log_terminal_withTimestamp(const QString &message, int level)
{
    // One Half Dark color scheme
    static const QColor colors[] = {
        QColor("#abb2bf"), // Normal (Black)
        QColor("#e45649"), // Error (Red)
        QColor("#3ff44f"), // Success (Green)
        QColor("#c18401"), // Warning (Yellow)
        QColor("#38ccd1"), // Info (Blue)
        QColor("#a626a4"), // Debug (Magenta)
        QColor("#ffa500"), // Trace (Cyan)
        QColor("#fafafa")  // Default (White)
    };

    // Get the current timestamp
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    // Format the log message with the timestamp
    QString formattedMessage = QString("[%1] %2").arg(timestamp, message);

    // Ensure level is within range
    int colorIndex = qBound(0, level, 7);

    // Set text format based on log level
    QTextCharFormat format;
    format.setForeground(colors[colorIndex]);

    // Prepare the cursor to insert text
    QTextCursor cursor(ui->textEdit_oligo_history_log->textCursor());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(formattedMessage + "\n", format);

    // Auto-scroll to the end
    ui->textEdit_oligo_history_log->setTextCursor(cursor);
}


/**
 * log terminal có xuống dòng không kèm thời gian
 */
void SYNO24::log_terminal(const QString &message, LogLevel level)
{
    // One Half Dark color scheme
    static const QColor colors[] = {
        QColor("#abb2bf"), // Normal (Black)
        QColor("#e45649"), // Error (Red)
        QColor("#3ff44f"), // Success (Green)
        QColor("#c18401"), // Warning (Yellow)
        QColor("#38ccd1"), // Info (Blue)
        QColor("#a626a4"), // Debug (Magenta)
        QColor("#ffa500"), // Trace (Cyan)
        QColor("#fafafa")  // Default (White)
    };

    // Ensure level is within range
    int colorIndex = static_cast<int>(level);
    colorIndex = qBound(0, colorIndex, 7);

    // Set text format based on log level
    QTextCharFormat format;
    format.setForeground(colors[colorIndex]);

    // Prepare the cursor to insert text
    QTextCursor cursor(ui->textEdit_oligo_history_log->textCursor());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(message + "\n", format);

    // Auto-scroll to the end
    ui->textEdit_oligo_history_log->setTextCursor(cursor);
}

void SYNO24::log_terminal_append(const QString &message, LogLevel level, bool newLine)
{
    // One Half Dark color scheme
    static const QColor colors[] = {
        QColor("#abb2bf"), // Normal (Black)
        QColor("#e45649"), // Error (Red)
        QColor("#3ff44f"), // Success (Green)
        QColor("#c18401"), // Warning (Yellow)
        QColor("#38ccd1"), // Info (Blue)
        QColor("#a626a4"), // Debug (Magenta)
        QColor("#ffa500"), // Trace (Cyan)
        QColor("#fafafa")  // Default (White)
    };

    // Ensure level is within range
    int colorIndex = static_cast<int>(level);
    colorIndex = qBound(0, colorIndex, 7);

    // Set text format based on log level
    QTextCharFormat format;
    format.setForeground(colors[colorIndex]);

    // Append the message
    QTextCursor cursor(ui->textEdit_oligo_history_log->textCursor());
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(message, format);

    // Optionally add a newline
    if (newLine) {
        cursor.insertText("\n");
    }

    // Auto-scroll to the end
    ui->textEdit_oligo_history_log->setTextCursor(cursor);
}

void SYNO24::ManualControlSystem()
{
    //  FAN_SV = 0, // 33 xa khi nito giam do am
    //	LED_RED_SV  = 1, // 34
    //	LED_GREEN_SV  = 2, // 35
    //	V38_EMPTY  = 3, // 36  high push
    //	LOW_PUSH_SV  = 4,// V37
    //	HIGH_PUSH_SV = 5,
    //	V39_EMPTY = 6,
    //	OPEN_NITOR_SV  = 7,
    //qDebug()<< "Control MANUAL System ";
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    Command_send[0] = CMD_CONTROL_SYNCHRONIZE_IO; // RUN STEPPER
    Command_send[1] = ui->checkbx_control_V_1->isChecked() ? 1 : 0;
    Command_send[1] = ui->checkbx_control_V_1->isChecked()? 1 : 0;
    Command_send[2] = ui->checkbx_control_V_2->isChecked()? 1 : 0;
    Command_send[3] = ui->checkbx_control_V_3->isChecked()? 1 : 0;
    Command_send[4] = ui->checkbx_control_V_4->isChecked()? 1 : 0;
    Command_send[5] = ui->checkbx_control_V_5->isChecked()? 1 : 0;
    Command_send[6] = ui->checkbx_control_V_6->isChecked()? 1 : 0;
    Command_send[7] = ui->checkbx_control_V_7->isChecked()? 1 : 0;
    Command_send[8] = ui->checkbx_control_V_8->isChecked()? 1 : 0;
    Command_send[9] = ui->checkbx_control_V_9->isChecked()? 1 : 0;
    Command_send[10] = ui->checkbx_control_V_10->isChecked()? 1 : 0;
    Command_send[11] = ui->checkbx_control_V_11->isChecked()? 1 : 0;
    Command_send[12] = ui->checkbx_control_V_12->isChecked()? 1 : 0;
    Command_send[13] = ui->checkbx_control_V_13->isChecked()? 1 : 0;
    Command_send[14] = ui->checkbx_control_V_14->isChecked()? 1 : 0;
    Command_send[15] = ui->checkbx_control_V_15->isChecked()? 1 : 0;
    Command_send[16] = ui->checkbx_control_V_16->isChecked()? 1 : 0;
    Command_send[17] = ui->checkbx_control_V_17->isChecked()? 1 : 0;

    Command_send[18] = ui->btnFAN->isChecked()? 1 : 0;
    //Command_send[19] =  // RED LED
    //Command_send[20] //3 // GREEN LED
    Command_send[21] = ui->btn_FanVacuumBox->isChecked();// fan in box
    Command_send[22] = ui->btnLowPushSV->isChecked();//5
    Command_send[23] = ui->btn_HighPushSV->isChecked();//6
    //Command_send[24] // empty//7
    Command_send[25] =  ui->btnOpenAirNitor->isChecked();//8

    //    for(int i = 1; i < 30; i++)
    //    {
    //        qDebug()<< "index" << i<< static_cast<int>(Command_send[i]);
    //    }
    STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 1000);
}
/*

void SYNO24::writeSettings() {
    QSettings settings("config.ini", QSettings::IniFormat);

    // Lưu cổng COM hiện tại
    settings.beginGroup("SerialPort");
    settings.setValue("LastPort", STM32_COM.currentPort);
    settings.endGroup();

    // Lưu coupling1List, coupling2List và couplingSpecialBaseList
    settings.beginGroup("Coupling");

    QStringList coupling1Str;
    for (quint8 val : coupling1List)
        coupling1Str << QString::number(val);
    settings.setValue("Coupling1", coupling1Str.join(","));

    QStringList coupling2Str;
    for (quint8 val : coupling2List)
        coupling2Str << QString::number(val);
    settings.setValue("Coupling2", coupling2Str.join(","));

    QStringList specialStr;
    for (quint8 val : couplingSpecialBaseList)
        specialStr << QString::number(val);
    settings.setValue("CouplingSpecial", specialStr.join(","));

    settings.endGroup();

    qDebug() << "Coupling 1 List:" << coupling1List;
    qDebug() << "Coupling 2 List:" << coupling2List;
    qDebug() << "Special Base List:" << couplingSpecialBaseList;
    qDebug() << "[INFO] Saved LastPort to config.ini:" << STM32_COM.currentPort;
}
void SYNO24::readSettings() {
    QSettings settings("config.ini", QSettings::IniFormat);

    // Đọc cổng COM cũ
    settings.beginGroup("SerialPort");
    QString oldPort = settings.value("LastPort", "").toString(); // Mặc định là chuỗi rỗng nếu không có giá trị
    settings.endGroup();

    // Đọc danh sách coupling từ file config
    settings.beginGroup("Coupling");

    QString coupling1Str = settings.value("Coupling1").toString();
    QString coupling2Str = settings.value("Coupling2").toString();
    QString couplingSpecialStr = settings.value("CouplingSpecial").toString();

    settings.endGroup();

    coupling1List.clear();
    coupling2List.clear();
    couplingSpecialBaseList.clear();

    // Phân tích coupling1
    if (!coupling1Str.isEmpty()) {
        QStringList parts = coupling1Str.split(",");
        for (const QString& s : parts) {
            bool ok;
            int val = s.toInt(&ok);
            if (ok && val >= 0 && val <= 255)
                coupling1List.append(static_cast<quint8>(val));
        }
    }

    // Phân tích coupling2
    if (!coupling2Str.isEmpty()) {
        QStringList parts = coupling2Str.split(",");
        for (const QString& s : parts) {
            bool ok;
            int val = s.toInt(&ok);
            if (ok && val >= 0 && val <= 255)
                coupling2List.append(static_cast<quint8>(val));
        }
    }

    // Phân tích couplingSpecial
    if (!couplingSpecialStr.isEmpty()) {
        QStringList parts = couplingSpecialStr.split(",");
        for (const QString& s : parts) {
            bool ok;
            int val = s.toInt(&ok);
            if (ok && val >= 0 && val <= 255)
                couplingSpecialBaseList.append(static_cast<quint8>(val));
        }
    }

    updateTableWithCouplingLists();
    updateTableWithSpecialList(); // Cập nhật bảng đặc biệt

    // Auto open port
    if (!oldPort.isEmpty()) {
        qDebug() << "[INFO] Loaded LastPort from config.ini:" << oldPort;
        scanAndSelectPort(oldPort);
        fnc_openSerialPort();
        STM32_COM.currentPort = oldPort;
    } else {
        qDebug() << "[WARNING] No LastPort found in config.ini";
    }

    qDebug() << "Coupling 1 List:" << coupling1List;
    qDebug() << "Coupling 2 List:" << coupling2List;
    qDebug() << "Special Base List:" << couplingSpecialBaseList;
    qDebug() << "[INFO] Loaded Coupling Lists from config.ini:";
}
*/
void SYNO24::writeSettings() {
    QSettings settings("config.ini", QSettings::IniFormat);

    // Lưu cổng COM hiện tại
    settings.beginGroup("SerialPort");
    settings.setValue("LastPort", STM32_COM.currentPort);
    settings.endGroup();

    // Lưu coupling1List, coupling2List và couplingSpecialBaseList
    settings.beginGroup("Coupling");

    QStringList coupling1Str;
    for (quint8 val : coupling1List)
        coupling1Str << QString::number(val);
    settings.setValue("Coupling1", coupling1Str.join(","));

    QStringList coupling2Str;
    for (quint8 val : coupling2List)
        coupling2Str << QString::number(val);
    settings.setValue("Coupling2", coupling2Str.join(","));

    QStringList specialStr;
    for (quint8 val : couplingSpecialBaseList)
        specialStr << QString::number(val);
    settings.setValue("CouplingSpecial", specialStr.join(","));


    settings.endGroup();

    // === THÊM PHẦN LƯU CÁC WIDGET VACUUM BOX ===
    settings.beginGroup("VacuumBox");

    // Lưu các checkbox
    settings.setValue("EnableVacuumBox", ui->chkbxEnaVaccuumBox->isChecked() ? 1 : 0);
    settings.setValue("WashingVacuum", ui->chkbxVacuumBoxWashing->isChecked() ? 1 : 0);
    settings.setValue("DeblockVacuum", ui->chkbxVacuumBoxDeblock->isChecked() ? 1 : 0);
    settings.setValue("CappingVacuum", ui->chkbxVacuumBoxCapping->isChecked() ? 1 : 0);
    settings.setValue("CouplingVacuum", ui->chkbxVacuumBoxCoupling->isChecked() ? 1 : 0);
    settings.setValue("OxVacuum", ui->chkbxVacuumBoxOx->isChecked() ? 1 : 0);

    // Lưu spinbox thời gian
    settings.setValue("FanTime", ui->spbx_time_FanVacuumBox->value());
    // === THÊM PHẦN LƯU TEXT TỪ LINE EDIT ===
    settings.setValue("SpecialBaseText", ui->lineEdit_special_base_VacuumBox->text()); 
    settings.endGroup();

    // Lưu dữ liệu bảng
    // Lưu phần nhân sức đẩy theo tỉ lệ %
    settings.beginGroup("TableMultiplier");
    settings.remove(""); // Xóa dữ liệu cũ trước khi lưu mới

    for (size_t i = 0; i < m_tableDataMultiplier.rows.size(); ++i) {
        const auto& row = m_tableDataMultiplier.rows[i];
        settings.beginGroup(QString::number(i));
        settings.setValue("From", row.from);
        settings.setValue("To", row.to);
        settings.setValue("Multiplier", row.multiplier);
        settings.endGroup();
    }
    settings.setValue("HighPush", ui->chkbx_HighPushMul->isChecked() ? 1 : 0);
    settings.setValue("LowPush", ui->chkbx_LowPushMul->isChecked() ? 1 : 0);
    // Lưu tổng số hàng
    settings.setValue("RowCount", static_cast<int>(m_tableDataMultiplier.rows.size()));
    settings.endGroup();
    qDebug() << "Coupling 1 List:" << coupling1List;
    qDebug() << "Coupling 2 List:" << coupling2List;
    qDebug() << "Special Base List:" << couplingSpecialBaseList;
    qDebug() << "[INFO] Saved LastPort to config.ini:" << STM32_COM.currentPort;
}

void SYNO24::readSettings() {
    QSettings settings("config.ini", QSettings::IniFormat);

    // Đọc cổng COM cũ
    settings.beginGroup("SerialPort");
    QString oldPort = settings.value("LastPort", "").toString(); // Mặc định là chuỗi rỗng nếu không có giá trị
    settings.endGroup();

    // Đọc danh sách coupling từ file config
    settings.beginGroup("Coupling");

    QString coupling1Str = settings.value("Coupling1").toString();
    QString coupling2Str = settings.value("Coupling2").toString();
    QString couplingSpecialStr = settings.value("CouplingSpecial").toString();

    settings.endGroup();

    coupling1List.clear();
    coupling2List.clear();
    couplingSpecialBaseList.clear();

    // Phân tích coupling1
    if (!coupling1Str.isEmpty()) {
        QStringList parts = coupling1Str.split(",");
        for (const QString& s : parts) {
            bool ok;
            int val = s.toInt(&ok);
            if (ok && val >= 0 && val <= 255)
                coupling1List.append(static_cast<quint8>(val));
        }
    }

    // Phân tích coupling2
    if (!coupling2Str.isEmpty()) {
        QStringList parts = coupling2Str.split(",");
        for (const QString& s : parts) {
            bool ok;
            int val = s.toInt(&ok);
            if (ok && val >= 0 && val <= 255)
                coupling2List.append(static_cast<quint8>(val));
        }
    }

    // Phân tích couplingSpecial
    if (!couplingSpecialStr.isEmpty()) {
        QStringList parts = couplingSpecialStr.split(",");
        for (const QString& s : parts) {
            bool ok;
            int val = s.toInt(&ok);
            if (ok && val >= 0 && val <= 255)
                couplingSpecialBaseList.append(static_cast<quint8>(val));
        }
    }

    // === THÊM PHẦN ĐỌC CÁC WIDGET VACUUM BOX ===
    settings.beginGroup("VacuumBox");
    // === THÊM PHẦN ĐỌC TEXT CHO LINE EDIT ===
    QString specialBaseText = settings.value("SpecialBaseText", "").toString();
    // Đọc các checkbox
    bool enableVacuum = settings.value("EnableVacuumBox", 0).toInt() == 1;
    bool washingVacuum = settings.value("WashingVacuum", 0).toInt() == 1;
    bool deblockVacuum = settings.value("DeblockVacuum", 0).toInt() == 1;
    bool cappingVacuum = settings.value("CappingVacuum", 0).toInt() == 1;
    bool couplingVacuum = settings.value("CouplingVacuum", 0).toInt() == 1;
    bool oxVacuum = settings.value("OxVacuum", 0).toInt() == 1;

    // Đọc spinbox thời gian
    int fanTime = settings.value("FanTime", 10).toInt(); // Mặc định 30 giây
    // === ÁP DỤNG TEXT CHO LINE EDIT ===
    ui->lineEdit_special_base_VacuumBox->setText(specialBaseText);
    settings.endGroup();

    // Áp dụng giá trị cho các widget
    ui->chkbxEnaVaccuumBox->setChecked(enableVacuum);
    ui->chkbxVacuumBoxWashing->setChecked(washingVacuum);
    ui->chkbxVacuumBoxDeblock->setChecked(deblockVacuum);
    ui->chkbxVacuumBoxCapping->setChecked(cappingVacuum);
    ui->chkbxVacuumBoxCoupling->setChecked(couplingVacuum);
    ui->chkbxVacuumBoxOx->setChecked(oxVacuum);
    ui->spbx_time_FanVacuumBox->setValue(fanTime);

    updateTableWithCouplingLists();
    updateTableWithSpecialList(); // Cập nhật bảng đặc biệt

    // Auto open port
    if (!oldPort.isEmpty()) {
        qDebug() << "[INFO] Loaded LastPort from config.ini:" << oldPort;
        scanAndSelectPort(oldPort);
        fnc_openSerialPort();
        STM32_COM.currentPort = oldPort;
    } else {
        qDebug() << "[WARNING] No LastPort found in config.ini";
    }

    qDebug() << "Coupling 1 List:" << coupling1List;
    qDebug() << "Coupling 2 List:" << coupling2List;
    qDebug() << "Special Base List:" << couplingSpecialBaseList;
    qDebug() << "[INFO] Loaded Coupling Lists from config.ini:";

    // Log thông tin vacuum box
    qDebug() << "[INFO] Loaded Vacuum Box Settings:";
    qDebug() << "  Enable Vacuum Box:" << enableVacuum;
    qDebug() << "  Washing Vacuum:" << washingVacuum;
    qDebug() << "  Deblock Vacuum:" << deblockVacuum;
    qDebug() << "  Capping Vacuum:" << cappingVacuum;
    qDebug() << "  Coupling Vacuum:" << couplingVacuum;
    qDebug() << "  Ox Vacuum:" << oxVacuum;
    qDebug() << "  Fan Time:" << fanTime;

    // Đọc dữ liệu bảng
    settings.beginGroup("TableMultiplier");

    // Xóa dữ liệu cũ
    m_tableDataMultiplier.rows.clear();

    // Đọc số hàng
    int rowCount = settings.value("RowCount", 0).toInt();

    // Đọc từng hàng
    for (int i = 0; i < rowCount; ++i) {
        QString groupKey = QString::number(i);

        if (settings.childGroups().contains(groupKey)) {
            settings.beginGroup(groupKey);

            TableRowData row;
            row.from = settings.value("From", 0).toInt();
            row.to = settings.value("To", 0).toInt();
            row.multiplier = settings.value("Multiplier", 0).toInt();

            settings.endGroup();

            m_tableDataMultiplier.rows.push_back(row);

            qDebug() << "[DEBUG] Loaded row" << i
                     << "From:" << row.from
                     << "To:" << row.to
                     << "Multiplier:" << row.multiplier;
        }
    }
    bool HighPushEn = settings.value("HighPush", 0).toInt() == 1;
    bool LowPushEn = settings.value("LowPush", 0).toInt() == 1;
    ui->chkbx_HighPushMul->setChecked(HighPushEn);
    ui->chkbx_LowPushMul->setChecked(LowPushEn);
    settings.endGroup();
    // Cập nhật lại TableView
    updateTableMultiplierView();
    qDebug() << "[INFO] Loaded" << m_tableDataMultiplier.rows.size() << "rows from config.ini.";
}

void SYNO24::on_btn_new_sequence_released()
{
    QString saveFileName = QFileDialog::getSaveFileName(this,
                                                        tr("Save Json File"),
                                                        filemanager.amidite_sequence_Dir,
                                                        tr("JSON (*.json)"));
    QFile File_Save(saveFileName);
    if( File_Save.open( QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate ) )
    {
        File_Save.close();
        filemanager.amidite_sequence_Path = saveFileName;
        ui->lineEdit_path_sequence->setText(saveFileName);
    }
    else
    {
        QMessageBox::warning(this, "Warning", " Can't Create or Read this file, Please try again!!!");
    }
}


void SYNO24::on_btnHomeCalibration_released()
{
    QByteArray Command_send(LENGTH_COMMAND_SEND,0);
    Command_send[0] = CMD_RUN2HOME; // RUN STEPPER
    STM32_COM.serial_send_command_Firmware(serialPort, Command_send, 20000);
}


void SYNO24::startCountdown(float interval_time) {

    secondsRemaining = interval_time;
    timerEstimate->start(1000);
}

void SYNO24::updateCountdown() {
    if (secondsRemaining > 0) {
        secondsRemaining--;
        int hours = secondsRemaining / 3600; // Tính số giờ
        int minutes = (secondsRemaining % 3600) / 60; // Tính số phút còn lại
        int seconds = secondsRemaining % 60; // Tính số giây còn lại
        // Định dạng chuỗi hiển thị giờ:phút:giây
        QString displayText = QString("%1:%2:%3")
                .arg(hours, 2, 10, QChar('0'))
                .arg(minutes, 2, 10, QChar('0'))
                .arg(seconds, 2, 10, QChar('0'));
        // Hiển thị chuỗi trên LCD
        ui->lcdNumber->display(displayText);
    } else {
        timerEstimate->stop(); // Dừng bộ đếm khi hết giờ
        // Định dạng chuỗi hiển thị giờ:phút:giây
        QString displayText = QString("%1:%2:%3")
                .arg(0, 2, 10, QChar('0'))
                .arg(0, 2, 10, QChar('0'))
                .arg(0, 2, 10, QChar('0'));
        // Hiển thị chuỗi trên LCD
        ui->lcdNumber->display(displayText);
    }
}

void SYNO24::stopCountdown() {
    timerEstimate->stop();  // Dừng timer

    // Đặt LCD về 00:00:00
    QString displayText = QString("%1:%2:%3")
            .arg(0, 2, 10, QChar('0'))
            .arg(0, 2, 10, QChar('0'))
            .arg(0, 2, 10, QChar('0'));
    ui->lcdNumber->display(displayText);
}

void SYNO24::pauseCountdown() {
    if (!isPausedEstimate && timerEstimate->isActive()) {
        isPausedEstimate = true;
        timerEstimate->stop();
    }
}

void SYNO24::resumeCountdown() {
    if (isPausedEstimate && secondsRemaining > 0) {
        isPausedEstimate = false;
        timerEstimate->start(1000);
    }
}

void SYNO24::CalEstimateTimeProtocol(quint32 *TimeEstimate)
{

    quint8 u8_number_step_run =  0;
    quint8 u8_number_base_run =  0;
    quint16 u16_counter_base_finished = 1;
    quint8 u8_lastest_sub = 0;
    quint16 u16_max_base_setting_protocol = 0;
    quint8 u8_number_sub_run = ui->spbox_number_sub->value(); // lấy số sub cần Run
    quint32 u32_time_oligo_process_step;
    quint64 u32_time_oligo_process_base;
    quint64 u32_time_oligo_process_protocol;
    protocol_oligo.u8_number_sub = u8_number_sub_run;
#ifdef  MAIN_WINDOW_DEBUG
    //qDebug()<< "CalEstimateTimeProtocol : ";
    // qDebug()<< "u16_max_sequence_amidite_setting" << global_var.signal_status_oligo.u16_max_sequence_amidite_setting;
#endif

    for(uint8_t ctn_sub = 0; ctn_sub < protocol_oligo.u8_number_sub; ctn_sub++)
    {
        u16_max_base_setting_protocol = u16_max_base_setting_protocol + protocol_oligo.sub[ctn_sub].u8_number_base_on_sub;
        if(u16_max_base_setting_protocol < global_var.signal_status_oligo.u16_max_sequence_amidite_setting)
        {
            if(protocol_oligo.u8_number_sub == 1) // truong hop chi cai dat 1 sub thi sub cuoi cung chinh là sub 0
            {
                u8_lastest_sub = 0;
            }
            else // trường hợp có nhiều sub phía sau thì cứ lấy sub tiếp theo mà chạy
            {
                if(ctn_sub < (protocol_oligo.u8_number_sub -1))
                {
                    u8_lastest_sub = ctn_sub + 1;
                }
                else
                {
                    u8_lastest_sub = ctn_sub;
                }
            }
        }
    }
    // kiểm tra xem sequence có dài hơi là protocol không
    int int_remain_base = global_var.signal_status_oligo.u16_max_sequence_amidite_setting - u16_max_base_setting_protocol;
    if(int_remain_base > 0) // sequence dài hơn protocol rồi // tự động tăng page cho sub
    {
        protocol_oligo.sub[u8_lastest_sub].u8_number_base_on_sub += int_remain_base;
#ifdef  MAIN_WINDOW_DEBUG
        // qDebug()<< "sub cuoi cung" << u8_lastest_sub;
        // qDebug()<< "so base cua sub cuoi cung" << protocol_oligo.sub[u8_lastest_sub].u8_number_base_on_sub;
#endif
    }
    else
    {

    }
    u32_time_oligo_process_protocol = 0;
    for(uint8_t u8_counter_sub_run = 0; u8_counter_sub_run < u8_number_sub_run; u8_counter_sub_run++)
    {
        // lấy số base của sub-protocol
        u8_number_base_run = protocol_oligo.sub[u8_counter_sub_run].u8_number_base_on_sub;
        //qDebug()<< "u8_number_base_run" << u8_number_base_run;
        u32_time_oligo_process_base = 0;
        for(quint8 u8_counter_base_on_sub = 0; u8_counter_base_on_sub < u8_number_base_run; u8_counter_base_on_sub++)
        {
            if(u16_counter_base_finished < global_var.signal_status_oligo.u16_max_sequence_amidite_setting)
            {
#ifdef  MAIN_WINDOW_DEBUG
                //        qDebug()<< "u16_counter_base_finished" << u16_counter_base_finished;
#endif

                u8_number_step_run = protocol_oligo.sub[u8_counter_sub_run].u8_number_step_on_base;
                u32_time_oligo_process_step = 0;// ofset for running time // cần phải tính toán thật kĩ cho từng base
                for(quint8 idx = 0; idx < MAX_WELL_AMIDITE; idx++)
                {
                    if(global_var.amidite_well[idx].u8_sequence[u8_counter_base_on_sub] != CHEMICAL_SUBTANCE_EMPTY)
                    {
                        u32_time_oligo_process_step = u32_time_oligo_process_step + 1200;//  + thêm thời gian chạy di chuyển đến giếng
                        u32_time_oligo_process_step = u32_time_oligo_process_step + 800;//  + thêm thời gian bơm hóa chất
                    }
                }
                for(quint8 u8_counter_step = 0; u8_counter_step < u8_number_step_run; u8_counter_step++)
                {
                    u32_time_oligo_process_step += protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_wait_after_fill.Data;
                    for(uint8_t idx_process = 0; idx_process < 10; idx_process++)
                    {
                        u32_time_oligo_process_step +=  protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_procs_time[idx_process].Data;
                        u32_time_oligo_process_step +=  protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[idx_process].Data * SCALE_LEVEL_WAITTIME;
                    }
                    if(u8_counter_base_on_sub == 0 && protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u8_first_type_chemical == COUPLING) // neu co coupling thi base dau tien se them primming + bu thời gian này thêm
                    {
                        u32_time_oligo_process_step = u32_time_oligo_process_step * 2;
                    }
                }
                u32_time_oligo_process_base += (u32_time_oligo_process_step / 1000); // millis to seconds
                u16_counter_base_finished++;
            }
#ifdef  MAIN_WINDOW_DEBUG
            //    qDebug()<< "Base : " << u8_counter_base_on_sub+ 1 << "Sub : " << u8_counter_sub_run + 1<< "Time Estimate : " << u32_time_oligo_process_base;
#endif

        }
        u32_time_oligo_process_protocol += u32_time_oligo_process_base;
#ifdef  MAIN_WINDOW_DEBUG
        //  qDebug()<<"Sub : " << u8_counter_sub_run + 1 << "Time Estimate" << u32_time_oligo_process_protocol; // seconds
#endif

    }
    *TimeEstimate = u32_time_oligo_process_protocol;
}


QString SYNO24:: convertSecondsToHHMMSS(quint32 TimeEstimate) {
    // Chuyển đổi giây thành giờ, phút, giây
    int hours = TimeEstimate / 3600;
    int minutes = (TimeEstimate % 3600) / 60;
    int seconds = TimeEstimate % 60;

    // Sử dụng QTime để định dạng hh:mm:ss
    return QString("%1:%2:%3")
            .arg(hours, 2, 10, QChar('0'))
            .arg(minutes, 2, 10, QChar('0'))
            .arg(seconds, 2, 10, QChar('0'));
}

void SYNO24::on_btnCopySub_released()
{
    // Hiển thị hộp thoại chọn SubProtocol nguồn và đích
    bool ok;
    QStringList subProtocolNames;
    for(int i = 0; i <  protocol_oligo.u8_number_sub; i++)
    {
        subProtocolNames << ( radioSubsellect[i]->text());
    }

    QString sourceName = QInputDialog::getItem(this, "Select Source SubProtocol", // Window title
                                               "Source SubProtocol:", // Label prompt
                                               subProtocolNames, 0, false, &ok);
    if (!ok || sourceName.isEmpty()) {
        qWarning() << "No source SubProtocol selected!"; // Warning message
        return;
    }

    QString destinationName = QInputDialog::getItem(this, "Select Destination SubProtocol", // Window title
                                                    "Destination SubProtocol:", // Label prompt
                                                    subProtocolNames, 0, false, &ok);
    if (!ok || destinationName.isEmpty()) {
        qWarning() << "No destination SubProtocol selected!"; // Warning message
        return;
    }

    // Lấy chỉ số của SubProtocol nguồn và đích
    int sourceIndex = subProtocolNames.indexOf(sourceName);
    int destinationIndex = subProtocolNames.indexOf(destinationName);
    // Copy
    copy_sub_protocol_data(protocol_oligo.sub[destinationIndex], protocol_oligo.sub[sourceIndex]);
    //on_btn_savecurrent_step_released();
    load_protocol_to_ui(destinationIndex, 0);
    Display_Protocol_to_user();
    calculator_volume_and_process_UI();
}

void SYNO24::on_btnDeleteSub_released()
{
    // Hiển thị hộp thoại chọn SubProtocol cần xóa
    bool ok;
    QStringList subProtocolNames;
    for(int i = 0; i < protocol_oligo.u8_number_sub; i++)
    {
        subProtocolNames << radioSubsellect[i]->text();
    }

    QString subToDelete = QInputDialog::getItem(this, "Select SubProtocol to Delete",
                                                "SubProtocol:", subProtocolNames, 0, false, &ok);
    if (!ok || subToDelete.isEmpty()) {
        qWarning() << "No SubProtocol selected for deletion!";
        return;
    }

    int indexToDelete = subProtocolNames.indexOf(subToDelete);

    // Xác nhận người dùng có chắc chắn muốn xóa không
    QMessageBox::StandardButton confirm = QMessageBox::question(this, "Confirm Deletion",
                                                                QString("Are you sure you want to delete SubProtocol '%1'?").arg(subToDelete),
                                                                QMessageBox::Yes | QMessageBox::No);
    if (confirm != QMessageBox::Yes) {
        return;
    }

    // Xóa và dồn dữ liệu
    for (int i = indexToDelete; i < protocol_oligo.u8_number_sub - 1; ++i)
    {
        copy_sub_protocol_data(protocol_oligo.sub[i], protocol_oligo.sub[i + 1]);
        //radioSubsellect[i]->setText(radioSubsellect[i + 1]->text());
    }

    // Giảm số lượng SubProtocol
    protocol_oligo.u8_number_sub--;

    // Cập nhật giao diện
    Display_Protocol_to_user();
    calculator_volume_and_process_UI();

    qDebug() << "SubProtocol deleted successfully.";
}



void SYNO24::copy_sub_protocol_data(sub_protocol_t &dest, const sub_protocol_t &src)
{
    for (int i = 0; i < MAX_STEP_OF_SUB; ++i) {
        dest.step[i] = src.step[i]; // Deep copy to ensure independent data
    }
    // Copy other members
    dest.u8_number_base_on_sub = src.u8_number_base_on_sub;
    dest.u8_number_step_on_base = src.u8_number_step_on_base;
    dest.douple_coupling_option = src.douple_coupling_option;

}

void SYNO24::copyAndInsertStep(int sourceSubIndex, int sourceStepIndex, int destSubIndex, int insertionIndex) {
    // --- Kiểm tra tính hợp lệ của chỉ số SubProtocol nguồn và đích ---
    if (sourceSubIndex < 0 || sourceSubIndex >= protocol_oligo.u8_number_sub ||
            destSubIndex < 0 || destSubIndex >= protocol_oligo.u8_number_sub) {
        qWarning() << "copyAndInsertStep - Invalid source or destination sub-protocol index.";
        QMessageBox::critical(this, "Critical Error", "Invalid source or destination SubProtocol index provided.");
        return;
    }

    // --- Lấy tham chiếu đến SubProtocol nguồn và đích ---
    sub_protocol_t &sourceSub = protocol_oligo.sub[sourceSubIndex];
    sub_protocol_t &destSub = protocol_oligo.sub[destSubIndex];

    // --- Kiểm tra tính hợp lệ của chỉ số Step nguồn ---
    if (sourceStepIndex < 0 || sourceStepIndex >= sourceSub.u8_number_step_on_base) {
        qWarning() << "copyAndInsertStep - Invalid source step index:" << sourceStepIndex;
        // Thông báo lỗi rõ ràng hơn
        QMessageBox::warning(this, "Error", QString("Invalid source Step index (%1) in Sub %2!")
                             .arg(sourceStepIndex + 1).arg(sourceSubIndex + 1));
        return;
    }

    // --- Kiểm tra tính hợp lệ của vị trí chèn trong SubProtocol đích ---
    // Vị trí chèn hợp lệ từ 0 đến số step hiện có (để chèn vào cuối)
    if (insertionIndex < 0 || insertionIndex > destSub.u8_number_step_on_base) {
        qWarning() << "copyAndInsertStep - Invalid insertion index:" << insertionIndex << "for destination sub" << destSubIndex;
        QMessageBox::warning(this, "Error", QString("Invalid insertion position (%1) in Sub %2!")
                             .arg(insertionIndex + 1).arg(destSubIndex + 1));
        return;
    }

    // --- Kiểm tra xem SubProtocol đích đã đầy chưa ---
    if (destSub.u8_number_step_on_base >= MAX_STEP_OF_SUB) {
        qWarning() << "copyAndInsertStep - Destination sub-protocol" << destSubIndex << "is full.";
        QMessageBox::warning(this, "Error", QString("Cannot insert Step: Destination SubProtocol (Sub %1) is full!")
                             .arg(destSubIndex + 1));
        return;
    }

    // --- Thực hiện sao chép và chèn ---
    // 1. Lấy bản sao của dữ liệu Step nguồn
    Step_process_parameter_t sourceStepData = sourceSub.step[sourceStepIndex];

    // 2. Dịch chuyển các Step trong SubProtocol đích để tạo không gian
    // Lặp ngược từ cuối về vị trí chèn
    for (int i = destSub.u8_number_step_on_base - 1; i >= insertionIndex; --i) {
        destSub.step[i + 1] = destSub.step[i]; // Dời step i -> i+1
    }

    // 3. Chèn dữ liệu Step đã sao chép vào vị trí đích
    destSub.step[insertionIndex] = sourceStepData;

    // 4. Tăng số lượng Step của SubProtocol đích
    destSub.u8_number_step_on_base++;

    qDebug() << "Successfully copied Step" << sourceStepIndex << "from Sub" << sourceSubIndex
             << "and inserted at index" << insertionIndex << "in Sub" << destSubIndex;

    // --- Cập nhật Giao diện Người dùng ---
    // Load lại giao diện cho SubProtocol ĐÍCH, và chọn Step vừa được chèn.
    load_protocol_to_ui(destSubIndex, insertionIndex);
    Display_Protocol_to_user(); // Cập nhật hiển thị chung (hoặc chỉ cập nhật cho destSubIndex?)
    calculator_volume_and_process_UI(); // Tính toán lại dựa trên thay đổi ở SubProtocol đích
}
void SYNO24::deleteStep(int subIndex, int stepIndex) {
    // --- 1. Kiểm tra tính hợp lệ của chỉ số SubProtocol ---
    if (subIndex < 0 || subIndex >= protocol_oligo.u8_number_sub) {
        qWarning() << "deleteStep - Invalid sub-protocol index:" << subIndex;
        QMessageBox::critical(this, "Lỗi nghiêm trọng", QString("Chỉ số SubProtocol không hợp lệ (%1).").arg(subIndex + 1));
        return;
    }

    // --- Lấy tham chiếu đến SubProtocol mục tiêu ---
    sub_protocol_t &targetSub = protocol_oligo.sub[subIndex];

    // --- 2. Kiểm tra tính hợp lệ của chỉ số Step cần xóa ---
    // Chỉ số step phải nằm trong phạm vi các step đang được sử dụng
    if (stepIndex < 0 || stepIndex >= targetSub.u8_number_step_on_base) {
        qWarning() << "deleteStep - Invalid step index:" << stepIndex << "for sub" << subIndex;
        if (targetSub.u8_number_step_on_base == 0) {
            QMessageBox::warning(this, "Lỗi", QString("Không thể xóa Step: SubProtocol (Sub %1) hiện đang trống!").arg(subIndex + 1));
        } else {
            QMessageBox::warning(this, "Lỗi", QString("Chỉ số Step không hợp lệ (%1) trong Sub %2! Chỉ số hợp lệ từ 1 đến %3.")
                                 .arg(stepIndex + 1).arg(subIndex + 1).arg(targetSub.u8_number_step_on_base));
        }
        return;
    }

    // --- 3. Thực hiện xóa và dịch chuyển ---
    qDebug() << "Attempting to delete Step" << stepIndex << "from Sub" << subIndex;

    // Dịch chuyển các Step phía sau lên một vị trí
    // Vòng lặp bắt đầu từ Step ngay sau Step bị xóa (stepIndex + 1)
    // và kết thúc ở Step cuối cùng hiện có (u8_number_step_on_base - 1)
    for (int i = stepIndex + 1; i < targetSub.u8_number_step_on_base; ++i) {
        targetSub.step[i - 1] = targetSub.step[i]; // Di chuyển step[i] về vị trí step[i-1]
    }

    // --- 4. Giảm số lượng Step của SubProtocol ---
    // Sau khi dịch chuyển, giảm bộ đếm số step đi 1
    targetSub.u8_number_step_on_base--;

    // (Tùy chọn) Xóa dữ liệu của step cuối cùng cũ (giờ không còn dùng đến)
    // Mặc dù không bắt buộc vì u8_number_step_on_base đã giảm, nhưng có thể giúp tránh nhầm lẫn khi debug
    if (targetSub.u8_number_step_on_base >= 0 && targetSub.u8_number_step_on_base < MAX_STEP_OF_SUB) {
        // Tạo một step rỗng hoặc mặc định để gán
        // Step_process_parameter_t emptyStep = {}; // Hoặc khởi tạo theo cách phù hợp
        // targetSub.step[targetSub.u8_number_step_on_base] = emptyStep;
        // Hoặc đơn giản là không cần làm gì nếu cấu trúc Step không chứa con trỏ hay tài nguyên cần giải phóng đặc biệt
    }
    qDebug() << "Successfully deleted Step" << stepIndex << "from Sub" << subIndex
             << ". New step count:" << targetSub.u8_number_step_on_base;

    // --- 5. Cập nhật Giao diện Người dùng ---
    // Load lại giao diện cho SubProtocol vừa bị thay đổi.
    // Chọn step ở vị trí vừa xóa (nếu còn step) hoặc step cuối cùng mới.
    int newSelectedIndex = -1;
    if (targetSub.u8_number_step_on_base > 0) {
        // Chọn step tại index cũ nếu nó vẫn hợp lệ, nếu không chọn step cuối cùng mới
        newSelectedIndex = qMin(stepIndex, targetSub.u8_number_step_on_base - 1);
    }
    load_protocol_to_ui(subIndex, newSelectedIndex); // Load lại UI cho sub này, chọn step mới (hoặc không chọn nếu sub rỗng)

    Display_Protocol_to_user();           // Cập nhật hiển thị tổng thể (nếu cần)
    calculator_volume_and_process_UI(); // Tính toán lại các thông số dựa trên thay đổi
}


void SYNO24::on_btn_delsub_released()
{
    ui->spbox_number_sub->setValue(ui->spbox_number_sub->value() - 1);
    protocol_oligo.u8_number_sub =  ui->spbox_number_sub->value();
}


void SYNO24::on_btn_addsub_released()
{
    ui->spbox_number_sub->setValue(ui->spbox_number_sub->value() + 1);
    protocol_oligo.u8_number_sub =  ui->spbox_number_sub->value();
}


void SYNO24::on_btn_InsertStep_released()
{
    bool ok; // Biến kiểm tra xem người dùng có nhấn OK trên QInputDialog hay không
    int sourceSubIndex = -1, sourceStepIndex = -1, destSubIndex = -1, insertionIndex = -1;

    // --- 0. Kiểm tra xem có SubProtocol nào tồn tại không ---
    if (protocol_oligo.u8_number_sub == 0) {
        QMessageBox::warning(this, "No Sub-Protocols", "There are no sub-protocols available.");
        return;
    }
    // Giới hạn số SubProtocol hiển thị trong lựa chọn (ví dụ: tối đa 5)
    int maxSubsToShow = qMin((int)protocol_oligo.u8_number_sub, 5);
    if (maxSubsToShow <=0) return; // Trường hợp dự phòng

    // --- 1. Chọn SubProtocol Nguồn ---
    QStringList subProtocolItems; // Danh sách tên SubProtocol để chọn
    for (int i = 0; i < maxSubsToShow; ++i) {
        subProtocolItems << QString("Sub %1").arg(i + 1);
    }

    QString selectedSourceSubStr = QInputDialog::getItem(this, "Select Source Sub-Protocol", // Tiêu đề cửa sổ
                                                         "Copy step FROM which sub-protocol:",     // Nhãn hướng dẫn
                                                         subProtocolItems,                         // Danh sách lựa chọn
                                                         0, false, &ok);
    if (!ok || selectedSourceSubStr.isEmpty()) return; // Người dùng hủy
    sourceSubIndex = subProtocolItems.indexOf(selectedSourceSubStr); // Lấy chỉ số (0-based)

    // --- 2. Chọn Step Nguồn ---
    sub_protocol_t &sourceSub = protocol_oligo.sub[sourceSubIndex];
    // Kiểm tra xem SubProtocol nguồn có Step nào không
    if (sourceSub.u8_number_step_on_base == 0) {
        QMessageBox::information(this, "Information", QString("Source SubProtocol (Sub %1) has no steps to copy from.")
                                 .arg(sourceSubIndex + 1));
        return;
    }

    // Tạo danh sách các Step trong SubProtocol nguồn
    QStringList sourceStepItems;
    for (int i = 0; i < sourceSub.u8_number_step_on_base; ++i) {
        sourceStepItems << QString("Step %1").arg(i + 1);
    }

    QString selectedSourceStepStr = QInputDialog::getItem(this, "Select Source Step",
                                                          QString("Select Step to copy FROM Sub %1:").arg(sourceSubIndex + 1),
                                                          sourceStepItems, 0, false, &ok);
    if (!ok || selectedSourceStepStr.isEmpty()) return; // Người dùng hủy
    sourceStepIndex = sourceStepItems.indexOf(selectedSourceStepStr); // Lấy chỉ số Step nguồn

    // --- 3. Chọn SubProtocol Đích ---
    // Sử dụng lại danh sách subProtocolItems đã tạo ở bước 1
    QString selectedDestSubStr = QInputDialog::getItem(this, "Select Destination Sub-Protocol",
                                                       "Insert step INTO which sub-protocol:",
                                                       subProtocolItems, 0, false, &ok);
    if (!ok || selectedDestSubStr.isEmpty()) return; // Người dùng hủy
    destSubIndex = subProtocolItems.indexOf(selectedDestSubStr); // Lấy chỉ số SubProtocol đích

    // --- 4. Chọn Vị Trí Chèn trong SubProtocol Đích ---
    sub_protocol_t &destSub = protocol_oligo.sub[destSubIndex];

    // Kiểm tra xem SubProtocol đích có đầy không TRƯỚC KHI hỏi vị trí chèn
    if (destSub.u8_number_step_on_base >= MAX_STEP_OF_SUB) {
        QMessageBox::warning(this, "Error", QString("Cannot insert Step: Destination SubProtocol (Sub %1) is full!")
                             .arg(destSubIndex + 1));
        return;
    }

    // Tạo danh sách các vị trí chèn có thể có
    QStringList insertionPointItems;
    for (int i = 0; i <= destSub.u8_number_step_on_base; ++i) {
        if (i < destSub.u8_number_step_on_base) {
            insertionPointItems << QString("Insert before Step %1").arg(i + 1);
        } else {
            // Vị trí cuối cùng
            insertionPointItems << QString("Insert at the end (after Step %1)").arg(destSub.u8_number_step_on_base);
        }
    }
    // Xử lý trường hợp SubProtocol đích rỗng
    if (destSub.u8_number_step_on_base == 0) {
        insertionPointItems.clear();
        insertionPointItems << QString("Insert at the beginning");
    }

    QString insertionPointStr = QInputDialog::getItem(this, "Select Insertion Point",
                                                      QString("Select where to insert INTO Sub %1:").arg(destSubIndex + 1),
                                                      insertionPointItems, 0, false, &ok);
    if (!ok || insertionPointStr.isEmpty()) return; // Người dùng hủy
    insertionIndex = insertionPointItems.indexOf(insertionPointStr); // Lấy chỉ số vị trí chèn

    // --- 5. Gọi hàm logic cốt lõi để thực hiện công việc ---
    copyAndInsertStep(sourceSubIndex, sourceStepIndex, destSubIndex, insertionIndex);

    // --- 6. Hiển thị thông báo thành công ---
    QMessageBox::information(this, "Success", QString("Successfully copied Step %1 from Sub %2 and inserted at position %3 in Sub %4.")
                             .arg(sourceStepIndex + 1) // +1 để hiển thị cho người dùng (1-based)
                             .arg(sourceSubIndex + 1)
                             .arg(insertionIndex + 1)
                             .arg(destSubIndex + 1));
}

// Hàm để thay đổi màu của một ô trong QTableWidget
void SYNO24::setCellColor(QTableWidget* tableWidget, int row, int col, const QColor& color) {
    // Kiểm tra để đảm bảo rằng row và col hợp lệ và ô không null
    if (row >= 0 && row < tableWidget->rowCount() && col >= 0 && col < tableWidget->columnCount()) {
        QTableWidgetItem* item = tableWidget->item(row, col);
        if (item == nullptr) {
            // Nếu không có ô, tạo mới một ô
            item = new QTableWidgetItem();
            tableWidget->setItem(row, col, item);
        }
        item->setBackground(QBrush(color));
    }
}

void  SYNO24::MonitorPlateUpdateUI(uint16_t baseFinished) {
    // Ví dụ: Thiết lập màu sắc cho một số ô trong bảng
    for (int cols = 0; cols < 3; cols++)
    { // 12 CỘT
        for (int rows = 0; rows < 8; rows++)
        { // 8 HÀNG
            // nếu seqence vị trí này rỗng thì màu xám
            if (global_var.amidite_well[cols * 8 + rows].string_sequence.length() == 0) {
                setCellColor(ui->tableWidget, rows, cols, Qt::gray);
            } else {
                if(global_var.signal_kill.well_index[cols * 8 + rows] == true) // neu bi kill
                {
                    setCellColor(ui->tableWidget, rows, cols, Qt::red);    //
                }
                else
                {
                    // nếu không phải rỗng
                    if((global_var.amidite_well[cols * 8 + rows].string_sequence.length() > 0 && syno24_machine.getAutoState() == State::STOPED) ||
                            (syno24_machine.getAutoState() == State::STOPED && global_var.amidite_well[cols * 8 + rows].string_sequence.length() >= baseFinished ))
                    {
                        setCellColor(ui->tableWidget, rows, cols, Qt::blue);    //
                    }
                    else
                    {
                        if(syno24_machine.getAutoState() == State::RUNNING && global_var.amidite_well[cols * 8 + rows].string_sequence.length() >= baseFinished)
                        {
                            setCellColor(ui->tableWidget, rows, cols,Qt::green );    //
                        }
                    }
                }
            }
        }
    }
}



void SYNO24::on_btn_delStep_released()
{
    bool ok; // Biến kiểm tra xem người dùng có nhấn OK trên QInputDialog hay không
    int subIndexToDelete = -1;
    int stepIndexToDelete = -1;

    // --- 0. Kiểm tra xem có SubProtocol nào tồn tại không ---
    if (protocol_oligo.u8_number_sub == 0) {
        QMessageBox::warning(this, "Warning", "No SubProtocols are currently available.");
        return;
    }

    // --- 1. Chọn SubProtocol để xóa Step ---
    QStringList subProtocolItems; // Danh sách tên SubProtocol để chọn
    // Lấy tất cả các SubProtocol hiện có
    for (int i = 0; i < protocol_oligo.u8_number_sub; ++i) {
        subProtocolItems << QString("Sub %1").arg(i + 1); // Hiển thị 1-based
    }

    QString selectedSubStr = QInputDialog::getItem(this, "Select SubProtocol", // Window title
                                                   "Select SubProtocol to delete Step from:", // Label prompt
                                                   subProtocolItems,            // List of options
                                                   0,                           // Default selection index
                                                   false,                       // Editable? (false)
                                                   &ok);                        // Pointer to bool for OK/Cancel

    if (!ok || selectedSubStr.isEmpty()) {
        qDebug() << "Người dùng đã hủy chọn SubProtocol.";
        return; // Người dùng nhấn Cancel hoặc không chọn gì
    }
    subIndexToDelete = subProtocolItems.indexOf(selectedSubStr); // Lấy chỉ số 0-based

    // --- 2. Chọn Step cần xóa từ SubProtocol đã chọn ---
    // Kiểm tra xem chỉ số Sub vừa lấy có hợp lệ không (dự phòng)
    if (subIndexToDelete < 0 || subIndexToDelete >= protocol_oligo.u8_number_sub) {
        qWarning() << "Logic Error: Invalid Sub index after selection:" << subIndexToDelete;
        return;
    }

    sub_protocol_t &selectedSub = protocol_oligo.sub[subIndexToDelete];

    // Kiểm tra xem SubProtocol đã chọn có Step nào không
    if (selectedSub.u8_number_step_on_base == 0) {
        QMessageBox::information(this, "Information", QString("Selected SubProtocol (Sub %1) has no Steps to delete.")
                                 .arg(subIndexToDelete + 1));
        return;
    }

    // Tạo danh sách các Step trong SubProtocol đã chọn
    QStringList stepItems;
    for (int i = 0; i < selectedSub.u8_number_step_on_base; ++i) {
        stepItems << QString("Step %1").arg(i + 1); // Hiển thị 1-based
    }

    QString selectedStepStr = QInputDialog::getItem(this, "Select Step to Delete", // Window title
                                                    QString("Select Step to delete from Sub %1:").arg(subIndexToDelete + 1), // Label prompt
                                                    stepItems, 0, false, &ok); // List of options, etc.

    if (!ok || selectedStepStr.isEmpty()) {
        qDebug() << "Người dùng đã hủy chọn Step.";
        return; // Người dùng nhấn Cancel
    }
    stepIndexToDelete = stepItems.indexOf(selectedStepStr); // Lấy chỉ số Step 0-based

    // Kiểm tra xem chỉ số Step vừa lấy có hợp lệ không (dự phòng)
    if (stepIndexToDelete < 0 || stepIndexToDelete >= selectedSub.u8_number_step_on_base) {
        qWarning() << "Logic Error: Invalid Step index after selection:" << stepIndexToDelete;
        return;
    }

    // --- 3. Gọi hàm logic cốt lõi để thực hiện xóa ---
    qDebug() << "Yêu cầu xóa - Sub Index:" << subIndexToDelete << ", Step Index:" << stepIndexToDelete;
    deleteStep(subIndexToDelete, stepIndexToDelete);
    // Hàm deleteStep đã tự cập nhật UI và tính toán lại
    // Bạn có thể thêm một thông báo thành công ở đây nếu muốn
    // --- (Tùy chọn) 4. Hiển thị thông báo thành công ---
    QMessageBox::information(this, "Success", QString("Successfully deleted Step %1 from Sub %2.")
                             .arg(stepIndexToDelete + 1) // +1 for 1-based display
                             .arg(subIndexToDelete + 1));
}


/*

void SYNO24::on_btnAddCoupling1_released()
{

    int selectedIndex = ui->cbx_coupling2Option->currentIndex();
    if (selectedIndex < 0 || selectedIndex >= amidite_.bottleNamesListFull.size())
        return;

    // Kiểm tra trùng lặp (tuỳ chọn)
    if (coupling1List.contains(static_cast<quint8>(selectedIndex))) {
        QMessageBox::warning(this, "Trùng lặp", "Index đã có trong Coupling 1.");
        return;
    }

    QString displayName = amidite_.bottleNamesListFull[selectedIndex];

    // Thêm vào bảng
    int rowCount = modelCoupling->rowCount();
    bool inserted = false;

    for (int i = 0; i < rowCount; ++i) {
        QModelIndex idx = modelCoupling->index(i, 0);
        if (!modelCoupling->data(idx).isValid()) {
            modelCoupling->setData(idx, displayName);
            inserted = true;
            break;
        }
    }

    if (!inserted) {
        QStandardItem* item1 = new QStandardItem(displayName);
        QStandardItem* item2 = new QStandardItem("");
        modelCoupling->appendRow(QList<QStandardItem*>() << item1 << item2);
    }

    // Thêm trực tiếp vào coupling1List
    coupling1List.append(static_cast<quint8>(selectedIndex));

    qDebug() << "Coupling 1 List:" << coupling1List;

}
*/

void SYNO24::on_btnAddCoupling1_released()
{
    int selectedIndex = ui->cbx_coupling2Option->currentIndex();
    if (selectedIndex < 0 || selectedIndex >= amidite_.bottleNamesListFull.size())
        return;

    QString displayName = amidite_.bottleNamesListFull[selectedIndex];

    // Kiểm tra nếu index đã có trong coupling2List → xóa nó
    if (coupling2List.contains(static_cast<quint8>(selectedIndex))) {
        coupling2List.removeOne(static_cast<quint8>(selectedIndex));

        // Xóa ô tương ứng trên bảng ở cột Coupling2
        for (int i = 0; i < modelCoupling->rowCount(); ++i) {
            QModelIndex idx = modelCoupling->index(i, 1);
            QString text = modelCoupling->data(idx).toString();
            if (text == displayName) {
                modelCoupling->setData(modelCoupling->index(i, 1), "");
                break;
            }
        }
    }

    // Nếu index đã có sẵn trong coupling1List → không thêm nữa
    if (coupling1List.contains(static_cast<quint8>(selectedIndex))) {
        QMessageBox::information(this, "Information", "Index already exists in Coupling 1.");
        return;
    }

    //    // Thêm vào bảng (nếu có ô trống)
    //    int rowCount = modelCoupling->rowCount();
    //    bool inserted = false;

    //    for (int i = 0; i < rowCount; ++i) {
    //        QModelIndex idx = modelCoupling->index(i, 0);
    //        if (!modelCoupling->data(idx).isValid()) {
    //            modelCoupling->setData(idx, displayName);
    //            inserted = true;
    //            break;
    //        }
    //    }

    //    if (!inserted) {
    //        QStandardItem* item1 = new QStandardItem(displayName);
    //        QStandardItem* item2 = new QStandardItem("");
    //        modelCoupling->appendRow(QList<QStandardItem*>() << item1 << item2);
    //    }

    // Thêm trực tiếp vào coupling1List
    coupling1List.append(static_cast<quint8>(selectedIndex));
    updateTableWithCouplingLists();
    qDebug() << "Coupling 1 List:" << coupling1List;
    qDebug() << "Coupling 2 List:" << coupling2List;
}

void SYNO24::on_btnAddCoupling2_released()
{

    int selectedIndex = ui->cbx_coupling2Option->currentIndex();
    if (selectedIndex < 0 || selectedIndex >= amidite_.bottleNamesListFull.size())
        return;

    QString displayName = amidite_.bottleNamesListFull[selectedIndex];

    // Kiểm tra nếu index tồn tại trong coupling1List → xóa nó
    if (coupling1List.contains(static_cast<quint8>(selectedIndex))) {
        coupling1List.removeOne(static_cast<quint8>(selectedIndex));

        // Xóa ô tương ứng trên bảng ở cột Coupling1
        for (int i = 0; i < modelCoupling->rowCount(); ++i) {
            QModelIndex idx = modelCoupling->index(i, 0);
            QString text = modelCoupling->data(idx).toString();
            if (text == displayName) {
                modelCoupling->setData(modelCoupling->index(i, 0), "");
                break;
            }
        }
    }

    // Nếu index đã có sẵn trong coupling2List → không thêm nữa
    if (coupling2List.contains(static_cast<quint8>(selectedIndex))) {
        QMessageBox::information(this, "Information", "Index already exists in Coupling 2.");
        return;
    }
    // Thêm trực tiếp vào coupling2List

    coupling2List.append(static_cast<quint8>(selectedIndex));
    updateTableWithCouplingLists();
    writeSettings();
    qDebug() << "Coupling 2 List:" << coupling2List;
}

void SYNO24::on_btn_AddAmiditeSubSpecial_released()
{
    int selectedIndex = ui->cbx_coupling2Option->currentIndex();
    if (selectedIndex < 0 || selectedIndex >= amidite_.bottleNamesListFull.size())
        return;

    QString displayName = amidite_.bottleNamesListFull[selectedIndex];

    // Kiểm tra xem đã tồn tại trong couplingSpecialBaseList chưa
    if (couplingSpecialBaseList.contains(static_cast<quint8>(selectedIndex))) {
        QMessageBox::information(this, "Information", "Index already exists in Special List.");
        return;
    }

    // Thêm vào danh sách
    couplingSpecialBaseList.append(static_cast<quint8>(selectedIndex));
    updateTableWithSpecialList();
    writeSettings();
    qDebug() << "Special Base List:" << couplingSpecialBaseList;
}
void SYNO24::updateTableWithSpecialList()
{
    modelAmiditeSubSpecial->setRowCount(0); // Xóa toàn bộ bảng cũ
    QSet<quint8> usedIndices;

    // Thêm hàng từ couplingSpecialBaseList
    for (quint8 idx : couplingSpecialBaseList) {
        if (idx >= amidite_.bottleNamesListFull.size()) continue;

        QString name = amidite_.bottleNamesListFull[idx];
        QStandardItem* item = new QStandardItem(name);
        modelAmiditeSubSpecial->appendRow(QList<QStandardItem*>() << item);
        usedIndices.insert(idx);
    }
}
void SYNO24::on_btndelGrCoupling_released()
{
    QModelIndexList selectedIndexes = ui->tableView_GroupCoupling->selectionModel()->selectedRows();
    if (!selectedIndexes.isEmpty()) {
        // Xử lý bảng Coupling
        foreach (const QModelIndex &index, selectedIndexes) {
            int row = index.row();

            QString textCoupling = modelCoupling->index(row, 0).data(Qt::DisplayRole).toString();
            QString textCoupling2 = modelCoupling->index(row, 1).data(Qt::DisplayRole).toString();

            int indexCoupling = amidite_.bottleNamesListFull.indexOf(textCoupling);
            int indexCoupling2 = amidite_.bottleNamesListFull.indexOf(textCoupling2);

            if (indexCoupling != -1)
                coupling1List.removeOne(static_cast<quint8>(indexCoupling));
            if (indexCoupling2 != -1)
                coupling2List.removeOne(static_cast<quint8>(indexCoupling2));

            modelCoupling->removeRow(row);
        }

        QMessageBox::information(this, "Information", "Item(s) deleted from Coupling Table.");
    } else {
        // Kiểm tra bảng Special
        selectedIndexes = ui->tableView_GroupSpecial->selectionModel()->selectedRows();
        if (!selectedIndexes.isEmpty()) {
            foreach (const QModelIndex &index, selectedIndexes) {
                int row = index.row();

                QString textSpecial = modelAmiditeSubSpecial->index(row, 0).data(Qt::DisplayRole).toString();
                int indexSpecial = amidite_.bottleNamesListFull.indexOf(textSpecial);

                if (indexSpecial != -1)
                    couplingSpecialBaseList.removeOne(static_cast<quint8>(indexSpecial));

                modelAmiditeSubSpecial->removeRow(row);
            }

            QMessageBox::information(this, "Information", "Item(s) deleted from Special Table.");
        } else {
            QMessageBox::information(this, "Information", "Please select a table to delete an item.");
            return;
        }
    }

    writeSettings();
    updateTableWithCouplingLists();     // Cập nhật bảng Coupling
    updateTableWithSpecialList();       // Cập nhật bảng Special

    qDebug() << "Coupling 1 List sau khi xóa:" << coupling1List;
    qDebug() << "Coupling 2 List sau khi xóa:" << coupling2List;
    qDebug() << "Special Base List sau khi xóa:" << couplingSpecialBaseList;
}


void SYNO24::on_btnSaveGrCoupling_released()
{

}


void SYNO24::updateTableWithCouplingLists()
{
    modelCoupling->setRowCount(0); // Xóa toàn bộ bảng cũ
    QSet<quint8> usedIndices;

    // Thêm hàng từ coupling1List
    for (quint8 idx : coupling1List) {
        if (idx >= amidite_.bottleNamesListFull.size()) continue;

        QString name = amidite_.bottleNamesListFull[idx];
        QStandardItem* item1 = new QStandardItem(name);
        QStandardItem* item2 = new QStandardItem("");
        modelCoupling->appendRow(QList<QStandardItem*>() << item1 << item2);
        usedIndices.insert(idx);
    }

    // Thêm hàng từ coupling2List (nếu chưa có trong coupling1)
    for (quint8 idx : coupling2List) {
        if (usedIndices.contains(idx)) continue; // Tránh trùng
        if (idx >= amidite_.bottleNamesListFull.size()) continue;

        QString name = amidite_.bottleNamesListFull[idx];
        QStandardItem* item1 = new QStandardItem("");
        QStandardItem* item2 = new QStandardItem(name);
        modelCoupling->appendRow(QList<QStandardItem*>() << item1 << item2);
    }
}

bool SYNO24::isInCoupling1(quint8 index) const
{
    return coupling1List.contains(index);
}


bool SYNO24::isInCoupling2(quint8 index) const
{
    return coupling2List.contains(index);
}


bool SYNO24::isInCouplingLastBase(quint8 index) const
{
    return couplingSpecialBaseList.contains(index);
}

void SYNO24::onCountdownFinished()
{
    ui->prgBar_Push->setValue(0);
    ui->prgBar_Wait->setValue(0);
}


void SYNO24::UpdateUISTTRun( uint8_t state)
{
    //ui->btnFuncRun->setIcon(QIcon(":/images/images/number-1 (1).png"));
    QString iconName = QString(":/images/images/number-%1.png").arg(state);
    ui->btnFuncRun->setIcon(QIcon(iconName));
    ui->lbl_stt_sub_step->setText("RUN ON SUB : " + QString::number(global_var.updateSTTRun2UI.currentSub + 1)
                                  + "| STEP : " + QString::number(global_var.updateSTTRun2UI.currentStep + 1));
    if(state == 0)
    {
        pulseLabel(ui->lbl_stt_fw_fb, "Delivery Reagent & Wait", "#FF0000");
    }
}

void SYNO24::on_btn_saveReagentFillCoupling2_released()
{
    global_var.Coupling2Setting.EnableFillWellDone = ui->chkbx_ReagentFillCoupling2->isChecked();
    global_var.Coupling2Setting.typeReagent = ui->cbx_type_reagentDelivery->currentIndex();
    global_var.Coupling2Setting.volume.Data = ui->spbx_volume_FillDeliveryCoupling2->value();
}

void SYNO24::FindSignalLastedSequence()
{
    // LastbaseinWellCounter
    // tìm xem đây có phải base cuối cùng đầu 5'
    // ********************************************************************************************


}

void SYNO24::on_btn_tabKillSequenceRun_released()
{
    if(syno24_machine.getAutoState() != State::RUNNING)
    {
        KillSequence m_killsequence;
        m_killsequence.setModal(true);
        m_killsequence.setKillSequence(global_var.signal_kill.well_index);
        // m_killsequence.exec();
        // Hiển thị dialog và kiểm tra kết quả
        if (m_killsequence.exec() == QDialog::Accepted) {
            qDebug() << "KillSequence OK.";
            m_killsequence.getKillSequence(global_var.signal_kill.well_index);
        } else {
            // Người dùng đã nhấn "Cancel" hoặc đóng dialog
            qDebug() << "KillSequence was closed without Tắt không đúng cách.";
        }
        MonitorPlateUpdateUI(0);
    }
    else
    {
        QMessageBox::information(this, "Information", " Please STOP or PAUSE system to Kill sequence");
    }
}
//****************************************************************************************************
void SYNO24 :: START_OLIGO_TASK()
{

}
// --- Các hàm hỗ trợ ---
/*
 * 08-08-2025
 * update 20-08-2025
 *
 * */
void SYNO24::on_chkbox_Allbase_toggled(bool checked)
{
    QString Basestr = "";
    if(checked)
    {
        Basestr = fnc.generateNumberString(global_var.signal_status_oligo.u16_max_sequence_amidite_setting);
        ui->lineEdit_special_base_VacuumBox->setText(Basestr);
    }
    else
    {
        ui->lineEdit_special_base_VacuumBox->setText(Basestr);
    }
}

void SYNO24:: GetFeatureVacuumBox()
{
    global_var.advanced_setting.VacuumBox.Enablefeature = ui->chkbxEnaVaccuumBox->isChecked()? 1 : 0;
    global_var.advanced_setting.VacuumBox.En_WASH =  ui->chkbxVacuumBoxWashing->isChecked()? 1 : 0;
    global_var.advanced_setting.VacuumBox.En_Deblock =  ui->chkbxVacuumBoxDeblock->isChecked()? 1 : 0;
    global_var.advanced_setting.VacuumBox.En_Cap =  ui->chkbxVacuumBoxCapping->isChecked()? 1 : 0;
    global_var.advanced_setting.VacuumBox.En_Coupling =  ui->chkbxVacuumBoxCoupling->isChecked()? 1 : 0;
    global_var.advanced_setting.VacuumBox.En_Ox =  ui->chkbxVacuumBoxOx->isChecked()? 1 : 0;
    global_var.advanced_setting.VacuumBox.time.Data = ui->spbx_time_FanVacuumBox->value();
    qDebug()<< "GetFeatureVacuumBox Enablefeature : " << global_var.advanced_setting.VacuumBox.Enablefeature;
    readSpecialBaseFromLineEdit(ui->lineEdit_special_base_VacuumBox, m_baseVacuumBox.speacial_base);
}

void SYNO24::on_btn_FanVacuumBox_released()
{

}

// ===== UTILITY FUNCTIONS =====

/**
 * @brief Convert SyntheticResult to human-readable string
 */
const char* syntheticResultToString(SyntheticResult result) {
    switch (result) {
    case SyntheticResult::SUCCESS: return "SUCCESS";
    case SyntheticResult::INVALID_STATE: return "INVALID_STATE";
    case SyntheticResult::CONNECTION_ERROR: return "CONNECTION_ERROR";
    case SyntheticResult::FIRMWARE_ERROR: return "FIRMWARE_ERROR";
    case SyntheticResult::HUMIDITY_TIMEOUT: return "HUMIDITY_TIMEOUT";
    case SyntheticResult::USER_STOPPED: return "USER_STOPPED";
    case SyntheticResult::SEQUENCE_COMPLETE: return "SEQUENCE_COMPLETE";
    case SyntheticResult::PROTOCOL_ERROR: return "PROTOCOL_ERROR";
    default: return "UNKNOWN_ERROR";
    }
}



/**
 * @brief Check if system is ready for synthetic operation
 */
bool SYNO24::checkSyntheticSystemState() {
    if (syno24_machine.getAutoState() != State::STOPED) {
        qDebug() << "System not in STOPPED state, current state:" << (int)syno24_machine.getAutoState();
        return false;
    }

    if (!STM32_COM.flag_connecttion) {
        qDebug() << "STM32 not connected";
        return false;
    }

    qDebug() << "System state check passed";
    return true;
}



/**
 * @brief Initialize synthetic operation with all required setup
 */
SyntheticResult SYNO24::initializeSyntheticOperation(LogHistory& logHistory) {
    qDebug() << "=== INITIALIZING SYNTHETIC OPERATION ===";

    // Check system state
    if (!checkSyntheticSystemState()) {
        qDebug() << "System state check failed";
        return SyntheticResult::INVALID_STATE;
    }

    // Stop humidity timer
    timer_update_humidity_tempareture.stop();
    qDebug() << "Humidity timer stopped";

    // Clear UI and set initial state
    ui->textEdit_oligo_history_log->clear();
    syno24_machine.setAutoState(State::RUNNING);
    qDebug() << "System state set to RUNNING";

    // Set humidity configuration
    global_var.status_and_sensor.u16tb_humidity_Preset.Data = ui->spinbx_humidity_value->value();
    global_var.status_and_sensor.flag_enable_auto_control_air_Nito = ui->checkbox_wait_humidity->isChecked();
    qDebug() << "Humidity preset:" << global_var.status_and_sensor.u16tb_humidity_Preset.Data;
    qDebug() << "Auto humidity control:" << global_var.status_and_sensor.flag_enable_auto_control_air_Nito;

    // Calculate and start countdown timer
    quint32 timeEstimate = 0;
    CalEstimateTimeProtocol(&timeEstimate);
    startCountdown(timeEstimate);
    qDebug() << "Countdown started with estimate:" << timeEstimate << "seconds";

    // Log system start
    log_terminal_withTimestamp(" : SYSTEM START", Info);

    // Send air control start command
    QByteArray Command_send(LENGTH_COMMAND_SEND, 0);
    Command_send[0] = CMD_CONTROL_AIR_START;
    Command_send[1] = 1;

    if (!STM32_COM.serial_send_command_Firmware(serialPort, Command_send, SyntheticConfig::COMMAND_TIMEOUT_MS)) {
        qDebug() << "Failed to send air control command";
        return SyntheticResult::FIRMWARE_ERROR;
    }
    qDebug() << "Air control command sent successfully";

    // Disable start button and update UI
    ui->btn_start_synthetic->setDisabled(true);
    ui->lbl_status_system->setText("System Running Synthetic");
    qDebug() << "UI updated - start button disabled";

    // Send system settings
    send_setting();
    qDebug() << "System settings sent";

    // Set advanced settings
    global_var.advanced_setting.flag_auto_primming_chemical = ui->checkbox_autoPrim_amidite->isChecked();
    global_var.advanced_setting.u16tb_autoPrim_volume_amidite.Data = ui->spinbx_volume_prim_amidite->value();
    global_var.advanced_setting.u16tb_autoPrim_volume_Activator.Data = ui->spinbx_volume_prim_activator->value();
    qDebug() << "Advanced settings configured";
    qDebug() << "Auto priming:" << global_var.advanced_setting.flag_auto_primming_chemical;
    qDebug() << "Amidite volume:" << global_var.advanced_setting.u16tb_autoPrim_volume_amidite.Data;
    qDebug() << "Activator volume:" << global_var.advanced_setting.u16tb_autoPrim_volume_Activator.Data;

    // Log protocol information to history
    logHistory.appendToLog(ui->textEdit_list_task_protocol->toPlainText());
    qDebug() << "Protocol information logged to history";

    qDebug() << "Synthetic operation initialized successfully";
    return SyntheticResult::SUCCESS;
}


/**
 * @brief Wait for humidity to reach target level
 */
SyntheticResult SYNO24::waitForSyntheticHumidity() {
    qDebug() << "=== CHECKING HUMIDITY REQUIREMENTS ===";

    get_sensor_humidity_tempareture();
    float humidity = global_var.status_and_sensor.f_humidity / 100.0f;

    // Check if humidity control is enabled
    if (!global_var.status_and_sensor.flag_enable_auto_control_air_Nito ||
            !global_var.status_and_sensor.flag_have_feedback_value) {
        qDebug() << "Humidity control disabled or no sensor feedback";
        qDebug() << "Auto control enabled:" << global_var.status_and_sensor.flag_enable_auto_control_air_Nito;
        qDebug() << "Sensor feedback available:" << global_var.status_and_sensor.flag_have_feedback_value;
        return SyntheticResult::SUCCESS;
    }

    log_terminal("Waiting for humidity to decrease", Info);
    wait_humidity();

    float target_humidity = global_var.status_and_sensor.u16tb_humidity_Preset.Data;
    qDebug() << "Target humidity:" << target_humidity << "Current:" << humidity;

    // Wait for humidity to reach target
    int wait_cycles = 0;
    const int MAX_WAIT_CYCLES = 1200; // 10 minutes at 500ms intervals

    while (humidity >= target_humidity) {
        if (syno24_machine.getAutoState() == State::STOPED) {
            qDebug() << "User stopped during humidity wait";
            return SyntheticResult::USER_STOPPED;
        }

        delay_ui.delay_ms(SyntheticConfig::HUMIDITY_CHECK_INTERVAL_MS);
        get_sensor_humidity_tempareture();
        humidity = global_var.status_and_sensor.f_humidity / 100.0f;

        wait_cycles++;
        if (wait_cycles % 20 == 0) { // Log every 10 seconds
            qDebug() << "Humidity:" << humidity << "Target:" << target_humidity << "Wait cycles:" << wait_cycles;
        }

        // Timeout protection
        if (wait_cycles >= MAX_WAIT_CYCLES) {
            qDebug() << "Humidity wait timeout after" << MAX_WAIT_CYCLES * SyntheticConfig::HUMIDITY_CHECK_INTERVAL_MS / 1000 << "seconds";
            return SyntheticResult::HUMIDITY_TIMEOUT;
        }
    }

    log_terminal("Finish Wait for humidity", Info);
    qDebug() << "Humidity target reached after" << wait_cycles << "cycles";
    return SyntheticResult::SUCCESS;
}


/**
 * @brief Calculate total bases and validate protocol configuration
 */
SyntheticResult SYNO24::calculateSyntheticTotalBases(uint8_t& u8_number_sub_run, uint8_t& u8_lastest_sub, uint16_t& u16_max_base_setting_protocol) {
    qDebug() << "=== CALCULATING PROTOCOL PARAMETERS ===";

    u8_number_sub_run = ui->spbox_number_sub->value();
    u8_lastest_sub = 0;
    u16_max_base_setting_protocol = 0;

    qDebug() << "Number of subs to run:" << u8_number_sub_run;
    qDebug() << "Protocol has" << protocol_oligo.u8_number_sub << "subs defined";
    qDebug() << "Target sequence length:" << global_var.signal_status_oligo.u16_max_sequence_amidite_setting;

    // Validate protocol configuration
    if (protocol_oligo.u8_number_sub == 0) {
        qDebug() << "No sub-protocols defined";
        return SyntheticResult::PROTOCOL_ERROR;
    }

    if (u8_number_sub_run == 0) {
        qDebug() << "No sub-protocols selected to run";
        return SyntheticResult::PROTOCOL_ERROR;
    }

    if (u8_number_sub_run > protocol_oligo.u8_number_sub) {
        qDebug() << "More subs requested than available";
        return SyntheticResult::PROTOCOL_ERROR;
    }

    // Calculate total bases in protocol and find last sub
    for (uint8_t ctn_sub = 0; ctn_sub < protocol_oligo.u8_number_sub; ctn_sub++) {
        if (syno24_machine.getAutoState() == State::STOPED) {
            qDebug() << "User stopped during protocol calculation";
            return SyntheticResult::USER_STOPPED;
        }

        uint8_t bases_in_sub = protocol_oligo.sub[ctn_sub].u8_number_base_on_sub;
        u16_max_base_setting_protocol += bases_in_sub;
        qDebug() << "Sub" << ctn_sub << "has" << bases_in_sub << "bases, total so far:" << u16_max_base_setting_protocol;

        // Determine last sub for sequence extension
        if (u16_max_base_setting_protocol < global_var.signal_status_oligo.u16_max_sequence_amidite_setting) {
            if (protocol_oligo.u8_number_sub == 1) {
                u8_lastest_sub = 0;
            } else {
                u8_lastest_sub = (ctn_sub < (protocol_oligo.u8_number_sub - 1)) ? ctn_sub + 1 : ctn_sub;
            }
            qDebug() << "Latest sub for extension:" << u8_lastest_sub;
        }
    }

    // Handle sequence longer than protocol
    int int_remain_base = global_var.signal_status_oligo.u16_max_sequence_amidite_setting - u16_max_base_setting_protocol;

    // Log protocol information
    log_terminal("Start Synthesis Oligo", Info);
    log_terminal("Protocol Included : " + QString::number(u8_number_sub_run) + " sub-protocol", Info);
    log_terminal("Protocol setting total: " + QString::number(u16_max_base_setting_protocol) + " base", Info);
    log_terminal("Table Sequence setting max: " + QString::number(global_var.signal_status_oligo.u16_max_sequence_amidite_setting) + " base", Info);

    if (int_remain_base > 0) {
        protocol_oligo.sub[u8_lastest_sub].u8_number_base_on_sub += int_remain_base;
        log_terminal("System auto continuous run " + QString::number(int_remain_base) + " last sequence with sub" + QString::number(u8_lastest_sub + 1), Info);
        qDebug() << "Extended sub" << u8_lastest_sub << "with" << int_remain_base << "additional bases";
        qDebug() << "Final base count for last sub:" << protocol_oligo.sub[u8_lastest_sub].u8_number_base_on_sub;
    } else if (int_remain_base < 0) {
        qDebug() << "Warning: Protocol has more bases than sequence length";
        log_terminal("Warning: Protocol longer than sequence - will stop at sequence end", Warning);
    }

    // Initialize counters
    global_var.signal_status_oligo.u16_counter_base_finished = 0;
    ui->lbl_base_finished->setText(QString::number(0));

    // Clear status display
    ui->textEdit_status_update_fw->clear();

    qDebug() << "Protocol calculation completed successfully";
    qDebug() << "Total protocol bases:" << u16_max_base_setting_protocol;
    qDebug() << "Target sequence bases:" << global_var.signal_status_oligo.u16_max_sequence_amidite_setting;
    qDebug() << "Remaining bases to extend:" << int_remain_base;

    return SyntheticResult::SUCCESS;
}


/**
 * @brief Process all sub-protocols in sequence
 */
SyntheticResult SYNO24::processAllSyntheticSubProtocols(uint8_t u8_number_sub_run, LogHistory& logHistory) {
    qDebug() << "=== PROCESSING ALL SUB-PROTOCOLS ===";
    qDebug() << "Total subs to process:" << u8_number_sub_run;

    // Set timestamp
    log_terminal_withTimestamp("", Info);

    for (uint8_t u8_counter_sub_run = 0; u8_counter_sub_run < u8_number_sub_run; u8_counter_sub_run++) {
        if (syno24_machine.getAutoState() == State::STOPED) {
            qDebug() << "User stopped during sub processing at sub" << u8_counter_sub_run;
            return SyntheticResult::USER_STOPPED;
        }

        qDebug() << "Processing sub" << u8_counter_sub_run + 1 << "of" << u8_number_sub_run;
        // bắt đầu tổng hợp
        SyntheticResult result = processSyntheticSubProtocol(u8_counter_sub_run, logHistory);
        if (result != SyntheticResult::SUCCESS) {
            qDebug() << "Sub processing failed with result:" << syntheticResultToString(result);
            return result;
        }

        // Delay between subs
        delay_ui.delay_ms(SyntheticConfig::DELAY_BETWEEN_SUBS_MS);
        qDebug() << "Sub" << u8_counter_sub_run + 1 << "completed successfully";
    }
    qDebug() << "All sub-protocols processed successfully";

    // Process final special sub-protocol after all normal subs are completed
    //    qDebug() << "Starting final special sub-protocol";
    //    SyntheticResult finalResult = processFinalSpecialSubProtocol(logHistory);
    //    if (finalResult != SyntheticResult::SUCCESS) {
    //        qDebug() << "Final special sub-protocol failed:" << syntheticResultToString(finalResult);
    //        return finalResult;
    //    }
    return SyntheticResult::SUCCESS;
}

/**
 * @brief Process a single sub-protocol
 */
SyntheticResult SYNO24::processSyntheticSubProtocol(uint8_t u8_counter_sub_run, LogHistory& logHistory) {
    qDebug() << "=== PROCESSING SUB-PROTOCOL" << u8_counter_sub_run + 1 << "===";

    // Update UI state
    global_var.updateSTTRun2UI.currentSub = u8_counter_sub_run;
    uint8_t u8_number_base_run = protocol_oligo.sub[u8_counter_sub_run].u8_number_base_on_sub;

    // Log sub start
    log_terminal("Running sub : " + QString::number(u8_counter_sub_run + 1), Info);
    logHistory.appendToLog("Running sub : " + QString::number(u8_counter_sub_run + 1));

    qDebug() << "Sub has" << u8_number_base_run << "bases to process";
    qDebug() << "Current bases completed:" << global_var.signal_status_oligo.u16_counter_base_finished;
    qDebug() << "Target total bases:" << global_var.signal_status_oligo.u16_max_sequence_amidite_setting;
    //int totalInitialWells;
    double remainingPercentage = 0;
    // Process each base in the sub-protocol
    for (uint8_t u8_counter_base_on_sub = 0; u8_counter_base_on_sub < u8_number_base_run; u8_counter_base_on_sub++) {

        // 03-11-2025 thêm function tính toán
        uint8_t current_sequence[MAX_WELL_AMIDITE];
        for (int i = 0; i < MAX_WELL_AMIDITE; ++i) {
            current_sequence[i] = global_var.amidite_well[i].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished]; // Vẫn có hóa chất
        }
        QPair<int, double> results = ChemicalWells::checkRemainingChemicalWells(current_sequence, m_tableDataMultiplier.totalInitialWells);
        remainingPercentage = results.second;
        ui->progressBar_percentRun->setValue(100.0 - remainingPercentage);
        qDebug() << "Số giếng còn lại chứa hóa chất:" << results.first;
        qDebug() << "Tỉ lệ phần trăm giếng còn lại:" << QString::number(results.second, 'f', 2) + "%";

        double DonePercentage = 100.0 - remainingPercentage;
        m_tableDataMultiplier.multiplier = findMultiplierByPercentage(DonePercentage);
        if (m_tableDataMultiplier.multiplier != 100) {
            qDebug() << "Tỉ lệ phần trăm giếng còn lại là:" << remainingPercentage << "%";
            qDebug() << "Hệ số nhân tương ứng là:" << m_tableDataMultiplier.multiplier;
            // Thực hiện các hành động khác với hệ số nhân này
        } else {
            qDebug() << "Không tìm thấy hệ số nhân phù hợp cho tỉ lệ:" << remainingPercentage << "%";
        }
        ui->lbl_multipler->setText("Multiplier : " + QString::number(m_tableDataMultiplier.multiplier) + " %" );

        if (syno24_machine.getAutoState() == State::STOPED) {
            qDebug() << "User stopped during base processing at base" << u8_counter_base_on_sub;
            return SyntheticResult::USER_STOPPED;
        }

        // Check if sequence is complete
        if (global_var.signal_status_oligo.u16_counter_base_finished >= global_var.signal_status_oligo.u16_max_sequence_amidite_setting) {
            log_terminal("THE SYSTEM HAS AUTOMATICALLY STOPPED, AND THE OLIGO SEQUENCE IS COMPLETE", Success);
            qDebug() << "Sequence completed - stopping at base" << global_var.signal_status_oligo.u16_counter_base_finished;
            return SyntheticResult::SEQUENCE_COMPLETE;
        }

        qDebug() << "Processing base" << global_var.signal_status_oligo.u16_counter_base_finished + 1 << "in sub" << u8_counter_sub_run + 1;

        SyntheticResult result = processSyntheticBase(u8_counter_sub_run, u8_counter_base_on_sub, logHistory);
        if (result != SyntheticResult::SUCCESS) {
            qDebug() << "Base processing failed with result:" << syntheticResultToString(result);
            return result;
        }

        // Update progress after successful base completion
        global_var.signal_status_oligo.u16_counter_base_finished++;
        MonitorPlateUpdateUI(global_var.signal_status_oligo.u16_counter_base_finished);
        ui->lbl_base_finished->setText(QString::number(global_var.signal_status_oligo.u16_counter_base_finished));

        qDebug() << "Base completed. Total bases finished:" << global_var.signal_status_oligo.u16_counter_base_finished;

        // Handle sensor updates periodically
        if ((global_var.signal_status_oligo.u16_counter_base_finished % 1) == 0) {
            get_sensor_humidity_tempareture();
            log_terminal("Humidity: " + QString::number(global_var.status_and_sensor.f_humidity / 100), Trace);
            log_terminal_withTimestamp("", Info);
        }

        // Handle pause state
        handleSyntheticPauseState();

        // Delay between bases
        delay_ui.delay_ms(SyntheticConfig::DELAY_BETWEEN_BASES_MS);
    }

    qDebug() << "Sub-protocol" << u8_counter_sub_run + 1 << "completed successfully";
    return SyntheticResult::SUCCESS;
}



/**
 * @brief Process a single base within a sub-protocol
 */
SyntheticResult SYNO24::processSyntheticBase(uint8_t u8_counter_sub_run, uint8_t u8_counter_base_on_sub, LogHistory& logHistory) {
    qDebug() << "=== PROCESSING BASE" << global_var.signal_status_oligo.u16_counter_base_finished + 1 << "===";

    // Update current base counter
    global_var.signal_status_oligo.u16_counter_current_base = global_var.signal_status_oligo.u16_counter_base_finished + 1;

    // Log base start
    log_terminal("    - Base : " + QString::number(global_var.signal_status_oligo.u16_counter_base_finished + 1), Info);
    logHistory.appendToLog("    - Base : " + QString::number(global_var.signal_status_oligo.u16_counter_base_finished + 1));

    // Get number of steps for this base
    uint8_t u8_number_step_run = protocol_oligo.sub[u8_counter_sub_run].u8_number_step_on_base;
    qDebug() << "Base has" << u8_number_step_run << "steps to process";

    // Process each step in the base
    for (uint8_t u8_counter_step = 0; u8_counter_step < u8_number_step_run; u8_counter_step++) {
        GetFeatureVacuumBox();
        if (syno24_machine.getAutoState() == State::STOPED) {
            qDebug() << "User stopped during step processing at step" << u8_counter_step;
            return SyntheticResult::USER_STOPPED;
        }

        qDebug() << "Processing step" << u8_counter_step + 1 << "of" << u8_number_step_run;

        SyntheticResult result = processSyntheticStep(u8_counter_sub_run, u8_counter_step, logHistory);
        if (result != SyntheticResult::SUCCESS) {
            qDebug() << "Step processing failed with result:" << syntheticResultToString(result);
            return result;
        }

        // Delay between steps
        delay_ui.delay_ms(SyntheticConfig::DELAY_BETWEEN_STEPS_MS);
        qDebug() << "Step" << u8_counter_step + 1 << "completed successfully";
    }

    qDebug() << "Base" << global_var.signal_status_oligo.u16_counter_base_finished + 1 << "completed successfully";
    return SyntheticResult::SUCCESS;
}

/**
 * @brief Handle pause state during operation
 */
void SYNO24::handleSyntheticPauseState() {
    if (syno24_machine.getAutoState() == State::PAUSE) {
        log_terminal("SYSTEM PAUSED", Error);
        qDebug() << "System paused - waiting for resume";

        while (syno24_machine.getAutoState() == State::PAUSE) {
            delay_ui.delay_ms(100);
            if (syno24_machine.getAutoState() == State::STOPED) {
                qDebug() << "System stopped during pause";
                break;
            }
        }

        if (syno24_machine.getAutoState() == State::RUNNING) {
            qDebug() << "System resumed from pause";
            log_terminal("SYSTEM RESUMED", Info);
        }
    }
}

/**
 * @brief Process a single step - Core chemical processing logic
 */
SyntheticResult SYNO24::processSyntheticStep(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, LogHistory& logHistory) {
    qDebug() << "=== PROCESSING STEP" << u8_counter_step + 1 << "===";

    // Update UI state
    global_var.updateSTTRun2UI.currentStep = u8_counter_step;
    UpdateUISTTRun(0);
    volume.save_parameter_valve();
    volume.ReloadUIVolumeMNG();

    // Get chemical type and log step
    uint8_t chemical_type = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u8_first_type_chemical;
    uint8_t u8_first_chemical_temp = get_FirstChemical(chemical_type);

    QString step_name = (chemical_type < STEP_NAME.size()) ? STEP_NAME[chemical_type] : "UNKNOWN";
    log_terminal_append("        * step : " + QString::number(u8_counter_step + 1) + " " + step_name, Normal);
    logHistory.appendToLog("        * step : " + QString::number(u8_counter_step + 1) + " " + step_name);

    ui->textEdit_status_update_fw->append("STEP RUN : " + QString::number(u8_counter_step));
    qDebug() << "Chemical type:" << chemical_type << "First chemical:" << u8_first_chemical_temp << "Step name:" << step_name;

    // Send oxidation sequence (critical for proper chemical processing)
    SyntheticResult oxResult = sendOxidationSequenceupdate();
    if (oxResult != SyntheticResult::SUCCESS) {
        qDebug() << "Oxidation sequence failed";
        return oxResult;
    }

    // Process chemical-specific logic
    SyntheticResult result = processSyntheticStepChemical(u8_counter_sub_run, u8_counter_step, u8_first_chemical_temp);
    if (result != SyntheticResult::SUCCESS) {
        qDebug() << "Chemical processing failed";
        return result;
    }

    qDebug() << "Step" << u8_counter_step + 1 << "processing completed successfully";
    return SyntheticResult::SUCCESS;
}

/**
 * @brief Send oxidation sequence command
 */
SyntheticResult SYNO24::sendOxidationSequenceupdate() {
    qDebug() << "Sending oxidation sequence for base" << global_var.signal_status_oligo.u16_counter_base_finished;

    QByteArray Command_send(LENGTH_COMMAND_SEND, 0);
    Command_send[0] = CMD_OX_SENQUENCE;

    for (uint8_t u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++) {
        uint8_t linkage = global_var.amidite_well[u8_idx_well].linkages[global_var.signal_status_oligo.u16_counter_base_finished];
        uint8_t sequence ;//= global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];
        // Fill linkages and sequences for all wells
        uint8_t sequence_val = global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];
        // Check if this is the last base of the well
        if (global_var.signal_status_oligo.u16_counter_base_finished >= global_var.amidite_well[u8_idx_well].LastbaseinWellCount) {
            if (isInCouplingLastBase(sequence_val)) {
                sequence = CHEMICAL_SUBTANCE_EMPTY;
                qDebug() << " Coupling base nay thuoc base dac biet nen = CHEMICAL_SUBTANCE_EMPTY : "<< u8_idx_well;
            } else {
                if(isInCoupling2(sequence_val) || isInCoupling1(sequence_val))
                {
                    if(isInCoupling2(sequence_val))
                    {
                        sequence = isInCoupling2(sequence_val) ? sequence_val : CHEMICAL_SUBTANCE_EMPTY;
                    }
                    else
                    {
                        sequence = isInCoupling1(sequence_val) ? sequence_val : CHEMICAL_SUBTANCE_EMPTY;
                    }
                }
                else
                {
                    sequence = CHEMICAL_SUBTANCE_EMPTY;
                }
                //qDebug() << " Coupling 2 = sequence_val : "<< u8_idx_well;
            }
        } else {
            if(isInCoupling2(sequence_val) || isInCoupling1(sequence_val))
            {
                if(isInCoupling2(sequence_val))
                {
                    sequence = isInCoupling2(sequence_val) ? sequence_val : CHEMICAL_SUBTANCE_EMPTY;
                }
                else
                {
                    sequence = isInCoupling1(sequence_val) ? sequence_val : CHEMICAL_SUBTANCE_EMPTY;
                }
            }
            else
            {
                sequence = CHEMICAL_SUBTANCE_EMPTY;
            }
        } // end so sanh base dang chay voi base cuoi cung

        Command_send[u8_idx_well + 1] = linkage;
        Command_send[u8_idx_well + 97] = sequence;

        if (linkage == 2) {
            qDebug() << "Well" << u8_idx_well << "OX2";
        }
    }

    // Add base counter
    Command_send[150] = (global_var.signal_status_oligo.u16_counter_base_finished) & 0xFF;
    Command_send[151] = (global_var.signal_status_oligo.u16_counter_base_finished >> 8) & 0xFF;

    if (!STM32_COM.serial_send_command_Firmware(serialPort, Command_send, SyntheticConfig::COMMAND_TIMEOUT_MS)) {
        qDebug() << "Failed to send oxidation sequence";
        return SyntheticResult::FIRMWARE_ERROR;
    }

    qDebug() << "Oxidation sequence sent successfully";
    return SyntheticResult::SUCCESS;
}

/**
 * @brief Process chemical-specific step logic
 */
SyntheticResult SYNO24::processSyntheticStepChemical(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp) {
    qDebug() << "Processing chemical type:" << u8_first_chemical_temp;

    // Handle coupling/capping chemicals (complex logic)
    if ((u8_first_chemical_temp == COUPLING) || (u8_first_chemical_temp == CAPPING) || (u8_first_chemical_temp == COUPLING2)) {
        qDebug() << "Processing coupling/capping chemical";
        return processSyntheticCouplingStep(u8_counter_sub_run, u8_counter_step, u8_first_chemical_temp);
    } else {
        // Handle non-coupling chemicals (simpler logic)
        qDebug() << "Processing non-coupling chemical";
        return processSyntheticNonCouplingStep(u8_counter_sub_run, u8_counter_step, u8_first_chemical_temp);
    }
}

/**
 * @brief Process coupling step (COUPLING, COUPLING2, CAPPING) - Complex logic from original
 */
SyntheticResult SYNO24::processSyntheticCouplingStep(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp) {
    qDebug() << "Processing coupling step - chemical type:" << u8_first_chemical_temp;

    // Define constants (exactly as original)
    const quint8 idx_start_opt_vaccum = 50;
    const quint8 idx_start_time_process = 60;
    const quint8 idx_start_time_wait = 80;
    const quint8 idx_start_time_fill_mixfunction_1 = 100;
    const quint8 idx_start_time_fill_mixfunction_2 = 150;
    const quint8 idx_start_time_fill_mixfunction_3 = 200;
    const quint16 idx_start_sequence_amidite = 290;

    QByteArray Command_send(LENGTH_COMMAND_SEND, 0);
    Command_send[0] = CMD_DATA_OLIGO;
    Command_send[1] = u8_first_chemical_temp;

    TwoByte_to_u16 u16tb_Timefill_Volume_function_mix[3];
    // Calculate process time using centralized function
    uint32_t u32_time_oligo_process_step = calculateProcessTime(u8_counter_sub_run, u8_counter_step);

    // Process mix functions (exactly as original logic)
    for (quint8 idx_mix_fnc = 0; idx_mix_fnc < 3; idx_mix_fnc++) {
        u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc].Data;

        // Set volume in command
        Command_send[5 + idx_mix_fnc * 2] = u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Byte[0];
        Command_send[6 + idx_mix_fnc * 2] = u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Byte[1];
        Command_send[11 + idx_mix_fnc] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc];

        //qDebug() << "Mix function" << idx_mix_fnc << "volume:" << u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data;

        // Handle sequence assignment based on chemical type
        if (u8_first_chemical_temp == COUPLING) {
            // COUPLING1 logic
            for (quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++) {
                uint8_t sequence_val = global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];

                // Check if this is the last base of the well
                qDebug() << " u16_counter_base_finished : "<< global_var.signal_status_oligo.u16_counter_base_finished
                         << "LastbaseinWellCount" << global_var.amidite_well[u8_idx_well].LastbaseinWellCount;
                if (global_var.signal_status_oligo.u16_counter_base_finished >= global_var.amidite_well[u8_idx_well].LastbaseinWellCount) {
                    if (isInCouplingLastBase(sequence_val)) {
                        Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
                        qDebug() << " Coupling base nay thuoc base dac biet nen = CHEMICAL_SUBTANCE_EMPTY : "<< u8_idx_well;
                    } else {
                        Command_send[idx_start_sequence_amidite + u8_idx_well] = isInCoupling1(sequence_val) ? sequence_val : CHEMICAL_SUBTANCE_EMPTY;
                        qDebug() << " Coupling 1 = sequence_val: "<< u8_idx_well;
                    }
                } else {
                    Command_send[idx_start_sequence_amidite + u8_idx_well] = isInCoupling1(sequence_val) ? sequence_val : CHEMICAL_SUBTANCE_EMPTY;
                }

                global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data = u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data;
            }
        } else if (u8_first_chemical_temp == COUPLING2) {
            // COUPLING2 logic
            for (quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++) {
                uint8_t sequence_val = global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];
                // Check if this is the last base of the well
                if (global_var.signal_status_oligo.u16_counter_base_finished >= global_var.amidite_well[u8_idx_well].LastbaseinWellCount) {
                    if (isInCouplingLastBase(sequence_val)) {
                        Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
                        qDebug() << " Coupling base nay thuoc base dac biet nen = CHEMICAL_SUBTANCE_EMPTY : "<< u8_idx_well;
                    } else {
                        Command_send[idx_start_sequence_amidite + u8_idx_well] = isInCoupling2(sequence_val) ? sequence_val : CHEMICAL_SUBTANCE_EMPTY;
                        qDebug() << " Coupling 2 = sequence_val : "<< u8_idx_well;
                    }
                } else {
                    Command_send[idx_start_sequence_amidite + u8_idx_well] = isInCoupling2(sequence_val) ? sequence_val : CHEMICAL_SUBTANCE_EMPTY;
                }

                global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data = u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data;
            }
        } else {
            // CAPPING logic
            for (quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++) {
                uint8_t sequence_val = global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];
                Command_send[idx_start_sequence_amidite + u8_idx_well] = sequence_val;
                global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data = u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data;
            }
        }
    }

    // Continue with command completion and execution
    return completeCouplingCommand(u8_counter_sub_run, u8_counter_step, u8_first_chemical_temp, Command_send, u32_time_oligo_process_step);
}

/**
 * @brief Complete coupling command setup and execution
 */
SyntheticResult SYNO24::completeCouplingCommand(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp, QByteArray& Command_send, uint32_t& u32_time_oligo_process_step) {
    qDebug() << "Completing coupling command setup";
    const quint8 idx_start_time_fill_mixfunction_1 = 100;
    const quint8 idx_start_time_fill_mixfunction_2 = 150;
    const quint8 idx_start_time_fill_mixfunction_3 = 200;
    // Add common parameters (exactly as original)
    Command_send[4] = protocol_oligo.sub[u8_counter_sub_run].douple_coupling_option;
    Command_send[14] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_wait_after_fill.Byte[0];
    Command_send[15] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_wait_after_fill.Byte[1];
    Command_send[16] = global_var.status_and_sensor.flag_enable_auto_control_air_Nito;

    // Handle special base
    uint16_t current_base_number = global_var.signal_status_oligo.u16_counter_base_finished + 1;
    if (isbaseSpecial(current_base_number, protocol_oligo.speacial_base)) {
        Command_send[17] = true;
        Command_send[18] = protocol_oligo.u16_scale_volume.Byte[0];
        Command_send[19] = protocol_oligo.u16_scale_volume.Byte[1];
        Command_send[20] = protocol_oligo.u16_scale_time.Byte[0];
        Command_send[21] = protocol_oligo.u16_scale_time.Byte[1];
        qDebug() << "Special base detected:" << current_base_number;
    } else {
        Command_send[17] = false;
    }

    // Add step and auto-priming info
    Command_send[26] = u8_counter_step;
    Command_send[27] = global_var.advanced_setting.flag_auto_primming_chemical;
    Command_send[28] = global_var.advanced_setting.u16tb_autoPrim_volume_amidite.Byte[0];
    Command_send[29] = global_var.advanced_setting.u16tb_autoPrim_volume_amidite.Byte[1];
    Command_send[30] = global_var.advanced_setting.u16tb_autoPrim_volume_Activator.Byte[0];
    Command_send[31] = global_var.advanced_setting.u16tb_autoPrim_volume_Activator.Byte[1];

    // Add pressure control parameters and calculate timeout
    for (uint8_t i = 0; i < 10; i++) {
        // 01-08-2025 cập nhật thành cho phép chọn lựa nhân hay không nhân tỉ lệ
        bool LowpushMul = ui->chkbx_LowPushMul->isChecked();
        bool HighpushMul = ui->chkbx_HighPushMul->isChecked();
        uint8_t option = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u8_option_pressure[i];

        Command_send[50 + i] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u8_option_pressure[i];

        uint16_t process_time = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_procs_time[i].Data;


        uint16_t wait_time = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[i].Data;
        if(option == LOW_PUSH && LowpushMul)
        {
            process_time = process_time * m_tableDataMultiplier.multiplier / 100;
            qDebug() << "LOW_PUSH multiplier applied:" << process_time;
        }
        else if(option == HIGH_PUSH && HighpushMul)
        {
            process_time = process_time * m_tableDataMultiplier.multiplier / 100;
            qDebug() << "HIGH_PUSH multiplier applied:" << process_time;
        }
        else
        {
            // Mặc định giữ nguyên giá trị
            process_time = process_time;
            qDebug() << "No multiplier applied for option:" << option;
        }

        Command_send[60 + i * 2] = process_time & 0xFF;
        Command_send[61 + i * 2] = (process_time >> 8) & 0xFF;

        Command_send[80 + i * 2] = wait_time & 0xFF;
        Command_send[81 + i * 2] = (wait_time >> 8) & 0xFF;

        u32_time_oligo_process_step += process_time + (wait_time * SCALE_LEVEL_WAITTIME);
    }
    // từ byte 100 đến 147
    for(uint8_t u8_time_fill_idx = 0; u8_time_fill_idx < MAX_WELL_AMIDITE; u8_time_fill_idx++)
    {
        Command_send[idx_start_time_fill_mixfunction_1 + u8_time_fill_idx*2]       = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[0].Byte[0];
        Command_send[idx_start_time_fill_mixfunction_1 + u8_time_fill_idx*2 +1]    = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[0].Byte[1];
    }
    // tu byte 150 den byte 197
    for(uint8_t u8_time_fill_idx = 0; u8_time_fill_idx < MAX_WELL_AMIDITE; u8_time_fill_idx++)
    {
        Command_send[idx_start_time_fill_mixfunction_2 + u8_time_fill_idx*2]       = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[1].Byte[0];
        Command_send[idx_start_time_fill_mixfunction_2 + u8_time_fill_idx*2 +1]    = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[1].Byte[1];
    }
    // tu byte 200 den byte 247
    for(uint8_t u8_time_fill_idx = 0; u8_time_fill_idx < MAX_WELL_AMIDITE; u8_time_fill_idx++)
    {
        Command_send[idx_start_time_fill_mixfunction_3 + u8_time_fill_idx*2]       = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[2].Byte[0];
        Command_send[idx_start_time_fill_mixfunction_3 + u8_time_fill_idx*2 +1]    = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[2].Byte[1];
    }

    const quint16 idx_VacuumBox = 350; //
    if(isbaseSpecial((global_var.signal_status_oligo.u16_counter_base_finished +1), m_baseVacuumBox.speacial_base))
    {
        int currentIdx = idx_VacuumBox; // Biến theo dõi chỉ số hiện tại
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.Enablefeature;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_WASH;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Deblock;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Coupling;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Ox;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Cap;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.time.Byte[0];
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.time.Byte[1];
        //currentIdx++;
    }else
    {
        int currentIdx = idx_VacuumBox; // Biến theo dõi chỉ số hiện tại
        Command_send[currentIdx]= false;
    }
    // Adjust timeout for oxidation
    if (u8_first_chemical_temp == OXIDATION_IODINE || u8_first_chemical_temp == OXIDATION_IODINE2) {
        u32_time_oligo_process_step *= SyntheticConfig::OXIDATION_TIME_MULTIPLIER;
        qDebug() << "Oxidation timeout multiplied";
    }
    qDebug() << "Calculated step timeout:" << u32_time_oligo_process_step << "ms";
    // Send main command
    if (!STM32_COM.serial_send_command_Firmware(serialPort, Command_send, SyntheticConfig::COMMAND_TIMEOUT_MS)) {
        qDebug() << "Failed to send coupling command";
        return SyntheticResult::FIRMWARE_ERROR;
    }

    // Execute step
    QByteArray start_command(LENGTH_COMMAND_SEND, 0);
    start_command[0] = CMD_START_OLIGO_STEP;

    if (!STM32_COM.serial_send_command_Firmware(serialPort, start_command, u32_time_oligo_process_step)) {
        qDebug() << "Failed to execute coupling step";
        return SyntheticResult::FIRMWARE_ERROR;
    }

    log_terminal(" >> Completed", Success);
    qDebug() << "Coupling step executed successfully";

    // Handle double coupling if needed
    if (u8_first_chemical_temp == COUPLING) {
        return handleDoubleCoupling(u8_counter_sub_run, u8_counter_step, u8_first_chemical_temp, u32_time_oligo_process_step);
    }

    return SyntheticResult::SUCCESS;
}

/**
 * @brief Process non-coupling step (other chemicals)
 */
SyntheticResult SYNO24::processSyntheticNonCouplingStep(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp) {
    qDebug() << "Processing non-coupling step - chemical type:" << u8_first_chemical_temp;
    // Define constants (exactly as original)
    const quint8 idx_start_opt_vaccum = 50;
    const quint8 idx_start_time_process = 60;
    const quint8 idx_start_time_wait = 80;
    const quint8 idx_start_time_fill_mixfunction_1 = 100;
    const quint8 idx_start_time_fill_mixfunction_2 = 150;
    const quint8 idx_start_time_fill_mixfunction_3 = 200;
    const quint16 idx_start_sequence_amidite = 290;
    QByteArray Command_send(LENGTH_COMMAND_SEND, 0);
    Command_send[0] = CMD_DATA_OLIGO;
    Command_send[1] = u8_first_chemical_temp;

    // Get volume for non-coupling chemical
    TwoByte_to_u16 u16tb_Timefill_Volume_first_type;
    u16tb_Timefill_Volume_first_type.Data = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_Volume.Data;

    Command_send[2] = u16tb_Timefill_Volume_first_type.Byte[0];
    Command_send[3] = u16tb_Timefill_Volume_first_type.Byte[1];

    qDebug() << "Non-coupling volume:" << u16tb_Timefill_Volume_first_type.Data;

    // Add common parameters
    Command_send[4] = protocol_oligo.sub[u8_counter_sub_run].douple_coupling_option;
    Command_send[14] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_wait_after_fill.Byte[0];
    Command_send[15] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_wait_after_fill.Byte[1];
    Command_send[16] = global_var.status_and_sensor.flag_enable_auto_control_air_Nito;

    // Handle special base
    uint16_t current_base_number = global_var.signal_status_oligo.u16_counter_base_finished + 1;
    if (isbaseSpecial(current_base_number, protocol_oligo.speacial_base)) {
        Command_send[17] = true;
        Command_send[18] = protocol_oligo.u16_scale_volume.Byte[0];
        Command_send[19] = protocol_oligo.u16_scale_volume.Byte[1];
        Command_send[20] = protocol_oligo.u16_scale_time.Byte[0];
        Command_send[21] = protocol_oligo.u16_scale_time.Byte[1];
        qDebug() << "Special base detected:" << current_base_number;
    } else {
        Command_send[17] = false;
    }

    // Add step and auto-priming info
    Command_send[26] = u8_counter_step;
    Command_send[27] = global_var.advanced_setting.flag_auto_primming_chemical;
    Command_send[28] = global_var.advanced_setting.u16tb_autoPrim_volume_amidite.Byte[0];
    Command_send[29] = global_var.advanced_setting.u16tb_autoPrim_volume_amidite.Byte[1];
    Command_send[30] = global_var.advanced_setting.u16tb_autoPrim_volume_Activator.Byte[0];
    Command_send[31] = global_var.advanced_setting.u16tb_autoPrim_volume_Activator.Byte[1];

    // Calculate process time using centralized function
    uint32_t u32_time_oligo_process_step = calculateProcessTime(u8_counter_sub_run, u8_counter_step);

    // Add pressure control parameters
    for (uint8_t i = 0; i < 10; i++) {
//        Command_send[50 + i] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u8_option_pressure[i];

//        uint16_t process_time = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_procs_time[i].Data;
//        Command_send[60 + i * 2] = process_time & 0xFF;
//        Command_send[61 + i * 2] = (process_time >> 8) & 0xFF;

//        uint16_t wait_time = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[i].Data;
//        Command_send[80 + i * 2] = wait_time & 0xFF;
//        Command_send[81 + i * 2] = (wait_time >> 8) & 0xFF;
        bool LowpushMul = ui->chkbx_LowPushMul->isChecked();
        bool HighpushMul = ui->chkbx_HighPushMul->isChecked();
        uint8_t option = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u8_option_pressure[i];

        Command_send[50 + i] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u8_option_pressure[i];

        uint16_t process_time = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_procs_time[i].Data;


        uint16_t wait_time = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[i].Data;
        if(option == LOW_PUSH && LowpushMul)
        {
            process_time = process_time * m_tableDataMultiplier.multiplier / 100;
            qDebug() << "LOW_PUSH multiplier applied:" << process_time;
        }
        else if(option == HIGH_PUSH && HighpushMul)
        {
            process_time = process_time * m_tableDataMultiplier.multiplier / 100;
            qDebug() << "HIGH_PUSH multiplier applied:" << process_time;
        }
        else
        {
            // Mặc định giữ nguyên giá trị
            //process_time = process_time;
            qDebug() << "No multiplier applied for option:" << option;
        }
        Command_send[60 + i * 2] = process_time & 0xFF;
        Command_send[61 + i * 2] = (process_time >> 8) & 0xFF;

        Command_send[80 + i * 2] = wait_time & 0xFF;
        Command_send[81 + i * 2] = (wait_time >> 8) & 0xFF;
    }


    for (quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++) {
        uint8_t sequence_val = global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];

        // Check if this is the last base of the well
        if (global_var.signal_status_oligo.u16_counter_base_finished >= global_var.amidite_well[u8_idx_well].LastbaseinWellCount) {
            if (isInCouplingLastBase(sequence_val)) {
                Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
                qDebug() << " Coupling base nay thuoc base dac biet nen = CHEMICAL_SUBTANCE_EMPTY : "<< u8_idx_well;
            } else {
                if(isInCoupling2(sequence_val) || isInCoupling1(sequence_val))
                {
                    if(isInCoupling2(sequence_val))
                    {
                        Command_send[idx_start_sequence_amidite + u8_idx_well] = isInCoupling2(sequence_val) ? sequence_val : CHEMICAL_SUBTANCE_EMPTY;
                    }
                    else
                    {
                        Command_send[idx_start_sequence_amidite + u8_idx_well] = isInCoupling1(sequence_val) ? sequence_val : CHEMICAL_SUBTANCE_EMPTY;
                    }
                }
                else
                {
                    Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
                }
                //qDebug() << " Coupling 2 = sequence_val : "<< u8_idx_well;
            }
        } else {
            if(isInCoupling2(sequence_val) || isInCoupling1(sequence_val))
            {
                if(isInCoupling2(sequence_val))
                {
                    Command_send[idx_start_sequence_amidite + u8_idx_well] = isInCoupling2(sequence_val) ? sequence_val : CHEMICAL_SUBTANCE_EMPTY;
                }
                else
                {
                    Command_send[idx_start_sequence_amidite + u8_idx_well] = isInCoupling1(sequence_val) ? sequence_val : CHEMICAL_SUBTANCE_EMPTY;
                }
            }
            else
            {
                Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
            }
        } // end so sanh base dang chay voi base cuoi cung
    }
    qDebug() << "Calculated step timeout:" << u32_time_oligo_process_step << "ms";
    const quint16 idx_VacuumBox = 350; // 20byte tu 90 den 109
    if(isbaseSpecial((global_var.signal_status_oligo.u16_counter_base_finished +1), m_baseVacuumBox.speacial_base))
    {
        int currentIdx = idx_VacuumBox; // Biến theo dõi chỉ số hiện tại
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.Enablefeature;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_WASH;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Deblock;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Coupling;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Ox;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Cap;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.time.Byte[0];
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.time.Byte[1];
        //currentIdx++;
    }else
    {
        int currentIdx = idx_VacuumBox; // Biến theo dõi chỉ số hiện tại
        Command_send[currentIdx]= false;
    }
    // Adjust timeout for oxidation
    if (u8_first_chemical_temp == OXIDATION_IODINE || u8_first_chemical_temp == OXIDATION_IODINE2) {
        u32_time_oligo_process_step *= SyntheticConfig::OXIDATION_TIME_MULTIPLIER;
        qDebug() << "Oxidation timeout multiplied";
    }
    // Send main command
    if (!STM32_COM.serial_send_command_Firmware(serialPort, Command_send, SyntheticConfig::COMMAND_TIMEOUT_MS)) {
        qDebug() << "Failed to send non-coupling command";
        return SyntheticResult::FIRMWARE_ERROR;
    }

    // Execute step
    QByteArray start_command(LENGTH_COMMAND_SEND, 0);
    start_command[0] = CMD_START_OLIGO_STEP;

    if (!STM32_COM.serial_send_command_Firmware(serialPort, start_command, u32_time_oligo_process_step)) {
        qDebug() << "Failed to execute non-coupling step";
        return SyntheticResult::FIRMWARE_ERROR;
    }

    log_terminal(" >> Completed", Success);
    qDebug() << "Non-coupling step executed successfully";

    return SyntheticResult::SUCCESS;
}

/**
 * @brief Handle double coupling operation
 */
SyntheticResult SYNO24::handleDoubleCoupling(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp, uint32_t process_time) {
    uint8_t coupling_option = protocol_oligo.sub[u8_counter_sub_run].douple_coupling_option;
    uint8_t current_base_in_sub = global_var.signal_status_oligo.u16_counter_base_finished -
            (u8_counter_sub_run > 0 ? protocol_oligo.sub[u8_counter_sub_run - 1].u8_number_base_on_sub : 0);

    bool should_double = false;

    switch (coupling_option) {
    case DOUBLE_COUPLING_FIRSTBASE:
        should_double = (current_base_in_sub == 0);
        break;
    case DOUBLE_COUPLING_FIRST_SECOND_BASE:
        should_double = (current_base_in_sub == 0 || current_base_in_sub == 1);
        break;
    case DOUBLE_COUPLING_ALL_BASE:
        should_double = true;
        break;
    default:
        should_double = false;
        break;
    }

    if (!should_double) {
        qDebug() << "Double coupling not required for this base";
        return SyntheticResult::SUCCESS;
    }

    log_terminal("Run Double Coupling", Info);
    qDebug() << "Executing double coupling for base" << current_base_in_sub;

    QByteArray double_command(LENGTH_COMMAND_SEND, 0);
    double_command[0] = CMD_START_OLIGO_STEP;

    if (!STM32_COM.serial_send_command_Firmware(serialPort, double_command, process_time)) {
        log_terminal("Double coupling failed", Error);
        qDebug() << "Double coupling execution failed";
        return SyntheticResult::FIRMWARE_ERROR;
    }

    log_terminal("Double coupling completed", Success);
    qDebug() << "Double coupling executed successfully";
    return SyntheticResult::SUCCESS;
}

/**
 * @brief Finalize synthetic operation and cleanup
 */


SyntheticResult SYNO24::finalizeSyntheticOperation(LogHistory& logHistory) {
    qDebug() << "=== FINALIZING SYNTHETIC OPERATION ===";

    // Set system to stopped state
    syno24_machine.setAutoState(State::STOPED);
    qDebug() << "System state set to STOPPED";

    // Run to home position
    //on_btn_Run2HomeStep_released();
    qDebug() << "System moved to home position";

    // Handle exhaust fan settings
    global_var.advanced_setting.flag_exhaustFan = ui->checkbox_exhaustFan->isChecked();
    global_var.advanced_setting.u16tb_timeExhaustFan.Data = ui->spinbx_exhaustFan->value();

    qDebug() << "Exhaust fan enabled:" << global_var.advanced_setting.flag_exhaustFan;
    qDebug() << "Exhaust fan time:" << global_var.advanced_setting.u16tb_timeExhaustFan.Data << "minutes";

    // Send stop command with exhaust fan configuration
    QByteArray Command_send(LENGTH_COMMAND_SEND, 0);
    Command_send[0] = CMD_STOP_SYSTHETIC_OLIGO;
    Command_send[1] = global_var.advanced_setting.flag_exhaustFan;
    Command_send[2] = global_var.advanced_setting.u16tb_timeExhaustFan.Byte[0];
    Command_send[3] = global_var.advanced_setting.u16tb_timeExhaustFan.Byte[1];

    uint32_t timeExhaustFan = global_var.advanced_setting.u16tb_timeExhaustFan.Data * 60 * 1000;
    if (STM32_COM.serial_send_command_Firmware(serialPort, Command_send, timeExhaustFan)) {
        qDebug() << "STOP command sent successfully";
    } else {
        qDebug() << "Failed to send STOP command";
    }

    // Restore UI state
    ui->btn_start_synthetic->setDisabled(false);
    stopCountdown();
    timer_update_humidity_tempareture.start(SyntheticConfig::SENSOR_UPDATE_INTERVAL_MS);
    qDebug() << "UI state restored";

    // Save final state
    volume.save_parameter_valve();
    volume.ReloadUIVolumeMNG();
    qDebug() << "Volume parameters saved";

    // Finalize logging
    logHistory.appendToLog(ui->textEdit_oligo_history_log->toPlainText());
    logHistory.appendToLog("Finish Run Protocol");
    logHistory.setLogFileReadOnly();
    qDebug() << "Logging finalized";

    log_terminal("Finish Synthesis Oligo", Success);
    qDebug() << "Synthetic operation finalized successfully";

    return SyntheticResult::SUCCESS;
}


/**
 * @brief Main optimized function - Replaces the original 738-line monolithic function
 */
SyntheticResult SYNO24::on_btn_start_synthetic_released_optimized() {
    qDebug() << "========== STARTING OPTIMIZED SYNTHETIC OPERATION ==========";
    qDebug() << "========== STARTING OPTIMIZED SYNTHETIC OPERATION ==========";
    uint8_t initial_sequence[MAX_WELL_AMIDITE];
    for (int i = 0; i < MAX_WELL_AMIDITE; ++i) {
        initial_sequence[i] = global_var.amidite_well[i].u8_sequence[0]; // Giả sử giá trị khác 127 là có hóa chất
    }
    m_tableDataMultiplier.totalInitialWells  = ChemicalWells::calculateTotalChemicalWells(initial_sequence);
    qDebug() << "Total well after Run (100%):" << m_tableDataMultiplier.totalInitialWells;
    // Create log history
    QFileInfo fileInfo(filemanager.protocol_Path);
    QString baseName = fileInfo.baseName();
    qDebug() << "Log history Run Path:" << baseName;
    LogHistory logHistory(baseName);

    // Step 1: Initialize operation
    qDebug() << "Step 1: Initializing operation";
    SyntheticResult result = initializeSyntheticOperation(logHistory);
    if (result != SyntheticResult::SUCCESS) {
        qDebug() << "Initialization failed:" << syntheticResultToString(result);
        return result;
    }

    // Step 2: Wait for humidity if required
    qDebug() << "Step 2: Checking humidity requirements";
    result = waitForSyntheticHumidity();
    if (result != SyntheticResult::SUCCESS) {
        qDebug() << "Humidity wait failed:" << syntheticResultToString(result);
        return result;
    }

    // Step 3: Verify system is still running and connected
    qDebug() << "Step 3: Verifying system state";
    if (syno24_machine.getAutoState() != State::RUNNING || !STM32_COM.flag_connecttion) {
        qDebug() << "System state changed or connection lost";
        qDebug() << "Current state:" << (int)syno24_machine.getAutoState() << "Connection:" << STM32_COM.flag_connecttion;
        return SyntheticResult::CONNECTION_ERROR;
    }

    // Step 4: Calculate protocol parameters
    qDebug() << "Step 4: Calculating protocol parameters";
    uint8_t u8_number_sub_run, u8_lastest_sub;
    uint16_t u16_max_base_setting_protocol;
    result = calculateSyntheticTotalBases(u8_number_sub_run, u8_lastest_sub, u16_max_base_setting_protocol);
    if (result != SyntheticResult::SUCCESS) {
        qDebug() << "Protocol calculation failed:" << syntheticResultToString(result);
        return result;
    }

    // Step 5: Process all sub-protocols
    qDebug() << "Step 5: Processing all sub-protocols";
    result = processAllSyntheticSubProtocols(u8_number_sub_run, logHistory);
    if (result != SyntheticResult::SUCCESS && result != SyntheticResult::SEQUENCE_COMPLETE) {
        qDebug() << "Sub-protocol processing not finish:" << syntheticResultToString(result);

        //return result;
    }

    qDebug() << "========== OPTIMIZED SYNTHETIC OPERATION COMPLETED SUCCESSFULLY ==========";
    // Step 6: Process final special sub-protocol
    qDebug() << "Step 6: Processing final special sub-protocol";
    if(ui->chkbx_EnaSpecialLastBase->isChecked())
    {
        result = processFinalSpecialSubProtocol(logHistory);
        if (result != SyntheticResult::SUCCESS) {
            qDebug() << "Final special sub-protocol failed:" << syntheticResultToString(result);
            syno24_machine.setAutoState(State::STOPED);
            syno24_machine.procsess_ui();
            stopCountdown();
            //return result;
        }
    }
    int remainingPercentage;
    uint8_t current_sequence[MAX_WELL_AMIDITE];
    for (int i = 0; i < MAX_WELL_AMIDITE; ++i) {
        current_sequence[i] = global_var.amidite_well[i].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished]; // Vẫn có hóa chất
    }
    QPair<int, double> results = ChemicalWells::checkRemainingChemicalWells(current_sequence, m_tableDataMultiplier.totalInitialWells);
    remainingPercentage = results.second;
    ui->progressBar_percentRun->setValue(100.0 - remainingPercentage);
    qDebug() << "Số giếng còn lại chứa hóa chất:" << results.first;
    qDebug() << "Tỉ lệ phần trăm giếng còn lại:" << QString::number(results.second, 'f', 2) + "%";

    // Step 7: Finalize operation
    qDebug() << "Step 7: Finalizing operation";
    result = finalizeSyntheticOperation(logHistory);
    if (result != SyntheticResult::SUCCESS) {
        qDebug() << "Finalization failed:" << syntheticResultToString(result);
        //return result;
    }
    return SyntheticResult::SUCCESS;
}


/**
 * @brief Process final special sub-protocol with special coupling logic for LAST SEQUENCES
 * This sub is different from others in processSyntheticCouplingStep:
 * - Only processes the LAST BASE of each well sequence (stored in lastSequence)
 * - If coupling and lastSequence is isInCouplingLastBase: use that sequence character
 * - If coupling and lastSequence is NOT isInCouplingLastBase: send CHEMICAL_SUBTANCE_EMPTY
 */
SyntheticResult SYNO24::processFinalSpecialSubProtocol(LogHistory& logHistory) {
    qDebug() << "=== PROCESSING FINAL SPECIAL SUB-PROTOCOL FOR LAST SEQUENCES ===";

    // Check if machine is still running
    if (syno24_machine.getAutoState() == State::STOPED) {
        qDebug() << "User stopped during final special sub processing";
        return SyntheticResult::USER_STOPPED;
    }

    // Log start of final special sub
    log_terminal("Running final special sub-protocol for last sequences", Info);
    logHistory.appendToLog("Running final special sub-protocol for last sequences");

    // Use the last sub-protocol configuration as template
    uint8_t template_sub_index = 9;
    if (template_sub_index >= MAX_SUB_OF_PROTOCOL) {
        qDebug() << "Invalid template sub index:" << template_sub_index;
        return SyntheticResult::INVALID_PARAMETER;
    }

    // Update UI state for final special sub
    global_var.updateSTTRun2UI.currentSub = template_sub_index;
    uint8_t u8_number_base_run = 1; // Final special sub runs only 1 base (the LAST base of all sequences)
    uint8_t u8_number_step_run = protocol_oligo.sub[template_sub_index].u8_number_step_on_base;

    qDebug() << "Final special sub - processing LAST sequences only";
    qDebug() << "Final special sub - bases to run:" << u8_number_base_run;
    qDebug() << "Final special sub - steps per base:" << u8_number_step_run;

    // Process single base with special logic
    for (uint8_t u8_counter_base = 0; u8_counter_base < u8_number_base_run; u8_counter_base++) {

        if (syno24_machine.getAutoState() == State::STOPED) {
            qDebug() << "User stopped during final special base processing";
            return SyntheticResult::USER_STOPPED;
        }

        qDebug() << "Processing final special base (LAST sequences)" << u8_counter_base + 1;

        // Update base counter for final special processing
        //global_var.updateSTTRun2UI.currentBase = u8_counter_base;
        // Note: Do NOT increment u16_counter_base_finished as we're processing LAST sequences specifically

        // Process each step in the final special base (using lastSequence from each well)
        for (uint8_t u8_counter_step = 0; u8_counter_step < u8_number_step_run; u8_counter_step++) {
            GetFeatureVacuumBox();
            if (syno24_machine.getAutoState() == State::STOPED) {
                qDebug() << "User stopped during final special step processing";
                return SyntheticResult::USER_STOPPED;
            }

            qDebug() << "Processing final special step" << u8_counter_step + 1;

            SyntheticResult result = processFinalSpecialStep(template_sub_index, u8_counter_step, logHistory);
            if (result != SyntheticResult::SUCCESS) {
                qDebug() << "Final special step processing failed:" << syntheticResultToString(result);
                return result;
            }

            // Delay between steps
            delay_ui.delay_ms(SyntheticConfig::DELAY_BETWEEN_STEPS_MS);
        }

        // Delay between bases (if multiple bases in future)
        delay_ui.delay_ms(SyntheticConfig::DELAY_BETWEEN_BASES_MS);
    }

    qDebug() << "Final special sub-protocol for last sequences completed successfully";
    log_terminal("Final special sub-protocol for last sequences completed", Success);
    logHistory.appendToLog("Final special sub-protocol for last sequences completed");

    return SyntheticResult::SUCCESS;
}

/**
 * @brief Process a single step in final special sub-protocol
 * Uses special coupling logic different from normal sub-protocols
 */
SyntheticResult SYNO24::processFinalSpecialStep(uint8_t template_sub_index, uint8_t u8_counter_step, LogHistory& logHistory) {
    qDebug() << "=== PROCESSING FINAL SPECIAL STEP" << u8_counter_step + 1 << "===";

    // Update UI state
    global_var.updateSTTRun2UI.currentStep = u8_counter_step;
    UpdateUISTTRun(0);
    volume.save_parameter_valve();
    volume.ReloadUIVolumeMNG();

    // Get chemical type from template sub
    uint8_t chemical_type = protocol_oligo.sub[template_sub_index].step[u8_counter_step].fill_chemical.u8_first_type_chemical;
    uint8_t u8_first_chemical_temp = get_FirstChemical(chemical_type);

    // Log step start
    QString step_name = STEP_NAME[chemical_type];
    log_terminal("Final special step (last sequences): " + step_name, Info);
    logHistory.appendToLog("Final special step (last sequences): " + step_name);

    qDebug() << "Final special step chemical type:" << chemical_type;
    qDebug() << "Final special step first chemical:" << u8_first_chemical_temp;
    // Send oxidation sequence (critical for proper chemical processing)
    SyntheticResult oxResult = sendFinalSpecialOxidationSequenceupdate();
    if (oxResult != SyntheticResult::SUCCESS) {
        qDebug() << "Oxidation sequence failed";
        return oxResult;
    }
    // Handle coupling/capping chemicals with SPECIAL logic
    if ((u8_first_chemical_temp == COUPLING) || (u8_first_chemical_temp == CAPPING) || (u8_first_chemical_temp == COUPLING2)) {
        qDebug() << "Processing final special coupling/capping chemical";
        return processFinalSpecialCouplingStep(template_sub_index, u8_counter_step, u8_first_chemical_temp);
    } else {
        // Handle non-coupling chemicals (use normal logic)
        qDebug() << "Processing final special non-coupling chemical";
        return processFinalSpecialSyntheticNonCouplingStep(template_sub_index, u8_counter_step, u8_first_chemical_temp);
    }
}

/**
 * @brief Process final special sub-protocol with special coupling logic for LAST SEQUENCES
 * This sub is different from others in processSyntheticCouplingStep:
 * - Only processes the LAST BASE of each well sequence (stored in lastSequence)
 * - If coupling and lastSequence is isInCouplingLastBase: use that sequence character
 * - If coupling and lastSequence is NOT isInCouplingLastBase: send CHEMICAL_SUBTANCE_EMPTY
 */
SyntheticResult SYNO24::processFinalSpecialCouplingStep(uint8_t template_sub_index, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp) {
    qDebug() << "Processing FINAL SPECIAL coupling step for LAST SEQUENCES - chemical type:" << u8_first_chemical_temp;

    // Define constants (same as processSyntheticCouplingStep)
    const quint8 idx_start_opt_vaccum = 50;
    const quint8 idx_start_time_process = 60;
    const quint8 idx_start_time_wait = 80;
    const quint8 idx_start_time_fill_mixfunction_1 = 100;
    const quint8 idx_start_time_fill_mixfunction_2 = 150;
    const quint8 idx_start_time_fill_mixfunction_3 = 200;
    const quint16 idx_start_sequence_amidite = 290;

    // Prepare command array (same structure as processSyntheticCouplingStep)
    QByteArray Command_send(LENGTH_COMMAND_SEND, 0);
    Command_send[0] = CMD_DATA_OLIGO;
    Command_send[1] = u8_first_chemical_temp;

    // Set volume from template
    TwoByte_to_u16 u16tb_Volume_temp = protocol_oligo.sub[template_sub_index].step[u8_counter_step].fill_chemical.u16tb_Volume;
    Command_send[2] = u16tb_Volume_temp.Byte[0];
    Command_send[3] = u16tb_Volume_temp.Byte[1];
    Command_send[4] = protocol_oligo.sub[template_sub_index].douple_coupling_option;

    // Set mix function volumes from template
    TwoByte_to_u16 u16tb_Timefill_Volume_function_mix[3];
    for (int idx_mix_fnc = 0; idx_mix_fnc < 3; idx_mix_fnc++) {
        u16tb_Timefill_Volume_function_mix[idx_mix_fnc] = protocol_oligo.sub[template_sub_index].step[u8_counter_step].fill_chemical.mix_funtion.u16tb_Volume[idx_mix_fnc];
        Command_send[5 + idx_mix_fnc * 2] = u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Byte[0];
        Command_send[6 + idx_mix_fnc * 2] = u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Byte[1];
        Command_send[11 + idx_mix_fnc] = protocol_oligo.sub[template_sub_index].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc];
    }

    // Calculate process time (same as normal coupling)
    uint32_t u32_time_oligo_process_step = calculateProcessTime(template_sub_index, u8_counter_step);

    // SPECIAL LAST SEQUENCE MAPPING LOGIC (only difference from normal coupling)
    qDebug() << "Applying FINAL SPECIAL last sequence mapping logic";

    // Process mix functions with SPECIAL LAST SEQUENCE logic
    for (int idx_mix_fnc = 0; idx_mix_fnc < 3; idx_mix_fnc++) {
        if (protocol_oligo.sub[template_sub_index].step[u8_counter_step].fill_chemical.mix_funtion.u8_type_chemical[idx_mix_fnc] == CLG_AMIDITE) {
            // COUPLING logic with SPECIAL LAST SEQUENCE mapping
            if (u8_first_chemical_temp == COUPLING) {
                for (quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++) {
                    // Get LAST sequence value for this well (SPECIAL LOGIC)
                    quint8 sequence_val = global_var.amidite_well[u8_idx_well].lastSequence;

                    qDebug() << "Final special COUPLING well" << u8_idx_well << "LAST sequence value:" << sequence_val;

                    // SPECIAL LOGIC: Check if lastSequence is in CouplingLastBase
                    if (isInCouplingLastBase(sequence_val)) {
                        Command_send[idx_start_sequence_amidite + u8_idx_well] = sequence_val;
                        qDebug() << "Final special COUPLING well" << u8_idx_well << "- lastSequence isInCouplingLastBase: using sequence" << sequence_val;
                    } else {
                        Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
                        qDebug() << "Final special COUPLING well" << u8_idx_well << "- lastSequence NOT isInCouplingLastBase: using EMPTY";
                    }

                    global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data = u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data;
                }
            } else if (u8_first_chemical_temp == COUPLING2) {
                // COUPLING2 logic with SPECIAL LAST SEQUENCE mapping
                for (quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++) {
                    // Get LAST sequence value for this well (SPECIAL LOGIC)
                    quint8 sequence_val = global_var.amidite_well[u8_idx_well].lastSequence;

                    //qDebug() << "Final special COUPLING2 well" << u8_idx_well << "LAST sequence value:" << sequence_val;

                    // SPECIAL LOGIC: Check if lastSequence is in CouplingLastBase
                    if (isInCouplingLastBase(sequence_val)) {
                        Command_send[idx_start_sequence_amidite + u8_idx_well] = sequence_val;
                        //qDebug() << "Final special COUPLING2 well" << u8_idx_well << "- lastSequence isInCouplingLastBase: using sequence" << sequence_val;
                    } else {
                        Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
                        //qDebug() << "Final special COUPLING2 well" << u8_idx_well << "- lastSequence NOT isInCouplingLastBase: using EMPTY";
                    }

                    global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data = u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data;
                }
            } else {
                // CAPPING logic (same as normal - all wells get EMPTY)
                for (quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++) {
                    uint8_t sequence_val = global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];
                    Command_send[idx_start_sequence_amidite + u8_idx_well] = sequence_val;
                    //qDebug() << "Final special CAPPING well" << u8_idx_well << "- using EMPTY";
                    global_var.amidite_well[u8_idx_well].u16_timefill_well[idx_mix_fnc].Data = u16tb_Timefill_Volume_function_mix[idx_mix_fnc].Data;
                }
            }
        }
    }
    quint16 u16_counter_base_finished = global_var.signal_status_oligo.u16_counter_base_finished -1;
    qDebug() << "Process Vaccumn box base " << u16_counter_base_finished;

    // từ byte 100 đến 147
    for(uint8_t u8_time_fill_idx = 0; u8_time_fill_idx < MAX_WELL_AMIDITE; u8_time_fill_idx++)
    {
        Command_send[idx_start_time_fill_mixfunction_1 + u8_time_fill_idx*2]       = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[0].Byte[0];
        Command_send[idx_start_time_fill_mixfunction_1 + u8_time_fill_idx*2 +1]    = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[0].Byte[1];
    }
    // tu byte 150 den byte 197
    for(uint8_t u8_time_fill_idx = 0; u8_time_fill_idx < MAX_WELL_AMIDITE; u8_time_fill_idx++)
    {
        Command_send[idx_start_time_fill_mixfunction_2 + u8_time_fill_idx*2]       = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[1].Byte[0];
        Command_send[idx_start_time_fill_mixfunction_2 + u8_time_fill_idx*2 +1]    = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[1].Byte[1];
    }
    // tu byte 200 den byte 247
    for(uint8_t u8_time_fill_idx = 0; u8_time_fill_idx < MAX_WELL_AMIDITE; u8_time_fill_idx++)
    {
        Command_send[idx_start_time_fill_mixfunction_3 + u8_time_fill_idx*2]       = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[2].Byte[0];
        Command_send[idx_start_time_fill_mixfunction_3 + u8_time_fill_idx*2 +1]    = global_var.amidite_well[u8_time_fill_idx].u16_timefill_well[2].Byte[1];
    }


    const quint16 idx_VacuumBox = 350; // 20byte tu 90 den 109
    if(isbaseSpecial((u16_counter_base_finished +1), m_baseVacuumBox.speacial_base))
    {
        int currentIdx = idx_VacuumBox; // Biến theo dõi chỉ số hiện tại
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.Enablefeature;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_WASH;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Deblock;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Coupling;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Ox;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Cap;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.time.Byte[0];
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.time.Byte[1];
        //currentIdx++;
    }else
    {
        int currentIdx = idx_VacuumBox; // Biến theo dõi chỉ số hiện tại
        Command_send[currentIdx]= false;
    }
    // Send main command
    if (!STM32_COM.serial_send_command_Firmware(serialPort, Command_send, SyntheticConfig::COMMAND_TIMEOUT_MS)) {
        qDebug() << "Failed to send non-coupling command";
        return SyntheticResult::FIRMWARE_ERROR;
    }

    // Continue with completeCouplingCommand for consistency (reuse existing logic)
    return completeCouplingCommand(template_sub_index, u8_counter_step, u8_first_chemical_temp, Command_send, u32_time_oligo_process_step);
}

/**
 * @brief Calculate process time for a specific step
 * Based on the logic from original code and completeCouplingCommand
 */
uint32_t SYNO24::calculateProcessTime(uint8_t u8_counter_sub_run, uint8_t u8_counter_step) {
    uint32_t u32_time_oligo_process_step = 0;

    // Add wait time after fill
    u32_time_oligo_process_step += protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_wait_after_fill.Data;

    // Add process and wait times for all pressure control steps (0-9)
    for (uint8_t idx_process = 0; idx_process < 10; idx_process++) {
        u32_time_oligo_process_step += protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_procs_time[idx_process].Data;
        u32_time_oligo_process_step += protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[idx_process].Data * SCALE_LEVEL_WAITTIME;
    }

    // Add command processing time
    u32_time_oligo_process_step += 100000; // 100 seconds for command processing

    // Get chemical type for special handling
    uint8_t u8_first_chemical_temp = get_FirstChemical(protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u8_first_type_chemical);

    // Special handling for oxidation chemicals
    if (u8_first_chemical_temp == OXIDATION_IODINE || u8_first_chemical_temp == OXIDATION_IODINE2) {
        u32_time_oligo_process_step = u32_time_oligo_process_step * SyntheticConfig::OXIDATION_TIME_MULTIPLIER; // Original comment: "coi lại chỗ này sao phải nhân cho 2.5 ?? quên comment rồi"
    }

    qDebug() << "Calculated process time for sub" << u8_counter_sub_run << "step" << u8_counter_step << ":" << u32_time_oligo_process_step << "ms";

    return u32_time_oligo_process_step;
}

/**
 * @brief Send oxidation sequence command
 */
SyntheticResult SYNO24::sendFinalSpecialOxidationSequenceupdate() {
    qDebug() << "Sending oxidation sequence for base" << global_var.signal_status_oligo.u16_counter_base_finished;
    uint16_t u16_counter_base_finished = global_var.signal_status_oligo.u16_counter_base_finished - 1; // trừ 1 vì base này đặc biệt nằm cuối cùng
    QByteArray Command_send(LENGTH_COMMAND_SEND, 0);
    Command_send[0] = CMD_OX_SENQUENCE;

    for (uint8_t u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++) {
        uint8_t linkage = global_var.amidite_well[u8_idx_well].linkages[u16_counter_base_finished];
        uint8_t sequence ;//= global_var.amidite_well[u8_idx_well].u8_sequence[global_var.signal_status_oligo.u16_counter_base_finished];
        // Fill linkages and sequences for all wells
        // Check if this is the last base of the well
        uint8_t sequence_val = global_var.amidite_well[u8_idx_well].lastSequence;
        if (isInCouplingLastBase(sequence_val)) {
            sequence = sequence_val;
            qDebug() << " Coupling base nay thuoc base dac biet nen = CHEMICAL_SUBTANCE_EMPTY : "<< u8_idx_well;
        }
        else
        {
            sequence = CHEMICAL_SUBTANCE_EMPTY;
        }
        Command_send[u8_idx_well + 1] = linkage;
        Command_send[u8_idx_well + 97] = sequence;

        if (linkage == 2) {
            qDebug() << "Well" << u8_idx_well << "OX2";
        }
    }

    // Add base counter
    //Command_send[150] = (global_var.signal_status_oligo.u16_counter_base_finished) & 0xFF;
    //Command_send[151] = (global_var.signal_status_oligo.u16_counter_base_finished >> 8) & 0xFF;

    if (!STM32_COM.serial_send_command_Firmware(serialPort, Command_send, SyntheticConfig::COMMAND_TIMEOUT_MS)) {
        qDebug() << "Failed to send oxidation sequence";
        return SyntheticResult::FIRMWARE_ERROR;
    }

    qDebug() << "Oxidation sequence sent successfully";
    return SyntheticResult::SUCCESS;
}


/**
 * @brief Process non-coupling step (other chemicals)
 */
SyntheticResult SYNO24::processFinalSpecialSyntheticNonCouplingStep(uint8_t u8_counter_sub_run, uint8_t u8_counter_step, uint8_t u8_first_chemical_temp) {
    qDebug() << "Processing non-coupling step - chemical type:" << u8_first_chemical_temp;
    // Define constants (exactly as original)
    const quint8 idx_start_opt_vaccum = 50;
    const quint8 idx_start_time_process = 60;
    const quint8 idx_start_time_wait = 80;
    const quint8 idx_start_time_fill_mixfunction_1 = 100;
    const quint8 idx_start_time_fill_mixfunction_2 = 150;
    const quint8 idx_start_time_fill_mixfunction_3 = 200;
    const quint16 idx_start_sequence_amidite = 290;
    QByteArray Command_send(LENGTH_COMMAND_SEND, 0);
    Command_send[0] = CMD_DATA_OLIGO;
    Command_send[1] = u8_first_chemical_temp;

    // Get volume for non-coupling chemical
    TwoByte_to_u16 u16tb_Timefill_Volume_first_type;
    u16tb_Timefill_Volume_first_type.Data = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_Volume.Data;

    Command_send[2] = u16tb_Timefill_Volume_first_type.Byte[0];
    Command_send[3] = u16tb_Timefill_Volume_first_type.Byte[1];

    qDebug() << "Non-coupling volume:" << u16tb_Timefill_Volume_first_type.Data;

    // Add common parameters
    Command_send[4] = protocol_oligo.sub[u8_counter_sub_run].douple_coupling_option;
    Command_send[14] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_wait_after_fill.Byte[0];
    Command_send[15] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].fill_chemical.u16tb_wait_after_fill.Byte[1];
    Command_send[16] = global_var.status_and_sensor.flag_enable_auto_control_air_Nito;

    // Handle special base
    uint16_t current_base_number = global_var.signal_status_oligo.u16_counter_base_finished;
    if (isbaseSpecial(current_base_number, protocol_oligo.speacial_base)) {
        Command_send[17] = true;
        Command_send[18] = protocol_oligo.u16_scale_volume.Byte[0];
        Command_send[19] = protocol_oligo.u16_scale_volume.Byte[1];
        Command_send[20] = protocol_oligo.u16_scale_time.Byte[0];
        Command_send[21] = protocol_oligo.u16_scale_time.Byte[1];
        qDebug() << "Special base detected:" << current_base_number;
    } else {
        Command_send[17] = false;
    }

    // Add step and auto-priming info
    Command_send[26] = u8_counter_step;
    Command_send[27] = global_var.advanced_setting.flag_auto_primming_chemical;
    Command_send[28] = global_var.advanced_setting.u16tb_autoPrim_volume_amidite.Byte[0];
    Command_send[29] = global_var.advanced_setting.u16tb_autoPrim_volume_amidite.Byte[1];
    Command_send[30] = global_var.advanced_setting.u16tb_autoPrim_volume_Activator.Byte[0];
    Command_send[31] = global_var.advanced_setting.u16tb_autoPrim_volume_Activator.Byte[1];

    // Calculate process time using centralized function
    uint32_t u32_time_oligo_process_step = calculateProcessTime(u8_counter_sub_run, u8_counter_step);

    // Add pressure control parameters
    for (uint8_t i = 0; i < 10; i++) {
//        Command_send[50 + i] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u8_option_pressure[i];

//        uint16_t process_time = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_procs_time[i].Data;
//        Command_send[60 + i * 2] = process_time & 0xFF;
//        Command_send[61 + i * 2] = (process_time >> 8) & 0xFF;

//        uint16_t wait_time = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[i].Data;
//        Command_send[80 + i * 2] = wait_time & 0xFF;
//        Command_send[81 + i * 2] = (wait_time >> 8) & 0xFF;

        // Note: Timeout calculation is now done by calculateProcessTime() function
        bool LowpushMul = ui->chkbx_LowPushMul->isChecked();
        bool HighpushMul = ui->chkbx_HighPushMul->isChecked();
        uint8_t option = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u8_option_pressure[i];

        Command_send[50 + i] = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u8_option_pressure[i];

        uint16_t process_time = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_procs_time[i].Data;


        uint16_t wait_time = protocol_oligo.sub[u8_counter_sub_run].step[u8_counter_step].control_pressure.u16tb_waitting_after_time[i].Data;
        if(option == LOW_PUSH && LowpushMul)
        {
            process_time = process_time * m_tableDataMultiplier.multiplier / 100;
            qDebug() << "LOW_PUSH multiplier applied:" << process_time;
        }
        else if(option == HIGH_PUSH && HighpushMul)
        {
            process_time = process_time * m_tableDataMultiplier.multiplier / 100;
            qDebug() << "HIGH_PUSH multiplier applied:" << process_time;
        }
        else
        {
            // Mặc định giữ nguyên giá trị
            //process_time = process_time;
            qDebug() << "No multiplier applied for option:" << option;
        }
        Command_send[60 + i * 2] = process_time & 0xFF;
        Command_send[61 + i * 2] = (process_time >> 8) & 0xFF;

        Command_send[80 + i * 2] = wait_time & 0xFF;
        Command_send[81 + i * 2] = (wait_time >> 8) & 0xFF;
    }
    quint16 u16_counter_base_finished = global_var.signal_status_oligo.u16_counter_base_finished -1;
    for (quint8 u8_idx_well = 0; u8_idx_well < MAX_WELL_AMIDITE; u8_idx_well++) {
        uint8_t sequence_val = global_var.amidite_well[u8_idx_well].u8_sequence[u16_counter_base_finished];
        if (isInCouplingLastBase(sequence_val)) {
            Command_send[idx_start_sequence_amidite + u8_idx_well] = sequence_val;
            qDebug() << " Coupling base nay thuoc base dac biet nen = CHEMICAL_SUBTANCE_EMPTY : "<< u8_idx_well;
        }
        else
        {
            Command_send[idx_start_sequence_amidite + u8_idx_well] = CHEMICAL_SUBTANCE_EMPTY;
        }
    }
    qDebug() << "Calculated step timeout:" << u32_time_oligo_process_step << "ms";
    const quint16 idx_VacuumBox = 350; // 20byte tu 90 den 109
    if(isbaseSpecial((u16_counter_base_finished +1), m_baseVacuumBox.speacial_base))
    {
        int currentIdx = idx_VacuumBox; // Biến theo dõi chỉ số hiện tại
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.Enablefeature;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_WASH;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Deblock;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Coupling;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Ox;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.En_Cap;
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.time.Byte[0];
        currentIdx++;
        Command_send[currentIdx]= global_var.advanced_setting.VacuumBox.time.Byte[1];
        //currentIdx++;
    }else
    {
        int currentIdx = idx_VacuumBox; // Biến theo dõi chỉ số hiện tại
        Command_send[currentIdx]= false;
    }
    // Send main command
    if (!STM32_COM.serial_send_command_Firmware(serialPort, Command_send, SyntheticConfig::COMMAND_TIMEOUT_MS)) {
        qDebug() << "Failed to send non-coupling command";
        return SyntheticResult::FIRMWARE_ERROR;
    }

    // Execute step
    QByteArray start_command(LENGTH_COMMAND_SEND, 0);
    start_command[0] = CMD_START_OLIGO_STEP;

    if (!STM32_COM.serial_send_command_Firmware(serialPort, start_command, u32_time_oligo_process_step)) {
        qDebug() << "Failed to execute non-coupling step";
        return SyntheticResult::FIRMWARE_ERROR;
    }

    log_terminal(" >> Completed", Success);
    qDebug() << "Non-coupling step executed successfully";

    return SyntheticResult::SUCCESS;
}

/**
 * @brief Recursive function to create blinking effect
 * @param label Target label
 * @param remainingBlinks Number of remaining blinks
 * @param originalStyleSheet Original stylesheet to restore
 */
void SYNO24::blinkLabel(QLabel* label, int remainingBlinks, const QString& originalStyleSheet) {
    if (!label || remainingBlinks <= 0) {
        // Restore original stylesheet when done
        if (label) {
            label->setStyleSheet(originalStyleSheet);
        }
        return;
    }

    // Toggle visibility
    if (remainingBlinks % 2 == 0) {
        // Even number = hide (make transparent)
        label->setStyleSheet("QLabel { color: transparent; }");
    } else {
        // Odd number = show with highlight
        label->setStyleSheet("QLabel { color: #FF0000; font-weight: bold; background-color: rgba(255, 255, 0, 100); }");
    }

    // Continue blinking after 300ms
    QTimer::singleShot(300, [this, label, remainingBlinks, originalStyleSheet]() {
        blinkLabel(label, remainingBlinks - 1, originalStyleSheet);
    });
}


/**
 * @brief Create pulsing effect for label
 * @param label Target label to pulse
 * @param text New text to set
 * @param color Color for text and border (default: "#FF4500" - orange)
 */
void SYNO24::pulseLabel(QLabel* label, const QString& text, const QString& color) {
    if (!label) return;

    // Set new text
    label->setText(text);

    // Create opacity effect
    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(label);
    label->setGraphicsEffect(opacityEffect);

    // Create pulsing animation
    QPropertyAnimation* pulseAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    pulseAnimation->setDuration(500);
    pulseAnimation->setStartValue(1.0);
    pulseAnimation->setKeyValueAt(0.5, 0.3);
    pulseAnimation->setEndValue(1.0);
    pulseAnimation->setEasingCurve(QEasingCurve::InOutSine);
    pulseAnimation->setLoopCount(3); // Pulse 3 times

    // Apply highlight style during pulse with custom color
    // Store original stylesheet or set default if empty

    QString originalStyleSheet = "QLabel { color: #000000; background-color: #FFFFFF; border: 1px solid #000000; border-radius: 2px; padding: 1px; }";

    QString pulseStyle = QString("QLabel { color: %1; font-weight: bold; border: 2px solid %1; border-radius: 3px; padding: 2px; }").arg(color);
    label->setStyleSheet(pulseStyle);

    // Restore original style after animation
    connect(pulseAnimation, &QAbstractAnimation::finished, [label, originalStyleSheet]() {
        label->setGraphicsEffect(nullptr);
        label->setStyleSheet(originalStyleSheet);
    });

    pulseAnimation->start(QAbstractAnimation::DeleteWhenStopped);
}

void SYNO24::on_chkbx_EnaSpecialLastBase_toggled(bool checked)
{
    Display_Protocol_to_user();
}


void SYNO24::on_btn_AddStateMultiplier_released()
{
    int fromValue = ui->spbx_formDoneOligo->value();
    int toValue = ui->spbx_ToDoneOligo->value();
    int multiplierValue = ui->spbx_multiplier->value();

    // Thêm dữ liệu vào struct
    m_tableDataMultiplier.rows.push_back({fromValue, toValue, multiplierValue});
    // Sử dụng vòng lặp for truyền thống với index
    for (size_t i = 0; i <  m_tableDataMultiplier.rows.size(); ++i) {
        TableRowData rowData = m_tableDataMultiplier.rows[i];
        qDebug()<< "Index: " << i
                << "From: " << rowData.from
                << ", To: " << rowData.to
                << ", Multiplier: " << rowData.multiplier;
    }
    // Cập nhật lại TableView
    updateTableMultiplierView();
}


void SYNO24::updateTableMultiplierView()
{
    if (!m_tableModel) {
        qDebug() << "[ERROR] Table model is null!";
        return;
    }

    m_tableModel->clear();
    m_tableModel->setHorizontalHeaderLabels({"From", "To", "Multiplier"});

    for (const auto& rowData : m_tableDataMultiplier.rows) {
        QList<QStandardItem*> items;

        // Tạo items với căn giữa
        QStandardItem* fromItem = new QStandardItem(QString::number(rowData.from));
        QStandardItem* toItem = new QStandardItem(QString::number(rowData.to));
        QStandardItem* multiplierItem = new QStandardItem(QString::number(rowData.multiplier));

        fromItem->setTextAlignment(Qt::AlignCenter);
        toItem->setTextAlignment(Qt::AlignCenter);
        multiplierItem->setTextAlignment(Qt::AlignCenter);

        items.append(fromItem);
        items.append(toItem);
        items.append(multiplierItem);

        m_tableModel->appendRow(items);
    }
}

void SYNO24::on_btn_DeleteMultiplier_released()
{
    QModelIndexList selectedIndexes = ui->tableView_2->selectionModel()->selectedRows();

    if (selectedIndexes.isEmpty()) {
        QMessageBox::warning(this, "Error", "Please select a row to delete.");
        return;
    }

    // Xóa các hàng đã chọn (duyệt từ dưới lên để tránh lỗi index)
    for (int i = selectedIndexes.count() - 1; i >= 0; --i) {
        int rowIndex = selectedIndexes.at(i).row();
        m_tableDataMultiplier.rows.erase(m_tableDataMultiplier.rows.begin() + rowIndex);
    }
    updateTableMultiplierView();
    for (size_t i = 0; i < m_tableDataMultiplier.rows.size(); ++i) {
        TableRowData rowData = m_tableDataMultiplier.rows[i];
        qDebug() << "Index: " << i
                 << "From: " << rowData.from
                 << ", To: " << rowData.to
                 << ", Multiplier: " << rowData.multiplier;
    }
}

// Hàm mới để tìm hệ số nhân dựa trên tỉ lệ phần trăm
int SYNO24::findMultiplierByPercentage(double percentage)
{
    for (const auto& rowData : m_tableDataMultiplier.rows) {
        if (percentage >= rowData.from && percentage <= rowData.to) {
            return rowData.multiplier;
        }
    }
    return 100; // Ví dụ: trả về 100 nếu không tìm thấy 100% tức là về nhân tỉ lệ
}

