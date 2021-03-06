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
#include <QJsonDocument>
#include <QTimer>
#include <QNetworkReply>
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

private://nested types

protected://fields

    QByteArray _port; 
    QThread* _port_thread;
    DeviceStatus _current_status;
    bool _is_ready;
    bool _is_busy;
    bool _want_remove;
    bool _device_data_loaded;
    QMap<QByteArray,QByteArray> _device_stats;
    //gcode commands
    QList<class GCodeCommand*> _commands;
    class GCodeCommand* _current_command;
    bool _commands_paused,_delay_command_state;
    std::chrono::time_point<std::chrono::steady_clock> _last_command_time_finished;
    QTimer* _delay_command_timer;
    QNetworkReply* _network_reply;
    QJsonDocument _device_data;
    //end gcode commands
    class DeviceFilesSystem* _fileSystem;
    class DevicePortDetector* _port_detector;
    class DeviceConnection* _device_connection;
    class DeviceInfo* _device_info;
    class DeviceProblemSolver* _problem_solver;
    class DeviceMonitor* _device_monitor;
    class DeviceActions* _device_actions;
    class PrintController* _print_controller;
    class Camera* _camera;


public:
    void DetectDevicePort();
    void SetPort(const QByteArray& port);
    QByteArray GetPort();
    void SetDeviceInfo(DeviceInfo* device);
    DeviceInfo *GetDeviceInfo();
    DeviceStatus GetStatus()const;
    bool IsOpen()const;
    bool IsDeviceDataLoaded()const;
    void Open();
    void Close();
    virtual void UpdateDeviceStats()=0;
    void Clear();
    //void Write(QByteArray bytes);
    virtual DeviceFilesSystem *GetFileSystem()const;
    QMap<QByteArray,QByteArray> GetStats()const;
    DeviceConnection* GetDeviceConnection();
    DeviceProblemSolver *GetProblemSolver()const;
    virtual DeviceMonitor *GetDeviceMonitor();
    PrintController *GetPrintController();
    Camera *GetCamera();
    //json data
    void Load();
    void Save();
    void AddData(QByteArray name,QJsonObject);
    QJsonObject GetData(QByteArray name);
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


    virtual ~Device();


protected://methods
    explicit Device(DeviceInfo* device_info,QObject *parent = nullptr);
    virtual void Setup();
    void SetStatus(DeviceStatus status);
    void CalculateAndSetStatus();
    void SerialInputFilter(QByteArrayList& list);
    void SetFlags(bool ready=true,bool busy=false);
    void DelayCommandCallback();
    void Remove();
    void CompleteRemove();

protected slots:
    void OnErrorOccurred(int i);
    void OnClosed();
    void OnOpen(bool);
    void WhenCommandFinished(bool);
    void StartNextCommand();
    void OnDetectPort(QByteArray port="");
    virtual void WhenStatsUpdated()=0;


signals:
    void StatusChanged(DeviceStatus);
    void DetectPortSucceed();
    void DetectPortFailed();
    void Opened();
    void Closed();
    void ErrorOccurred(int);
    void BytesWritten();
    void EndWrite(bool);
    void CommandFinished(GCodeCommand* , bool);
    void CommandStarted(GCodeCommand*);
    void CommandAdded(GCodeCommand*);
    void CommandRemoved(GCodeCommand*);
    void DeviceStatsUpdated(GCodeCommand*);
    void DeviceStatsUpdateFailed(GCodeCommand*);
    void DeviceDataLoaded();
    void AfterDeviceDataLoaded();
    void BeforeSaveDeviceData();
    void DataSaved();
    void DeviceRemoved();


};
#endif // DEVICE_H
