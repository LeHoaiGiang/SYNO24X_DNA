#include "killsequence.h"
#include "ui_killsequence.h"
#include "qdebug.h"
#include "macro.h"
KillSequence::KillSequence(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::KillSequence)
{

    ui->setupUi(this);
    this->setWindowTitle("Kill Sequence");
    chkboxWellKill[0] = ui->well_1;
    chkboxWellKill[1] = ui->well_2;
    chkboxWellKill[2] = ui->well_3;
    chkboxWellKill[3] = ui->well_4;
    chkboxWellKill[4] = ui->well_5;
    chkboxWellKill[5] = ui->well_6;
    chkboxWellKill[6] = ui->well_7;
    chkboxWellKill[7] = ui->well_8;
    chkboxWellKill[8] = ui->well_9;
    chkboxWellKill[9] = ui->well_10;
    chkboxWellKill[10] = ui->well_11;
    chkboxWellKill[11] = ui->well_12;
    chkboxWellKill[12] = ui->well_13;
    chkboxWellKill[13] = ui->well_14;
    chkboxWellKill[14] = ui->well_15;
    chkboxWellKill[15] = ui->well_16;
    chkboxWellKill[16] = ui->well_17;
    chkboxWellKill[17] = ui->well_18;
    chkboxWellKill[18] = ui->well_19;
    chkboxWellKill[19] = ui->well_20;
    chkboxWellKill[20] = ui->well_21;
    chkboxWellKill[21] = ui->well_22;
    chkboxWellKill[22] = ui->well_23;
    chkboxWellKill[23] = ui->well_24;
}

KillSequence::~KillSequence()
{
    delete ui;
}

void KillSequence::on_btn_saveKill_released()
{
    for(int i =0; i < MAX_WELL_AMIDITE; i++)
    {
        signalKill[i] = chkboxWellKill[i]->isChecked();
    }
    accept();                             // Đóng cửa sổ
}


// Hàm để set giá trị cho các checkbox từ mảng quint8_t
void KillSequence::  setKillSequence(const quint8 values[]) {
    for (int i = 0; i < MAX_WELL_AMIDITE; ++i) {
        if (chkboxWellKill[i]) { // Kiểm tra xem checkbox có tồn tại không
            chkboxWellKill[i]->setChecked(values[i] != 0); // Set giá trị true/false
        } else {
         //   qWarning() << "Checkbox at index" << i << "is null!";
        }
    }
}

// Hàm để set giá trị cho các checkbox từ mảng quint8_t
void KillSequence::  getKillSequence(quint8 values[]) {
    for (int i = 0; i < MAX_WELL_AMIDITE; ++i) {
        if (chkboxWellKill[i]) { // Kiểm tra xem checkbox có tồn tại không
            //chkboxWellKill[i]->setChecked(values[i] != 0); // Set giá trị true/false
            values[i] = chkboxWellKill[i]->isChecked();
        } else {
         //   qWarning() << "Checkbox at index" << i << "is null!";
        }
    }
}
