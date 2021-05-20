#ifndef SLAPRINTCONTROLLER_H
#define SLAPRINTCONTROLLER_H
#include "../printcontroller.h"
#include "QByteArray"
#include "../gcode/CHITU/chitustartstopprint.h"
#include "../gcode/CHITU/chituresumepauseprint.h"
#include "chrono"
#include <QDateTime>

class SLAPrintController:public PrintController
{
private://fields
    GCode::Chitu::ChituStartStopPrint* _start_stop_printing_command;
    GCode::Chitu::ChituResumePausePrint* _resume_pause_printing_command;
    QByteArray _file;
    uint32_t _printed_bytes,_total_bytes;
    QDateTime _start_printing_time,_finished_printing_time;
    int _timer_id;

public:
    SLAPrintController(Device* dev);

public:
    void Setup() override;
    void Disable() override;
    void StartPrint(QByteArray file) override;
    void ContinuePrint() override;
    void StopPrint() override;
    bool CanContinuePrinting() override;
    QJsonDocument ToJson() override;
    void FromJson(QJsonDocument json) override;
private:
    void SetCurrentStatus(Status);
    void WhenPrintingFinished();
    void timerEvent(QTimerEvent* event)override;
    void PrintUpdate();
    void StopUpdate();
    void PausePrint() override;
private slots:
    void AfterLoad();
    void WhenCommandFinished(GCodeCommand*,bool);
    void WhenMonitorUpdated();
    void WhenPortClosed();
    void WhenPortOpened();
};

#endif // SLAPRINTCONTROLLER_H
