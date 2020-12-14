#ifndef DEVICES_H
#define DEVICES_H

#include <QObject>
#include "device.h"
#include <algorithm>
#include <QSignalMapper>
#include <QApplication>
#include <QThread>
#include "remoteserver.h"
#include <QJsonArray>

class Devices : public QObject
{
    Q_OBJECT
private://fields
    QList<Device*> _detect_ports_wait_list;
    QList<Device*> _devices;
    static Devices* _INSTANCE;
    QByteArray _network_id;

public:
    Device* AddDevice(DeviceInfo* dev);
    void RemoveDevice(const DeviceInfo *name);
    Device *GetDevice(const DeviceInfo *name)const;
    Device *GetDevice(const QByteArray &name)const;
    Device *GetDeviceByPort(const QByteArray& port)const;
    void DetectPortAndConnectForAllDevices(bool=false);
    void LoadDevicesFromRemoteServer();
    QList<Device *> GetAllDevices();
    QList<QByteArray> GetAllDevicesPort();
    QByteArray GetNetworkID()const;
    void SetNetworkID(const QByteArray&);
    void Clear();
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
    void DevicesLoaded(bool);
    void DeviceAdded(Device*);
    void DeviceRemoved(Device*);

};

#endif // DEVICES_H
