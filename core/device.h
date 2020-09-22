#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include "deviceinfo.h"
#include "QSerialPort"
#include "QSerialPortInfo"
#include "QThreadPool"
#include "QRunnable"
#include "QSignalMapper"
#include <QMutex>
#include <QMutexLocker>
#include <QReadWriteLock>
#include "devicefilessystem.h"
#include "gcodecommand.h"

class Device : public QObject
{
    Q_OBJECT

    friend class Devices;
public://nested types
    enum class DeviceStatus{
        Non,
        DetectDevicePort,
        PortDetected,
        Connected,
    };
    Q_ENUM(DeviceStatus);
    using Errors=QSerialPort::SerialPortError;
    Q_ENUMS(Errors);
private://nested types

    struct WriteTask:public QRunnable{
        Device* dev;
        QReadWriteLock _writ_safe_thread;
        bool _stopWrite;
        bool _write_task_is_busy=false;
        QByteArrayList _write_wait_list;
        void run() override;
        void WhenEndWriteTask(bool success);
        void StartWrite(QByteArrayList list);
        void StopWrite();
        bool IsBussy();

    };

private://fields
    QByteArray _port; 
    DeviceInfo* _device_info;
    QSerialPort* _serial_port;
    DeviceStatus _current_status;
    QMutex mutex;
    // read data
    QByteArrayList _availableLines;
    QByteArray _availableData;
    //write data
    WriteTask* _write_task;
    //detect ports by device name
    QList<QSerialPort*> _lookfor_available_ports;
    QByteArrayList _lookfor_available_ports_data;
    QSignalMapper _lookfor_ports_signals_mapper;
    int _lookfor_timer_id=0;
    //end end detect ports by device name
    QList<GCodeCommand*> _commands;
    DeviceFilesSystem* _fileSystem;


public:
    Q_INVOKABLE void DetectDevicePort();
    void SetPort(const QByteArray& port);
    QByteArray GetPort();
    void SetDeviceInfo(DeviceInfo* device);
    DeviceInfo* GetDeviceInfo();
    DeviceStatus GetStatus()const;
    bool IsOpen()const;
    bool OpenPort();
    void ClosePort();
    void Clear();
    void StopWrite();
    bool IsThereAvailableLines()const;
    void Write(QByteArray bytes);
    void Write(QByteArrayList bytes);
    QByteArray ReadLine();
    bool WriteIsBusy();
    void ClearLines();
    void AddGCodeCommand(GCodeCommand* command);
    void ClearCommands();
    GCodeCommand *GetCurrentCommand();
    DeviceFilesSystem *GetFileSystem()const;

    ~Device();


private://methods
    explicit Device(DeviceInfo* device_info,QObject *parent = nullptr);
    void timerEvent(QTimerEvent *event) override;
    void DetectPortProcessing();
    void SetStatus(DeviceStatus status);
    void WhenEndDetectPort();
    void CleanDetectPortsProcessing();
    void CalculateAndSetStatus();
    void SerialInputFilter(QByteArrayList& list);

private slots:
    void OnAvailableData();
    //void OnWrittenData();
    void OnChecPortsDataAvailableMapped(int);
    void OnErrorOccurred(QSerialPort::SerialPortError);
    void OnClosed();
    void WhenCommandFinished(bool);
    void CallFunction(const char* function);
    void StartNextCommand();



signals:
    void StatusChanged(DeviceStatus);
    void DetectPortSucceed();
    void DetectPortFailed();
    void PortOpened();
    void PortClosed();
    void ErrorOccurred(Errors);
    void NewLinesAvailable(QByteArrayList);
    void BytesWritten();
    void EndWrite(bool);
    void CommandFinished(GCodeCommand* , bool);
    void CommandStarted(GCodeCommand*);

};

#endif // DEVICE_H
