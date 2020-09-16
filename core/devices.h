#ifndef DEVICES_H
#define DEVICES_H

#include <QObject>
#include "device.h"
#include <algorithm>
#include <QSignalMapper>
#include <QApplication>
#include <QThread>

class Devices : public QObject
{
    Q_OBJECT
private://fields
    QList<Device*> _detect_ports_wait_list;
    QList<Device*> _devices;
    static Devices* _INSTANCE;


public:
    Device* AddDevice(DeviceInfo& dev);
    void RemoveDevice(const DeviceInfo &name);
    Device *GetDevice(const DeviceInfo &name)const;
    Device* GetDeviceByPort(const QByteArray& port)const;
    void DetectPortAndConnectForAllDevices(bool=false);
private:
    explicit Devices(QObject *parent = nullptr);
    void WhenEndDevicesPortDetection();
public://static methods
    static Devices* GetInstance();


    ~Devices();

private slots:
    void OnDevicePortDetected();
signals:
    void DetectPortsEnded();

};

#endif // DEVICES_H
