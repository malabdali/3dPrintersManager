#ifndef FDMDEVICEMONITOR_H
#define FDMDEVICEMONITOR_H

#include "../devicemonitor.h"

class FDMDeviceMonitor:public DeviceMonitor
{
    Q_OBJECT
private: //fields

    class GCode::PrintingStats* _printing_stats;
    class GCode::ReportTemperature* _report_temprature;
    class GCode::EndstopsStates* _end_stops;
public:
    FDMDeviceMonitor(Device *dev);

    // DeviceMonitor interface
public:
    void Update() override;



protected:
    void timerEvent(QTimerEvent *event) override;
    void WhenDeviceStatsUpdated(GCodeCommand*);
    bool ReadTemperatureStats(QByteArray& ba);
    bool CommandReader(GCodeCommand*);
protected slots:
    void WhenCommandFinished(GCodeCommand* , bool);

    // DeviceComponent interface
public:
    void Setup() override;
    void Disable() override;
    void Play() override;
    void Pause()override;
};

#endif // FDMDEVICEMONITOR_H
