#include "device.h"
#include<functional>
#include <QDebug>
#include <algorithm>
#include "../config.h"
#include <QMetaMethod>
#include "devicefilessystem.h"
#include "gcodecommand.h"
#include "deviceportdetector.h"
#include "gcode/devicestats.h"
#include "deviceport.h"
#include "deviceproblemsolver.h"
#include "deviceinfo.h"
#include <QTimer>
#include <QDir>
#include "devicemonitor.h"
#include "deviceactions.h"
#include "remoteserver.h"
#include "printcontroller.h"
#include "camera.h"

Device::Device(DeviceInfo* device_info,QObject *parent) : QObject(parent),_port(""),_port_thread(new QThread()),
    _fileSystem(new DeviceFilesSystem(this)),_device_port(new DevicePort(this)),_device_info(device_info),_problem_solver(new DeviceProblemSolver(this)),
    _device_monitor(new DeviceMonitor(this)),_device_actions(new DeviceActions(this)),_print_controller(new PrintController(this)),_camera(new Camera(this))
{
    _port_detector=nullptr;
    qRegisterMetaType<Errors>();
    device_info->setParent(this);
    _is_ready=false;
    _is_busy=false;
    _device_data_loaded=false;
    _want_remove=false;
    _commands_paused=false;
    _current_command=nullptr;
    _network_reply=nullptr;
    _port_thread->start();
    _device_port->setParent(nullptr);
    _device_port->moveToThread(_port_thread);
    _last_command_time_finished=std::chrono::steady_clock::now();
    _fileSystem->Initiate();
    _delay_command_state=false;
    _delay_command_timer=new QTimer(this);
    this->connect(_delay_command_timer,&QTimer::timeout,this,&Device::DelayCommandCallback);
    _device_actions->Setup();
    _device_monitor->Setup();
    _fileSystem->Setup();
    _device_port->Setup();
    _problem_solver->Setup();
    _print_controller->Setup();
    _camera->Setup();

    QObject::connect(_device_port,&DevicePort::ErrorOccurred,this,&Device::OnErrorOccurred);
    QObject::connect(_device_port,&DevicePort::PortClosed,this,&Device::OnClosed);
    QObject::connect(_device_port,&DevicePort::PortOpened,this,&Device::OnOpen);
    QObject::connect(_device_port,&DevicePort::Reconnected,this,&Device::OnOpen);
    QObject::connect(_port_thread,&QThread::finished,this,&Device::CompleteRemove);


}

void Device::DetectDevicePort()
{
    _port="";
    _port_detector=new DevicePortDetector(_device_info->GetDeviceName(),_device_info->GetBaudRate(),this);
    QObject::connect(_port_detector,&DevicePortDetector::DetectPortFinished,this,&Device::OnDetectPort);
    _port_detector->StartDetect();
    CalculateAndSetStatus();
}

QByteArray Device::GetPort(){
    return this->_port;
}

void Device::SetPort(const QByteArray &port)
{
    _port=port;
}

void Device::Device::SetDeviceInfo(DeviceInfo* device)
{
    _device_info=device;
}

DeviceInfo *Device::GetDeviceInfo(){
    return this->_device_info;
}

void Device::SetStatus(Device::DeviceStatus status){
    DeviceStatus old=_current_status;
    _current_status=status;
    if(status!=old)
        emit StatusChanged(status);
}

void Device::CalculateAndSetStatus()
{
    DeviceStatus ds=DeviceStatus::Non;
    if(_port_detector!=nullptr)
        ds=DeviceStatus::DetectDevicePort;
    else if(_device_port->IsOpen() && !_is_ready && !_is_busy)
        ds=DeviceStatus::Connected;
    else if(!_port.isEmpty() && !_device_port->IsOpen())
        ds=DeviceStatus::PortDetected;
    else if(_device_port->IsOpen() && _is_ready)
        ds=DeviceStatus::Ready;
    else if(_device_port->IsOpen() && _is_busy)
        ds=DeviceStatus::Busy;

    SetStatus(ds);
}

void Device::SetFlags(bool ready, bool busy)
{
    _is_ready=ready;
    _is_busy=busy;
    CalculateAndSetStatus();
    if(_is_ready && !_is_busy)
        StartNextCommand();
}

