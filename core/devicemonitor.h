#ifndef DEVICEMONITOR_H
#define DEVICEMONITOR_H

#include <QObject>
#include "gcode/printingstats.h"
#include "gcode/reporttemperature.h"
#include "gcode/endstopsstates.h"
#include <chrono>
#include <QJsonDocument>
#include "devicecomponent.h"

class DeviceMonitor : public DeviceComponent
{
    Q_OBJECT
private://fields
    class GCode::PrintingStats* _printing_stats;
    class GCode::ReportTemperature* _report_temprature;
    class GCode::EndstopsStates* _end_stops;
    QMap<QByteArray,QByteArray> _data;
    bool _wait_device_stats;
    std::chrono::time_point<std::chrono::steady_clock> _last_update_during_busy;
public:
    explicit DeviceMonitor(Device *dev);
    void Setup() override;
    bool IsPrinting()const;
    bool IsBussy()const;
    bool IsWasPrinting()const;
    bool IsPaused()const;
    bool PrintingFinished()const;
    bool IsFilamentRunout()const;
    double GetPrintProgress()const;
    double GetHotendTemperature()const;
    double GetBedTemperature()const;
    QByteArray GetPrintingFile()const;

    void Reset();
    void Update();
    QJsonDocument ToJson() const;
    void FromJson(QJsonDocument json);
    void Save();
    void Load();
    void Pause();
    void Play();


private:
    void timerEvent(QTimerEvent *event) override;
    void WhenDeviceStatsUpdated(GCodeCommand*);
    bool ReadTemperatureStats(QByteArray& ba);
    bool CommandReader(GCodeCommand*);
private slots:
    void WhenCommandFinished(GCodeCommand* , bool);

signals:
    void updated();


    // DeviceComponent interface
};

#endif // DEVICEMONITOR_H
