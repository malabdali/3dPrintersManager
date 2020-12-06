#include "deviceactions.h"
#include "device.h"
#include "devicemonitor.h"
#include "gcode/m600.h"
#include "./deviceproblemsolver.h"
#include <QTimer>
#include "config.h"
DeviceActions::DeviceActions(Device *device) : DeviceComponent(device),_device(device),_reconnect_timer(new QTimer(this)),_save_device_data_timer(new QTimer(this))
{
}

void DeviceActions::Setup(){
    _is_playing=true;
    connect(_reconnect_timer,&QTimer::timeout,this,&DeviceActions::SolveReconnectProblem);
    connect(_save_device_data_timer,&QTimer::timeout,this,&DeviceActions::SaveData);
    connect(_device->GetDeviceMonitor(),&DeviceMonitor::updated,this,&DeviceActions::WhenMonitorUpdated);
    connect(_device,&Device::CommandFinished,this,&DeviceActions::WhenCommandFinished);
    QObject::connect(this->_device->GetProblemSolver(),&DeviceProblemSolver::ProblemDetected,this,&DeviceActions::WhenProblemDetected);
    QObject::connect(this->_device->GetProblemSolver(),&DeviceProblemSolver::SolveFinished,this,&DeviceActions::WhenSolveProblemFinished);
    _reconnect_timer->setSingleShot(true);
    _save_device_data_timer->start(SAVE_DATA_EACH);
    _device->Load();
}

void DeviceActions::Play()
{
    _is_playing=true;
}

void DeviceActions::Pause()
{
    _is_playing=false;
    _reconnect_timer->stop();
}

bool DeviceActions::IsPlaying()
{
    return _is_playing;
}

void DeviceActions::WhenMonitorUpdated()
{
    if(!_is_playing)
        return;
    DeviceMonitor* monitor=_device->GetDeviceMonitor();
    if(monitor->IsPrinting()&& monitor->IsFilamentRunout() && !monitor->IsPaused()){
        qDebug()<<"filament runout";
        GCode::M600* m600=new GCode::M600(_device);
        _device->AddGCodeCommand(m600);
    }

}

void DeviceActions::WhenCommandFinished(GCodeCommand *command, bool success)
{
    if(!_is_playing)
        return;
    if(command->GetGCode()=="M600" && success){
        qDebug()<<"sent M600";
        _device->GetDeviceMonitor()->Pause();
    }
}


void DeviceActions::WhenProblemDetected()
{
    _reconnect_timer->stop();
    if(!_is_playing)
        return;
    if(_device->GetProblemSolver()->GetSolvingType()== DeviceProblemSolver::OpenPort)
    {
        _reconnect_timer->start(ACTION_WAITING_TIME);
    }
    else
        _device->GetProblemSolver()->SolveProblem();


}

void DeviceActions::WhenSolveProblemFinished()
{
}

void DeviceActions::SolveReconnectProblem()
{
    _device->GetProblemSolver()->SolveProblem();
}

void DeviceActions::SaveData()
{
    _device->Save();
}
