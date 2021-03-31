#ifndef DEVICEACTIONS_H
#define DEVICEACTIONS_H

#include <QObject>
#include "devicecomponent.h"
#include <QTimer>
#include "device.h"

class DeviceActions : public DeviceComponent
{
    Q_OBJECT
private://fields
    class Device* _device;
    bool _is_playing,_device_data_loaded;
    QTimer *_reconnect_timer, *_save_device_data_timer, *_stop_timer, *_sd_recheck_timer;
public:
    explicit DeviceActions(Device *device);
    void Setup();
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
    void WhenDeviceReady();

signals:

};

#endif // DEVICEACTIONS_H
