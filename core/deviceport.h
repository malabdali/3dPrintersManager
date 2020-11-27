#ifndef DEVICEPORT_H
#define DEVICEPORT_H

#include <QObject>
#include <QSerialPort>
#include <QMutex>
#include <QMutexLocker>

class DevicePort : public QObject
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
public:
    explicit DevicePort(QObject* object=nullptr);
    Q_INVOKABLE void Write(QByteArray bytes);
    Q_INVOKABLE void Open(QByteArray port,quint64 baud_rate);
    QByteArray ReadLine();
    bool IsThereAvailableLines();
    void Clear();
    bool IsOpen();
    Q_INVOKABLE void Close();
    ~DevicePort();


signals:
    void NewLinesAvailable(QByteArrayList);
    void DataWritten(bool);
    void ErrorOccurred(int);
    void PortOpened(bool);
    void PortClosed();
private slots:
    void OnAvailableData();
    void OnErrorOccurred(QSerialPort::SerialPortError);
    void OnDataWritten(quint64);
    void InsideWrite(QByteArray);
    void timerEvent(QTimerEvent *event) override;
private://methods
    void SerialInputFilter(QByteArrayList& list);
    void CallFunction(QByteArray function);
    void CallFunction(QByteArray function,QGenericArgument argument);
    void CallFunction(QByteArray function,QGenericArgument argument1,QGenericArgument argument2);


};

#endif // DEVICEPORT_H
