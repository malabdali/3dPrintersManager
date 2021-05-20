#ifndef DeviceUDP_H
#define DeviceUDP_H

#include <QObject>
#include <QSerialPort>
#include <QMutex>
#include <QMutexLocker>
#include "deviceconnection.h"
#include <QUdpSocket>
#include <QProcess>

class DeviceUDP : public DeviceConnection
{
    Q_OBJECT
    bool _is_open;
    bool _want_to_open;
private://fields
    QUdpSocket* _udp;
    QProcess* _ping;
    QTimer* _check_ip_timer;
public:
    explicit DeviceUDP(class Device* device);
    Q_INVOKABLE void Open()override;
    bool IsOpen()override;
    Q_INVOKABLE void Close()override;
    void CheckIP();
    //Q_INVOKABLE void Reconnect()override;
    ~DeviceUDP()override;

private slots:
    void OnAvailableData();
    void OnErrorOccurred(int);
    void OnDataWritten(quint64);
    void OnPingFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void OnChekcIPTimeout();
private://methods



    // DeviceComponent interface
public:
    void Disable() override;

    // DeviceConnection interface
protected:
    void WriteFunction(const QByteArray &) override;

    // DeviceConnection interface
public:
    void Clear() override;
};

#endif // DEVICEPORT_H
