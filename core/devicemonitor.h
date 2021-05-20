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
public://types

    enum MonitorOptions{
        PrintProgress=0x0001,
        HotendAndBedTemperature=0x0002,
        FilamentRunout=0x0004,
    };

protected://fields
    QMap<QByteArray,QByteArray> _data;
    bool _wait_device_stats;
    int _printing_stat_timer,_temperatures_timer,_endstops_timer,_monitor_timer;
    int _printing_stat_interval,_temperatures_interval,_endstops_interval;
    int _monitor_options;
    bool _is_baused;
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
    virtual void Update()=0;
    QJsonDocument ToJson() override;
    void FromJson(QJsonDocument json)override;
    void Save()override;
    void Load()override;
    virtual void Pause();
    virtual void Play();
    void SetMonitorOptions(int options);
    int GetMonitorOptions();



signals:
    void updated();


    // DeviceComponent interface

    // DeviceComponent interface
public:
    void Disable() override;
};

#endif // DEVICEMONITOR_H
