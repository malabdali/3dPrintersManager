#ifndef DEVICEPORT_H
#define DEVICEPORT_H

#include <QObject>
#include <QSerialPort>
#include <QMutex>
#include <QMutexLocker>
#include "devicecomponent.h"

class DevicePort : public DeviceComponent
{
    Q_OBJECT
private://fields
    QByteArrayList _available_lines;
    QByteArray _available_data;
    bool _can_read;
    QSerialPort* _serial_port;
    QMutex _mutex;
    quint64 _writing_data_size;
    int _writing_timer;
    bool _reconnect;
public:
    explicit DevicePort(class Device* device);
    void Setup()override;
    Q_INVOKABLE void Write(QByteArray bytes);
    Q_INVOKABLE void Open(QByteArray port,quint64 baud_rate);
    QByteArray ReadLine();
    QList<QByteArray> ReadAllLines();
    uint32_t LinesCount();
    QByteArray PeakLine(int i);
    bool IsThereAvailableLines();
    void Clear();
    bool IsOpen();
    int GetError()const;
    QString GetTextError()const;
    Q_INVOKABLE void Close();
    Q_INVOKABLE void Reconnect();
    ~DevicePort();


signals:
    void NewLinesAvailable(QByteArrayList);
    void DataWritten(bool);
    void ErrorOccurred(int);
    void PortOpened(bool);
    void PortClosed();
    void Reconnected(bool);
private slots:
    void OnAvailableData();
    void OnErrorOccurred(QSerialPort::SerialPortError);
    void OnDataWritten(quint64);
    void InsideWrite(QByteArray);
    void timerEvent(QTimerEvent *event) override;
    void WhenDeviceRemoved();
private://methods
    void SerialInputFilter(QByteArrayList& list);
    void CallFunction(QByteArray function);
    void CallFunction(QByteArray function,QGenericArgument argument);
    void CallFunction(QByteArray function,QGenericArgument argument1,QGenericArgument argument2);



    // DeviceComponent interface
public:
    void Disable() override;
};

#endif // DEVICEPORT_H