void Device::DelayCommandCallback()
{
    _delay_command_state=false;
    StartNextCommand();
}

void Device::CompleteRemove()
{
    if(_current_command==nullptr && !IsOpen()){
        if(_port_thread->isRunning())
        {
            _port_thread->exit(0);
        }
        else{
            _port_thread->deleteLater();
            this->deleteLater();
        }

    }
}


Device::DeviceStatus Device::GetStatus() const{
    return _current_status;
}

bool Device::IsOpen() const
{
    if(_want_remove)
        return false;
    return _device_port->IsOpen();
}

bool Device::IsDeviceDataLoaded() const
{
    return _device_data_loaded;
}

void Device::OpenPort(){
    if(_device_port->IsOpen())
        ClosePort();
    if(!this->_port.isEmpty())
    {
        _device_port->Open(_port,_device_info->GetBaudRate());
    }
}

void Device::ClosePort(){
    _device_port->Close();
}

void Device::UpdateDeviceStats(){
    if(_device_port->IsOpen())
    {
        GCode::DeviceStats* ds=new GCode::DeviceStats(const_cast<Device*>(this));
        QObject::connect(ds,&GCode::DeviceStats::Finished,this,&Device::WhenStatsUpdated);
        ds->setParent(nullptr);
        ds->moveToThread(_port_thread);
        ds->Start();
    }
}

void Device::Clear()
{
    ClosePort();
    _port="";
    CalculateAndSetStatus();
}

/*void Device::Write(QByteArray bytes)
{
    _device_port->Write(bytes);
}*/


void Device::AddGCodeCommand(GCodeCommand *command)
{
    command->setParent(this);
    _commands.append(command);
    emit CommandAdded(command);
    StartNextCommand();
}


void Device::ClearCommands()
{
    if(_commands.length()>0){
        for(GCodeCommand* command :_commands){
            ClearCommand(command);
        }
    }
}

void Device::ClearCommand(GCodeCommand *command)
{
    if(_commands.contains(command)){
        if(command->IsStarted())
        {
            command->Stop();
        }
        else
        {
            _commands.removeAll(command);
            emit CommandRemoved(command);
            command->deleteLater();
        }
    }
}

void Device::PauseCommands()
{
    _commands_paused=true;
}

void Device::PlayCommands()
{
    _commands_paused=false;
    StartNextCommand();

}

void Device::StartCommand(GCodeCommand *command)
{
    _device_port->Clear();
    _current_command=command;
    command->setParent(nullptr);
    command->moveToThread(_port_thread);
    QObject::connect(command,&GCodeCommand::Finished,this,&Device::WhenCommandFinished,Qt::ConnectionType::QueuedConnection);
    command->Start();
    emit CommandStarted(command);
}

bool Device::CommandsIsPlayed(){
    return !_commands_paused;
}

GCodeCommand *Device::GetCurrentCommand()
{
    GCodeCommand* command=nullptr;
    if(_commands.length()>0)
        command = _commands[0];
    return command;
}

QList<GCodeCommand *> Device::GetWaitingCommandsList() const
{
    return _commands;
}

DeviceFilesSystem *Device::GetFileSystem() const
{
    return _fileSystem;
}

Device::~Device()
{
    RemoteServer::GetInstance()->RemoveRequest(_network_reply);
}

void Device::OnErrorOccurred(int error)
{
    if(_want_remove)
        return;
    SetFlags(false,false);
    emit ErrorOccurred(error);
}

void Device::OnClosed()
{
    SetFlags(false,false);
    CalculateAndSetStatus();
    emit PortClosed();
    if(_want_remove)
        CompleteRemove();
}

void Device::OnOpen(bool b)
{
    CalculateAndSetStatus();
    if(b){
        emit PortOpened();
    }
    else{
    }

}

void Device::WhenCommandFinished(bool b)
{
    _commands.removeAll(_current_command);
    _last_command_time_finished=std::chrono::steady_clock::now();
    emit CommandFinished(_current_command,b);
    emit CommandRemoved(_current_command);
    _current_command->deleteLater();
    _current_command=nullptr;
    if(_want_remove)
    {
        CompleteRemove();
        return;
    }
    StartNextCommand();
}

