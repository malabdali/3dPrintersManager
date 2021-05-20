#ifndef DEVICEPORT_H
#define DEVICEPORT_H

#include <QObject>
#include <QSerialPort>
#include <QMutex>
#include <QMutexLocker>
#include "deviceconnection.h"

class DevicePort : public DeviceConnection
{
    Q_OBJECT
private://fields
    QSerialPort* _port;
public:
    explicit DevicePort(class Device* device);
    Q_INVOKABLE void Open()override;
    bool IsOpen()override;
    Q_INVOKABLE void Close()override;
    //Q_INVOKABLE void Reconnect()override;
    ~DevicePort()override;

private slots:
    void OnAvailableData();
    void OnErrorOccurred(int);
    void OnDataWritten(quint64);
private://methods



    // DeviceComponent interface
public:
    void Disable() override;

    // DeviceConnection interface
protected:
    void WriteFunction(const QByteArray &) override;
};

#endif // DEVICEPORT_H
