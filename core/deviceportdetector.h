#ifndef DEVICEPORTDETECTOR_H
#define DEVICEPORTDETECTOR_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSignalMapper>
#include "../config.h"
#include "devicecomponent.h"
class DevicePortDetector : public DeviceComponent
{
    Q_OBJECT
private://fields
    QByteArray _device_name;
    quint32 _baud_rate;
    QByteArray _port;

    QList<QSerialPort*> _lookfor_available_ports;
    QByteArrayList _lookfor_available_ports_data;
    QSignalMapper _lookfor_ports_signals_mapper;
    int _lookfor_timer_id=0;
    int _send_m115_counter;
    bool _wait_for_send_m115,_wait_for_send_m29;
public:
    explicit DevicePortDetector(QByteArray deviceName,quint32 baudRate, class Device* device);
    void timerEvent(QTimerEvent *event) override;
    void StartDetect();

signals:
    void DetectPortFinished(QByteArray);

private slots:
    void DetectPortProcessing();
    void WhenEndDetectPort();
    void OnChecPortsDataAvailableMapped(int);





    // DeviceComponent interface
public:
    void Setup() override;
};

#endif // DEVICEPORTDETECTOR_H