void Device::StartNextCommand()
{
    if(!_commands.length() || !_device_port->IsOpen() || _current_command!=nullptr || !_is_ready || _commands_paused || _delay_command_state)
        return;
    if(DELAY_BETWEEN_COMMAND_AND_OTHER>0 && _delay_command_state==false){
        std::chrono::milliseconds ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-_last_command_time_finished);

        if(ms.count()<DELAY_BETWEEN_COMMAND_AND_OTHER)
        {
            _delay_command_timer->stop();
            _delay_command_timer->start(DELAY_BETWEEN_COMMAND_AND_OTHER-ms.count());
            _delay_command_state=true;
            return;
        }
    }
    StartCommand(_commands[0]);
}

void Device::OnDetectPort(QByteArray port)
{
    this->SetPort(port);
     _port_detector->deleteLater();
    _port_detector=nullptr;
    CalculateAndSetStatus();
    if(port.isEmpty()){
        emit DetectPortFailed();
    }
    else
        emit DetectPortSucceed();
}

void Device::WhenStatsUpdated()
{
    GCodeCommand* command=qobject_cast<GCodeCommand*>(this->sender());
    if(command->IsSuccess())
    {
        GCode::DeviceStats* stats=qobject_cast<GCode::DeviceStats*>(this->sender());
        this->_device_stats=stats->GetStats();
        emit DeviceStatsUpdated(stats);
        SetFlags(true,false);
    }
    else
    {
        if(command->GetError()==GCodeCommand::Busy)
        {
            SetFlags(false,true);
        }
        else{
            SetFlags(false,false);
        }
        emit DeviceStatsUpdateFailed(command);
    }
    this->sender()->deleteLater();
}

QMap<QByteArray, QByteArray> Device::GetStats() const
{
    return _device_stats;
}



DevicePort *Device::GetDevicePort(){
    return _device_port;
}

DeviceProblemSolver *Device::GetProblemSolver() const
{
    return _problem_solver;
}

DeviceMonitor *Device::GetDeviceMonitor()
{
    return _device_monitor;
}

PrintController *Device::GetPrintController()
{
    return _print_controller;
}

Camera *Device::GetCamera()
{
    return _camera;
}

void Device::Remove()
{

    QObject::disconnect(_device_port,&DevicePort::ErrorOccurred,this,&Device::OnErrorOccurred);
    emit DeviceRemoved();
    _want_remove=true;
    PauseCommands();
    ClearCommands();
    ClosePort();
    if(_current_command==nullptr)
        CompleteRemove();
}

void Device::Load()
{
    _fileSystem->ReadLocaleFile(DEVICE_DATA_FILE, [this](QByteArray data)->void{
        _device_data=QJsonDocument::fromJson(data);
        _device_data_loaded=true;
        emit this->DeviceDataLoaded();
        emit AfterDeviceDataLoaded();
    });
}

void Device::Save()
{
    QJsonObject jo=this->_device_data.object();
    jo.insert("status",(int)this->_current_status);
    jo.insert("time","time("+QDateTime::currentDateTimeUtc().toString(Qt::DateFormat::ISODateWithMs)+")");
    this->_device_data.setObject(jo);
    emit this->BeforeSaveDeviceData();
    _fileSystem->SaveLocaleFile(DEVICE_DATA_FILE,this->_device_data.toJson(),[this](bool success)->void{
        if(success){
            _network_reply=RemoteServer::GetInstance()->SendUpdateQuery([this](QNetworkReply* reply)->void{
                    _network_reply=nullptr;
                    reply->deleteLater();
            },DEVICES_TABLE,this->_device_data.object().toVariantMap(),this->_device_info->GetID());
            emit DataSaved();
        }
    });
}

void Device::AddData(QByteArray name, QJsonObject data)
{
    QJsonObject jo=this->_device_data.object();
    jo.insert(name,data);
    this->_device_data.setObject(jo);
}

QJsonObject Device::GetData(QByteArray name)
{
    if(_device_data.object().contains(name))
        return _device_data.object()[name].toObject();
    else
        return QJsonObject();
}
