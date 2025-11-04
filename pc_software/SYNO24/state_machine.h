#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H
#include <QCoreApplication>
#include <QDebug>
#include <QPushButton>
#include "qglobal.h"
enum class Mode {
    AUTO_RUN,
    MANUAL_RUN
};

enum class State {
    RUNNING,
    STOPED,
    PAUSE
};

class state_machine
{
public:
    QPushButton* btn_Start_auto;
    QPushButton* btn_Stop_auto;
    QPushButton* btn_Pause_auto;
    QPushButton* btn_Start_PushDown_manual;
    QPushButton* btn_Start_FillChemical_manual;
    QPushButton* btn_Stop_manual;
    QPushButton* btn_Home;
    state_machine();
    void state_machine_init();
    void procsess_ui();
    bool setAutoMode();
    bool setManualMode();
    State setAutoState(State newstate);
    void setManualState(State newstatemanual);
    void printModeAndState()const;
    QString stateToString(State state) const;
    Mode getMode() const;
    State getAutoState() const;
    State getManualState() const;

    void setBtnStartAuto(QPushButton* btn_start_auto);
    void setBtnStopAuto(QPushButton* btn_stop_auto);
    void setBtnPauseAuto(QPushButton* btn_pause_auto);
    void setBtnStartPushDownManual(QPushButton* btn_start_manual);
    void setBtnStartFillChemicalManual(QPushButton* btn_start_fillchemical_manual);
    void setBtnstopManual(QPushButton* btn_stop_manual);
    void setBtnHomeManual(QPushButton* btn_home);
private:
    Mode mode;
    State auto_state;
    State manual_state;
};

#endif // STATE_MACHINE_H
