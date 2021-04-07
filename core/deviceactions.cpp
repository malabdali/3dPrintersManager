#include "deviceactions.h"
#include "device.h"
#include "devicemonitor.h"
#include "gcode/m600.h"
#include "./deviceproblemsolver.h"
#include <QTimer>
#include "config.h"
#include "devicefilessystem.h"
#include "printcontroller.h"
DeviceActions::DeviceActions(Device *device) : DeviceComponent(device),_device(device),_reconnect_timer(new QTimer(this)),_save_device_data_timer(new QTimer(this)),
    _stop_timer(new QTimer(this)),_sd_recheck_timer(new QTimer(this))
{
    _reconnect_timer->setSingleShot(true);
    _stop_timer->setSingleShot(true);
    _is_playing=false;
    _device_data_loaded=false;
}

void DeviceActions::Setup(){
    connect(_device,&Device::AfterDeviceDataLoaded,this,&DeviceActions::WhenDeviceLoaded);
    _device->Load();
}

void DeviceActions::Play()
{
    if(_is_playing)
        return;
    _is_playing=true;

    connect(_reconnect_timer,&QTimer::timeout,this,&DeviceActions::SolveReconnectProblem);
    connect(_stop_timer,&QTimer::timeout,this,&DeviceActions::StopPrinting);
    connect(_save_device_data_timer,&QTimer::timeout,this,&DeviceActions::SaveData);
    connect(_sd_recheck_timer,&QTimer::timeout,this,&DeviceActions::RecheckSDSUpport);
    connect(_device->GetDeviceMonitor(),&DeviceMonitor::updated,this,&DeviceActions::WhenMonitorUpdated);
    connect(_device,&Device::CommandFinished,this,&DeviceActions::WhenCommandFinished);
    connect(this->_device->GetProblemSolver(),&DeviceProblemSolver::ProblemDetected,this,&DeviceActions::WhenProblemDetected);
    connect(this->_device->GetProblemSolver(),&DeviceProblemSolver::SolveFinished,this,&DeviceActions::WhenSolveProblemFinished);
    connect(_device->GetPrintController(),&PrintController::StatusChanged,this,&DeviceActions::WhenPrintStatusChanged);
    connect(_device,&Device::StatusChanged,this,&DeviceActions::WhenDeviceStatusChanged);
    connect(_device,&Device::DeviceRemoved,this,&DeviceActions::WhenDeviceRemoved);
    _save_device_data_timer->start(SAVE_DEVICE_DATA_EACH);
}

void DeviceActions::Pause()
{
    Disable();
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
        if(!_stop_timer->isActive()){
            _stop_timer->start(WAIT_BEFORE_M600);
        }
    }
    else if(_stop_timer->isActive()){
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

void DeviceActions::RecheckSDSUpport()
{
    if(_device->GetStatus()==Device::DeviceStatus::Ready && !_device->GetFileSystem()->SdSupported()){
        _device->UpdateDeviceStats();
        _sd_recheck_timer->setSingleShot(true);
        _sd_recheck_timer->start(ACTION_WAITING_TIME);
    }
}

void DeviceActions::WhenDeviceLoaded()
{
    if(!IsPlaying())
    {
        Play();
    }
    _device_data_loaded=true;
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

void DeviceActions::WhenDeviceRemoved()
{
    Pause();
}

void DeviceActions::WhenDeviceStatusChanged()
{
    if(_device_data_loaded)
        _device->Save();

    RecheckSDSUpport();
}

void DeviceActions::WhenDeviceReady()
{
    this->_device->GetFileSystem()->UpdateFileList();
}


void DeviceActions::Disable()
{
    _is_playing=false;
    _reconnect_timer->stop();
    _save_device_data_timer->stop();
    disconnect(_reconnect_timer,&QTimer::timeout,this,&DeviceActions::SolveReconnectProblem);
    disconnect(_stop_timer,&QTimer::timeout,this,&DeviceActions::StopPrinting);
    disconnect(_save_device_data_timer,&QTimer::timeout,this,&DeviceActions::SaveData);
    disconnect(_device->GetDeviceMonitor(),&DeviceMonitor::updated,this,&DeviceActions::WhenMonitorUpdated);
    disconnect(_device,&Device::CommandFinished,this,&DeviceActions::WhenCommandFinished);
    disconnect(this->_device->GetProblemSolver(),&DeviceProblemSolver::ProblemDetected,this,&DeviceActions::WhenProblemDetected);
    disconnect(this->_device->GetProblemSolver(),&DeviceProblemSolver::SolveFinished,this,&DeviceActions::WhenSolveProblemFinished);
    disconnect(_device,&Device::DeviceDataLoaded,this,&DeviceActions::WhenDeviceLoaded);
    disconnect(_device->GetPrintController(),&PrintController::StatusChanged,this,&DeviceActions::WhenPrintStatusChanged);
    disconnect(_device,&Device::StatusChanged,this,&DeviceActions::WhenDeviceStatusChanged);
}
