#include "fdmdevice.h"
#include<functional>
#include <algorithm>
#include "../../config.h"
#include <QMetaMethod>
#include "../devicefilessystem.h"
#include "../gcodecommand.h"
#include "../deviceportdetector.h"
#include "../gcode/devicestats.h"
#include "../deviceconnection.h"
#include "../deviceport.h"
#include "../deviceproblemsolver.h"
#include "../deviceinfo.h"
#include <QTimer>
#include <QDir>

#include "../deviceactions.h"
#include "../remoteserver.h"
#include "../camera.h"
#include "../deviceudp.h"

FDMDevice::FDMDevice(DeviceInfo* device_info,QObject *parent):Device(device_info,parent)
{
    _problem_solver=new DeviceProblemSolver(this);
    _device_monitor=new FDMDeviceMonitor(this);
    _device_actions=new DeviceActions(this);
    _print_controller=new FDMPrintController(this);
    _camera=new Camera(this);
    _fileSystem=new FDMFilesSystem(this);
    if(_device_info->GetConnectionType()==DeviceInfo::ConnectionType::Serial)
        _device_connection=new DevicePort(this);
    else if(_device_info->GetConnectionType()!=DeviceInfo::ConnectionType::Serial)
        _device_connection=new DeviceUDP(this);
    _device_monitor->SetMonitorOptions(DeviceMonitor::FilamentRunout|DeviceMonitor::HotendAndBedTemperature|DeviceMonitor::PrintProgress);
    Setup();
}

void FDMDevice::Setup()
{
    Device::Setup();
}


void FDMDevice::UpdateDeviceStats(){
        if(_device_connection->IsOpen())
        {
            GCode::DeviceStats* ds=new GCode::DeviceStats(const_cast<FDMDevice*>(this));
            QObject::connect(ds,&GCode::DeviceStats::Finished,this,&FDMDevice::WhenStatsUpdated);
            ds->setParent(nullptr);
            ds->moveToThread(_port_thread);
            ds->Start();
        }
}

void FDMDevice::WhenStatsUpdated()
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


FDMFilesSystem *FDMDevice::GetFileSystem() const
{
    return (FDMFilesSystem *)_fileSystem;
}

FDMDeviceMonitor *FDMDevice::GetDeviceMonitor()
{
    return (FDMDeviceMonitor *)_device_monitor;
}
