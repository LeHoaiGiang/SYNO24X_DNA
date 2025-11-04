#include "state_machine.h"
#define PRINT_DEBUG

state_machine::state_machine()
{
    //    mode = AUTO_RUN;
    //    auto_state = STOP;
    //    manual_state = STOP;
}

void state_machine::state_machine_init()
{
    mode = Mode::AUTO_RUN;
    auto_state = State::STOPED;
    manual_state = State::STOPED;
    btn_Start_auto->setDisabled(false);
    btn_Stop_auto->setDisabled(true);
    btn_Pause_auto->setDisabled(true);
    procsess_ui();
}
void state_machine:: procsess_ui()
{
    if(mode == Mode::AUTO_RUN)
    {
        if(auto_state == State::STOPED )
        {
            btn_Start_auto->setDisabled(false);
            btn_Stop_auto->setDisabled(true);
            btn_Pause_auto->setDisabled(true);
        }
        if(auto_state == State::PAUSE)
        {
            btn_Start_auto->setDisabled(true);
            btn_Stop_auto->setDisabled(false);
            btn_Pause_auto->setDisabled(false);
        }
        if( auto_state == State::RUNNING)
        {
            btn_Start_auto->setDisabled(true);
            btn_Stop_auto->setDisabled(false);
            btn_Pause_auto->setDisabled(false);
        }
    }
    if(mode == Mode::MANUAL_RUN)
    {
        if(manual_state == State::STOPED )
        {
#ifdef PRINT_DEBUG
            qDebug() << " ===== TRUOC khi set MANUAL_RUN STOPED";
            printModeAndState();

            //qDebug() << "OPEN ALL";
#endif
            btn_Start_FillChemical_manual->setEnabled(true);
            btn_Start_PushDown_manual->setEnabled(true);
            //btn_Home->setEnabled(true);
#ifdef PRINT_DEBUG
            qDebug() << " ===== SAU khi set MANUAL_RUN STOPED";
            printModeAndState();
            //qDebug() << "OPEN ALL";
#endif
        }
        if(manual_state == State::RUNNING)
        {
            qDebug() << " ===== TRUOC khi set MANUAL_RUN  RUNNING";
            printModeAndState();
            btn_Start_FillChemical_manual->setEnabled(false);
            btn_Start_PushDown_manual->setEnabled(false);
            qDebug() << " ===== SAU khi set MANUAL_RUN RUNNING";
            printModeAndState();
            //btn_Home->setEnabled(false);
            //qDebug() << "DISABLE ALL";
        }
    }
}


bool state_machine :: setAutoMode()
{
    if(mode == Mode::MANUAL_RUN && manual_state == State::STOPED)
    {
        mode = Mode::AUTO_RUN;
#ifdef PRINT_DEBUG
        printModeAndState();
#endif
        return true;
    }
    else
    {
        return false;
    }
    procsess_ui();
}

bool state_machine ::setManualMode()
{
    if(mode == Mode::AUTO_RUN && auto_state == State::STOPED)
    {
        mode = Mode::MANUAL_RUN;
#ifdef PRINT_DEBUG
        printModeAndState();
#endif
        procsess_ui();
        return true;
    }
    else
    {
        return false;
    }
    procsess_ui();
}

State state_machine:: setAutoState(State newstate)
{
    if(newstate == State::RUNNING && (auto_state == State::STOPED || auto_state == State::PAUSE))
    {
        auto_state = State::RUNNING;
        procsess_ui();
    }
    if(newstate == State::PAUSE && auto_state == State::RUNNING)
    {
        auto_state = State::PAUSE;
        procsess_ui();
    }
    if(newstate == State::STOPED)
    {
        auto_state = State::STOPED;
        btn_Pause_auto->setDisabled(true);
    }
#ifdef PRINT_DEBUG

    printModeAndState();
#endif
    //procsess_ui();
    return auto_state;
}

void state_machine:: setManualState(State newstatemanual)
{
    manual_state = newstatemanual;
    procsess_ui();
#ifdef PRINT_DEBUG
    qDebug() << "setManualState" << (quint8)newstatemanual;
    printModeAndState();
#endif
}

Mode state_machine::getMode() const {
    return mode;
}

State state_machine::getAutoState() const {
    return auto_state;
}

State state_machine::getManualState() const {
    return manual_state;
}

void state_machine::printModeAndState() const {
    qDebug() << "Mode: " << (mode == Mode::AUTO_RUN ? "AutoRun" : "ManualRun");
    qDebug() << "Auto State: " << stateToString(auto_state);
    qDebug() << "Manual State: " << stateToString(manual_state);
}

QString state_machine::stateToString(State state) const {
    switch (state) {
    case State::RUNNING:
        return "RUNNING";
    case State::STOPED:
        return "STOP";
    case State::PAUSE:
        return "PAUSE";
    default:
        return "UNKNOWN";
    }
}
//============================================================ PROCESS UI =================================================================

void state_machine::setBtnStartAuto(QPushButton* btn_start_auto)
{
    btn_Start_auto = btn_start_auto;
}
void state_machine::setBtnStopAuto(QPushButton* btn_stop_auto)
{
    btn_Stop_auto = btn_stop_auto;
}
void state_machine::setBtnPauseAuto(QPushButton* btn_pause_auto)
{
    btn_Pause_auto = btn_pause_auto;
}
void state_machine::setBtnStartPushDownManual(QPushButton* btn_start_pushdown_manual)
{
    btn_Start_PushDown_manual = btn_start_pushdown_manual;
}
void state_machine::setBtnStartFillChemicalManual(QPushButton* btn_start_fillchemical_manual)
{
    btn_Start_FillChemical_manual = btn_start_fillchemical_manual;
}
void state_machine::setBtnstopManual(QPushButton* btn_stop_manual)
{
    btn_Stop_manual = btn_stop_manual;
}


void state_machine::setBtnHomeManual(QPushButton* btn_home)
{
    btn_Home = btn_home;
}
