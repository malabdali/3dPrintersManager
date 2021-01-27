#include "deviceactions.h"
#include "device.h"
#include "devicemonitor.h"
#include "gcode/m600.h"
#include "./deviceproblemsolver.h"
#include <QTimer>
#include "config.h"
#include "printcontroller.h"
DeviceActions::DeviceActions(Device *device) : DeviceComponent(device),_device(device),_reconnect_timer(new QTimer(this)),_save_device_data_timer(new QTimer(this)),
    _stop_timer(new QTimer(this))
{
    _is_playing=true;
    _reconnect_timer->setSingleShot(true);
    _stop_timer->setSingleShot(true);
}

void DeviceActions::Setup(){
    connect(_reconnect_timer,&QTimer::timeout,this,&DeviceActions::SolveReconnectProblem);
    connect(_stop_timer,&QTimer::timeout,this,&DeviceActions::StopPrinting);
    connect(_save_device_data_timer,&QTimer::timeout,this,&DeviceActions::SaveData);
    connect(_device->GetDeviceMonitor(),&DeviceMonitor::updated,this,&DeviceActions::WhenMonitorUpdated);
    connect(_device,&Device::CommandFinished,this,&DeviceActions::WhenCommandFinished);
    connect(this->_device->GetProblemSolver(),&DeviceProblemSolver::ProblemDetected,this,&DeviceActions::WhenProblemDetected);
    connect(this->_device->GetProblemSolver(),&DeviceProblemSolver::SolveFinished,this,&DeviceActions::WhenSolveProblemFinished);
    connect(_device,&Device::DeviceDataLoaded,this,&DeviceActions::WhenDeviceLoaded);
    connect(_device->GetPrintController(),&PrintController::StatusChanged,this,&DeviceActions::WhenPrintStatusChanged);
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
    PrintController* pc= _device->GetPrintController();
    if(!_is_playing)
        return;
    DeviceMonitor* monitor=_device->GetDeviceMonitor();
    if(pc->IsPrinting() && monitor->IsFilamentRunout() && !monitor->IsPaused()){
        _stop_timer->start(WAIT_BEFORE_M600);
    }
    else{
        _stop_timer->stop();
    }

}

void DeviceActions::WhenCommandFinished(GCodeCommand *command, bool success)
{
    if(!_is_playing)
        return;
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

void DeviceActions::WhenDeviceLoaded()
{
    _save_device_data_timer->start(SAVE_DEVICE_DATA_EACH);
}

void DeviceActions::StopPrinting()
{
    _device->GetPrintController()->StopPrint();
    DeviceMonitor* monitor=_device->GetDeviceMonitor();
    if(monitor->IsPrinting()&& monitor->IsFilamentRunout() && !monitor->IsPaused()){
        _device->GetPrintController()->StopPrint();
    }
}

void DeviceActions::WhenPrintStatusChanged()
{
    _device->Save();
}
