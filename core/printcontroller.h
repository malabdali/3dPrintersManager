#ifndef PRINTCONTROLLER_H
#define PRINTCONTROLLER_H

#include "./devicecomponent.h"
#include <QDebug>
#include "QByteArray"
#include "./gcode/startprinting.h"
#include "./gcode/stopsdprint.h"
#include "./gcode/settemperatures.h"
#include "./gcode/preprint.h"
#include "chrono"
#include <QDateTime>
class PrintController:public DeviceComponent
{
    Q_OBJECT

public://types
enum Status:int{
    Nothing=0,
    SendHeatUpCommands=1,
    HeatUp=2,
    SendPrintCommand=3,
    Printing=4,
    SendStopCommand=5,
    SendHeatOffCommand=6,
    Stopped=7,
    PreprintPrepare=8
};

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
Status _current_status,_wanted_status;
int _current_line;
bool _continue_print;
QByteArray _file_content;
int _timer_id;
QDateTime _start_printing_time,_finished_printing_time;
double _bed_temperature,_hotend_temperature;
public:
    explicit PrintController(class Device*);
    void StartPrint(QByteArray file);
    void ContinuePrint();
    void StopPrint();
    Status GetWantedStatus();
    Status GetCurrentStatus();
    uint GetWantedBedTemperature();
    uint GetWantedHotendTemperature();
    uint GetLastLayer(int index);
    double GetLastEValue(int index);
    int GetLastFanSpeed(int index);
    QByteArray GetLastAcceleration(int index);
    QByteArray GetLastJerk(int index);
    QByteArray GetMaxFeedRate(int index);
    QByteArray GetMaxAcceleration(int index);
    bool CanContinuePrinting();
    bool IsPrinting();
public:
    void Setup() override;
signals:
    void PrintStarted();
    void PrintStopped();
    void WantedStatusChanged();
    void StatusChanged();
    void PrintingFinished();

private://methods
    QByteArrayList LookForLines(int from,int to,QByteArray command="");
    QByteArray LookForLastLine(int from,int to,QByteArray command);
    QByteArray LookForFirstLine(int from,int to,QByteArray command);
    void CalculateWantedTempratures(int line);
    void timerEvent(QTimerEvent* event)override;
    void PrintUpdate();
    void StopUpdate();
    void SetCurrentStatus(Status status);
    QJsonDocument ToJson() const;
    void FromJson(QJsonDocument json);
    void Save();
    void Load();
    void AfterLoad();
    void WhenPrintingFinished();
private slots:
    void WhenCommandFinished(GCodeCommand* command,bool success);
    void WhenMonitorUpdated();
    void WhenPortClosed();
    void WhenPortOpened();

};

#endif // PRINTCONTROLLER_H
