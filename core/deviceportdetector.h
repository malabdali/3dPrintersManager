#ifndef DEVICEPORTDETECTOR_H
#define DEVICEPORTDETECTOR_H

#include <QObject>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QSignalMapper>
#include "../config.h"

class DevicePortDetector : public QObject
{
    Q_OBJECT
private://fields
    QByteArray _device_name;
    size_t _baud_rate;
    QByteArray _port;

    QList<QSerialPort*> _lookfor_available_ports;
    QByteArrayList _lookfor_available_ports_data;
    QSignalMapper _lookfor_ports_signals_mapper;
    int _lookfor_timer_id=0;
    int _send_m115_counter;
    bool _wait_for_send_m115,_wait_for_send_m29;
public:
    explicit DevicePortDetector(QByteArray deviceName,size_t baudRate, QObject *parent = nullptr);
    void timerEvent(QTimerEvent *event) override;
    void StartDetect();

signals:
    void DetectPortFinished(QByteArray);

private slots:
    void DetectPortProcessing();
    void WhenEndDetectPort();
    void OnChecPortsDataAvailableMapped(int);




};

#endif // DEVICEPORTDETECTOR_H