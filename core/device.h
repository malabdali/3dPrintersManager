#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include "QSerialPort"
#include "QSerialPortInfo"
#include "QThreadPool"
#include "QRunnable"
#include "QSignalMapper"
#include <QMutex>
#include <QMutexLocker>
#include <QReadWriteLock>
#include <QMap>
#include <chrono>
#include <QJsonObject>
#include <QJsonObject>
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
        Ready,
        Busy

    };
    Q_ENUM(DeviceStatus);
    using Errors=QSerialPort::SerialPortError;
    Q_ENUMS(Errors);

private://nested types

private://fields
    QByteArray _port; 
    QThread* _port_thread;
    DeviceStatus _current_status;
    bool _is_ready;
    bool _is_busy;
    QMap<QByteArray,QByteArray> _device_stats;
    //gcode commands
    QList<class GCodeCommand*> _commands;
    class GCodeCommand* _current_command;
    bool _commands_paused,_delay_command_state;
    std::chrono::time_point<std::chrono::steady_clock> _last_command_time_finished;
    QTimer* _delay_command_timer;
    //end gcode commands
    class DeviceFilesSystem* _fileSystem;
    class DevicePortDetector* _port_detector;
    class DevicePort* _device_port;
    class DeviceInfo* _device_info;
    class DeviceProblemSolver* _problem_solver;
    class DeviceMonitor* _device_monitor;


public:
    void DetectDevicePort();
    void SetPort(const QByteArray& port);
    QByteArray GetPort();
    void SetDeviceInfo(DeviceInfo* device);
    DeviceInfo *GetDeviceInfo();
    DeviceStatus GetStatus()const;
    bool IsOpen()const;
    void OpenPort();
    void ClosePort();
    void UpdateDeviceStats();
    void Clear();
    void Write(QByteArray bytes);
    DeviceFilesSystem *GetFileSystem()const;
    bool IsReady();
    QMap<QByteArray,QByteArray> GetStats()const;
    QJsonDocument GetStatsAsJSONObject()const;
    DevicePort* GetDevicePort();
    DeviceProblemSolver *GetProblemSolver()const;
    DeviceMonitor *GetDeviceMonitor();
    //commands
    void AddGCodeCommand(GCodeCommand* command);
    void ClearCommands();
    void ClearCommand(GCodeCommand* command);
    void PauseCommands();
    void PlayCommands();
    void StartCommand(GCodeCommand* command);
    bool CommandsIsPlayed();
    GCodeCommand *GetCurrentCommand();
    QList<GCodeCommand *> GetWaitingCommandsList()const;
    //end commands

    ~Device();


private://methods
    explicit Device(DeviceInfo* device_info,QObject *parent = nullptr);
    void SetStatus(DeviceStatus status);
    void CalculateAndSetStatus();
    void SerialInputFilter(QByteArrayList& list);
    void SetReady(bool ready);
    void DelayCommandCallback();

private slots:
    void OnErrorOccurred(int i);
    void OnClosed();
    void OnOpen(bool);
    void WhenCommandFinished(bool);
    void StartNextCommand();
    void OnDetectPort(QByteArray port="");
    void WhenStatsUpdated();


signals:
    void StatusChanged(DeviceStatus);
    void DetectPortSucceed();
    void DetectPortFailed();
    void PortOpened();
    void PortClosed();
    void ErrorOccurred(int);
    void BytesWritten();
    void EndWrite(bool);
    void CommandFinished(GCodeCommand* , bool);
    void CommandStarted(GCodeCommand*);
    void CommandAdded(GCodeCommand*);
    void CommandRemoved(GCodeCommand*);
    void ReadyFlagChanged(bool);
    void DeviceStatsUpdated(GCodeCommand*);
    void DeviceStatsUpdateFailed(GCodeCommand*);


};
#endif // DEVICE_H
