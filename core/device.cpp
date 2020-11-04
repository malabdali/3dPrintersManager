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
#include "gcode/printingstats.h"
#include "deviceport.h"
#include "deviceproblemsolver.h"
#include "deviceinfo.h"
#include <QTimer>
#include <QDir>

Device::Device(DeviceInfo* device_info,QObject *parent) : QObject(parent),_port(""),_port_thread(new QThread()),
    _fileSystem(new DeviceFilesSystem(this,this)),_device_port(new DevicePort()),_device_info(device_info),_problem_solver(new DeviceProblemSolver(this))
{
    _port_detector=nullptr;
    qRegisterMetaType<Errors>();
    device_info->setParent(this);
    _is_ready=false;
    _commands_paused=false;
    _current_command=nullptr;
    _port_thread->start();
    _device_port->moveToThread(_port_thread);
    _last_command_time_finished=std::chrono::steady_clock::now();
    _fileSystem->Initiate();

    QObject::connect(_device_port,&DevicePort::ErrorOccurred,this,&Device::OnErrorOccurred);
    QObject::connect(_device_port,&DevicePort::PortClosed,this,&Device::OnClosed);
    QObject::connect(_device_port,&DevicePort::PortOpened,this,&Device::OnOpen);


}

void Device::DetectDevicePort()
{
    _port_detector=new DevicePortDetector(this->GetDeviceInfo()->GetDeviceName(),this->GetDeviceInfo()->GetBaudRate(),this);
    QObject::connect(_port_detector,&DevicePortDetector::DetectPortFinished,this,&Device::OnDetectPort);
    _port_detector->StartDetect();
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
    if(_device_port->IsOpen() && !_is_ready)
        ds=DeviceStatus::Connected;
    if(!_port.isEmpty() && !_device_port->IsOpen())
        ds=DeviceStatus::PortDetected;
    if(_device_port->IsOpen() && _is_ready)
        ds=DeviceStatus::Ready;

    SetStatus(ds);
}

void Device::SetReady(bool ready)
{
    bool old=_is_ready;
    _is_ready=ready;

    if(old!=ready)
        emit ReadyFlagChanged(ready);

    StartNextCommand();
}


Device::DeviceStatus Device::GetStatus() const{
    return _current_status;
}

bool Device::IsOpen() const
{
    return _device_port->IsOpen();
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
    _stats_update_steps=0;
    GCode::DeviceStats* ds=new GCode::DeviceStats(this);
    ds->setParent(nullptr);
    ds->moveToThread(_port_thread);
    QObject::connect(ds,&GCode::DeviceStats::Finished,this,&Device::WhenStatsUpdated);
    ds->Start();
}

void Device::Clear()
{
    ClosePort();
    _port="";
    CalculateAndSetStatus();
}

void Device::Write(QByteArray bytes)
{
    _device_port->Write(bytes);
}


void Device::AddGCodeCommand(GCodeCommand *command)
{

    _commands.append(command);
    emit CommandAdded(command);
    QObject::connect(command,&GCodeCommand::Finished,this,&Device::WhenCommandFinished,Qt::ConnectionType::QueuedConnection);
    StartNextCommand();
}


void Device::ClearCommands()
{
    if(_commands.length()>0){
        for(GCodeCommand* command :_commands){
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
    ClearCommands();
    delete _device_port;
    _port_thread->quit();
    _port_thread->wait();
    delete  _port_thread;
}

void Device::OnErrorOccurred(int error)
{
    SetReady(false);
    emit ErrorOccurred(error);
}

void Device::OnClosed()
{
    SetReady(false);
    emit PortClosed();
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
    StartNextCommand();
}

void Device::StartNextCommand()
{
    if(DELAY_BETWEEN_COMMAND_AND_OTHER>0 && _delay_command_state==true){
        std::chrono::milliseconds ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-_last_command_time_finished);
        if(ms.count()<DELAY_BETWEEN_COMMAND_AND_OTHER)
        {
            QTimer::singleShot(ms ,this,SLOT(StartNextCommand()));
            _delay_command_state=true;
            return;
        }
    }
    _delay_command_state=false;
    if(!_commands.length() || !_device_port->IsOpen() || _current_command!=nullptr || !_is_ready || _commands_paused)
        return;
    StartCommand(_commands[0]);
}

void Device::OnDetectPort(QByteArray port)
{
    this->SetPort(port);
    _port_detector=nullptr;
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
        if(GCode::DeviceStats* stats=qobject_cast<GCode::DeviceStats*>(this->sender())){
            this->_device_stats=stats->GetStats();
            GCode::PrintingStats* ds=new GCode::PrintingStats(this);
            ds->setParent(nullptr);
            ds->moveToThread(_port_thread);
            QObject::connect(ds,&GCode::PrintingStats::Finished,this,&Device::WhenStatsUpdated);
            ds->Start();
        }
        else if(GCode::PrintingStats* stats=qobject_cast<GCode::PrintingStats*>(this->sender())){
            this->_device_stats.insert("IS_PRINTING",QByteArray::number(stats->IsPrinting()));
            this->_device_stats.insert("PRINT_PERCENT",QByteArray::number(stats->GetPercent()));
            emit DeviceStatsUpdated(stats);
            this->SetReady(true);
        }
    }
    else
    {
        emit DeviceStatsUpdateFailed(command);
    }
    this->sender()->deleteLater();
}

bool Device::IsReady()
{
    return _is_ready && _device_port->IsOpen();
}

QMap<QByteArray, QByteArray> Device::GetStats() const
{
    return _device_stats;
}

QJsonDocument Device::GetStatsAsJSONObject() const
{
    QVariantHash vh;
    for(auto& [k,v]:_device_stats.toStdMap()){
        vh.insert(k,v);
    }
    return QJsonDocument(QJsonObject::fromVariantHash(vh));
}

DevicePort *Device::GetDevicePort(){
    return _device_port;
}

DeviceProblemSolver *Device::GetProblemSolver() const
{
    return _problem_solver;
}
