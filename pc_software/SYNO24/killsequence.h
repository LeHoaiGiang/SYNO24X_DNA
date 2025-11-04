#ifndef KILLSEQUENCE_H
#define KILLSEQUENCE_H

#include <QDialog>
#include "QCheckBox"
namespace Ui {
class KillSequence;
}

class KillSequence : public QDialog
{
    Q_OBJECT

public:
    explicit KillSequence(QWidget *parent = nullptr);
    ~KillSequence();
    quint8 signalKill[24];
private slots:
    void on_btn_saveKill_released();
public slots:
   void setKillSequence(const quint8 values[]);
   void getKillSequence(quint8 values[]);
private:
    QCheckBox* chkboxWellKill[24];
    Ui::KillSequence *ui;
};

#endif // KILLSEQUENCE_H
