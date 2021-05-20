#ifndef FDMPRINTCONTROLLER_H
#define FDMPRINTCONTROLLER_H

#include "../printcontroller.h"
#include "QByteArray"
#include "../gcode/startprinting.h"
#include "../gcode/stopsdprint.h"
#include "../gcode/settemperatures.h"
#include "../gcode/preprint.h"
#include "chrono"
#include <QDateTime>
class FDMPrintController:public PrintController
{
private://fields
    GCode::StartPrinting* _start_printing_command;
    GCode::StopSDPrint* _stop_printing_command;
    GCode::SetTemperatures* _set_temperatures_command;
    GCode::PrePrint* _preprint_command;
    QByteArray _file;
    uint32_t _printed_bytes,_total_bytes;
    uint _wanted_bed_temprature,_wanted_hottend_temperature,_wanted_fan_speed;
    QByteArray _wanted_acceleration,_wanted_jerk,_max_feedrate,_max_acceleration;
    double _wanted_extruder;
    int _current_line;
    bool _continue_print;
    bool _can_heatup;
    QByteArray _file_content;
    int _timer_id;
    QDateTime _start_printing_time,_finished_printing_time;
    double _bed_temperature,_hotend_temperature;
public:
    FDMPrintController(Device* dev);

    // DeviceComponent interface
public:
    void Setup() override;
    void Disable() override;

    // PrintController interface
public:
    void StartPrint(QByteArray file) override;
    void ContinuePrint() override;
    void StopPrint() override;
    void PausePrint() override;
    bool IsPrinting() override;
    QJsonDocument ToJson()  override;
    void FromJson(QJsonDocument json) override;
    uint GetWantedBedTemperature();
    uint GetWantedHotendTemperature();
    uint GetLastLayer(int index);
    double GetLastEValue(int index);
    int GetLastFanSpeed(int index);
    QByteArray GetLastAcceleration(int index);
    QByteArray GetLastJerk(int index);
    QByteArray GetMaxFeedRate(int index);
    QByteArray GetMaxAcceleration(int index);
    bool CanContinuePrinting() override;
    void SetHeatupAbility(bool b);
    bool GetHeatupAbility()const;

private://methods
    QByteArrayList LookForLines(int from,int to,QByteArray command="");
    QByteArray LookForLastLine(int from,int to,QByteArray command);
    QByteArray LookForFirstLine(int from,int to,QByteArray command);
    void CalculateWantedTempratures(int line);
    void timerEvent(QTimerEvent* event)override;
    void PrintUpdate();
    void StopUpdate();
    void SetCurrentStatus(Status status);
    void WhenPrintingFinished();
    void AfterLoad();
private slots:
    void WhenCommandFinished(GCodeCommand* command,bool success);
    void WhenMonitorUpdated();
    void WhenPortClosed();
    void WhenPortOpened();

    // PrintController interface
};

#endif // FDMPRINTCONTROLLER_H
