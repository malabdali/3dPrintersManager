#ifndef DEVICEACTIONS_H
#define DEVICEACTIONS_H

#include <QObject>
#include "devicecomponent.h"

class DeviceActions : public DeviceComponent
{
    Q_OBJECT
private://fields
    class Device* _device;
    bool _is_playing;
    QTimer* _reconnect_timer;
    QTimer* _save_device_data_timer;
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

signals:

};

#endif // DEVICEACTIONS_H
