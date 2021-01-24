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
    int _printing_stat_timer,_temperatures_timer,_endstops_timer;
    int _printing_stat_interval,_temperatures_interval,_endstops_interval;
    std::chrono::time_point<std::chrono::steady_clock> _last_update_during_busy;
public:
    explicit DeviceMonitor(Device *dev);
    void Setup() override;
    bool IsPrinting()const;
    bool IsBusy()const;
    bool IsWasPrinting()const;
    bool IsPaused()const;
    bool PrintingFinished()const;
    bool IsFilamentRunout()const;
    double GetPrintProgress()const;
    double GetHotendTemperature()const;
    double GetBedTemperature()const;
    uint GetPrintedBytes()const;
    uint GetTotalBytes()const;
    QByteArray GetPrintingFile()const;
    void SetUpdateIntervals(uint printingStats,uint temperatures,uint endStops);
    void ResetIntervals();
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
