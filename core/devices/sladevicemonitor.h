#ifndef SLADEVICEMONITOR_H
#define SLADEVICEMONITOR_H

#include "../devicemonitor.h"
#include "../gcode/CHITU/chituprintingstats.h"
#include <chrono>

class SLADeviceMonitor:public DeviceMonitor
{
    Q_OBJECT
private: //fields

    GCode::Chitu::ChituPrintingStats* _printing_stats;
    std::chrono::time_point<std::chrono::steady_clock> _last_change_in_printing_progress;
public:
    SLADeviceMonitor(Device *dev);

    // DeviceMonitor interface
public:
    void Update() override;
    bool PrintingIsPaused();

protected:
    void timerEvent(QTimerEvent *event) override;
    void WhenDeviceStatsUpdated(GCodeCommand*);
    bool ReadTemperatureStats(QByteArray& ba);
    bool CommandReader(GCodeCommand*);
    void CheckPauseState();
protected slots:
    void WhenCommandFinished(GCodeCommand* , bool);
    void WhenDataLoaded();

    // DeviceComponent interface
public:
    void Setup() override;
    void Disable() override;
    void Play() override;
    void Pause()override;
};

#endif // SLADEVICEMONITOR_H
