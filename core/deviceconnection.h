#ifndef DEVICECONNECTION_H
#define DEVICECONNECTION_H


#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include "devicecomponent.h"

class DeviceConnection: public DeviceComponent
{

    Q_OBJECT
protected://fields
    QByteArrayList _available_lines;
    QByteArray _available_data;
    QByteArray _error_text;
    int _error;
    bool _can_read;
    QMutex _mutex;
    quint64 _writing_data_size;
    int _writing_timer;
    bool _is_was_open;

public:
    explicit DeviceConnection(class Device* device);
    virtual void Setup()override;
    virtual void Write(QByteArray bytes);
    virtual Q_INVOKABLE void Open()=0;
    virtual QByteArray ReadLine();
    virtual QList<QByteArray> ReadAllLines();
    virtual uint32_t LinesCount();
    virtual QByteArray PeakLine(int i);
    virtual bool IsThereAvailableLines();
    virtual void Clear();
    virtual bool IsOpen()=0;
    virtual int GetError();
    virtual QByteArray GetErrorText();
    virtual Q_INVOKABLE void Close()=0;
    //virtual Q_INVOKABLE void Reconnect()=0;
    virtual ~DeviceConnection();


signals:
    void NewLinesAvailable(QByteArrayList);
    void DataWritten(bool);
    void ErrorOccurred(int);
    void Opened(bool);
    void Closed();
private slots:
    void timerEvent(QTimerEvent *event) override;
    void WhenDeviceRemoved();
protected://methods
    virtual Q_INVOKABLE void InsideWrite(QByteArray);
    virtual void WriteFunction(const QByteArray&)=0;
    virtual void InputFilter(QByteArrayList& list);
    void CallFunction(QByteArray function);
    void CallFunction(QByteArray function,QGenericArgument argument);
    void CallFunction(QByteArray function,QGenericArgument argument1,QGenericArgument argument2);



    // DeviceComponent interface
public:
    void Disable() override;


    // DeviceComponent interface
public:
    QJsonDocument ToJson() override;
    void FromJson(QJsonDocument json) override;
    void Save() override;
    void Load() override;
};

#endif // DEVICECONNECTION_H
