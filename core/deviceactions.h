#ifndef DEVICEACTIONS_H
#define DEVICEACTIONS_H

#include <QObject>
#include "devicecomponent.h"
#include <QTimer>
#include "device.h"
#include "printcontroller.h"
#include "./gcode/settemperatures.h"

class DeviceActions : public DeviceComponent
{
    Q_OBJECT
private://fields
    class Device* _device;
    bool _is_playing,_device_data_loaded;
    QTimer *_reconnect_timer, *_save_device_data_timer, *_stop_timer, *_sd_recheck_timer;
    GCode::SetTemperatures* _set_temperatures_command;
public:
    explicit DeviceActions(Device *device);
    void Setup()override;
    void Play();
    void Pause();
    bool IsPlaying();
private slots:
    void WhenMonitorUpdated();
    void WhenCommandFinished(class GCodeCommand* command,bool success);
    void WhenProblemDetected();
    void WhenSolveProblemFinished();
    void SolveReconnectProblem();
    void SaveData();
    void RecheckSDSUpport();
    void WhenDeviceLoaded();
    void StopPrinting();
    void WhenPrintStatusChanged();
    void WhenDeviceRemoved();
    void WhenDeviceStatusChanged();
    void AfterDataLoaded();

signals:


    // DeviceComponent interface
public:
    void Disable() override;

    // DeviceComponent interface
public:
    QJsonDocument ToJson() override;
    void FromJson(QJsonDocument json) override;
    void Save() override;
    void Load() override;
};

#endif // DEVICEACTIONS_H
