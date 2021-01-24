#ifndef PRINTCONTROLLER_H
#define PRINTCONTROLLER_H

#include "./devicecomponent.h"
#include <QDebug>
#include "QByteArray"
#include "./gcode/startprinting.h"
#include "./gcode/stopsdprint.h"
#include "./gcode/settemperatures.h"
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
    Stopped=7
};

private://fields
GCode::StartPrinting* _start_printing_command;
GCode::StopSDPrint* _stop_printing_command;
GCode::SetTemperatures* _set_temperatures_command;
QByteArray _file;
uint32_t _printed_bytes,_total_bytes;
uint _wanted_bed_temprature,_wanted_hottend_temperature;
Status _current_status,_wanted_status;
int _current_line;
QByteArrayList _file_lines;
int _timer_id;

public:
    explicit PrintController(class Device*);
    void StartPrint(QByteArray file);
    void ContinuePrint();
    void StopPrint();
    void CalculateWantedTempratures(int line);
    Status GetWantedStatus();
    Status GetCurrentStatus();
    uint GetWantedBedTemperature();
    uint GetWantedHotendTemperature();
    bool CanContinuePrinting();
    bool IsPrinting();
public:
    void Setup() override;
signals:
    void PrintStarted();
    void PrintStopped();
    void WantedStatusChanged();
    void StatusChanged();

private://methods
    QByteArrayList LookForLines(int from,int to,QByteArray command="");
    QByteArray LookForLastLine(int from,int to,QByteArray command);
    QByteArray LookForFirstLine(int from,int to,QByteArray command);
    void timerEvent(QTimerEvent* event)override;
    void PrintUpdate();
    void StopUpdate();
    void SetCurrentStatus(Status status);
    QJsonDocument ToJson() const;
    void FromJson(QJsonDocument json);
    void Save();
    void Load();
    void AfterLoad();
private slots:
    void WhenCommandFinished(GCodeCommand* command,bool success);
    void WhenMonitorUpdated();
    void WhenPortClosed();
    void WhenPortOpened();

};

#endif // PRINTCONTROLLER_H
