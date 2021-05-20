#include "sladevice.h"
#include<functional>
#include <algorithm>
#include "../../config.h"
#include <QMetaMethod>
#include "../gcodecommand.h"
#include "../gcode/CHITU/chitudevicestats.h"
#include "../deviceconnection.h"
#include "../deviceport.h"
#include "../deviceproblemsolver.h"
#include "../deviceinfo.h"
#include <QTimer>
#include <QDir>
#include "../remoteserver.h"
#include "slaprintcontroller.h"
#include "../camera.h"
#include "../deviceudp.h"
#include "../deviceactions.h"

SLADevice::SLADevice(DeviceInfo* device_info,QObject *parent):Device(device_info,parent)
{
    _problem_solver=new DeviceProblemSolver(this);
    _device_monitor=new SLADeviceMonitor(this);
    _device_actions=new DeviceActions(this);
    _print_controller=new SLAPrintController(this);
    _camera=new Camera(this);
    _fileSystem=new SLAFilesSystem(this);
    if(_device_info->GetConnectionType()==DeviceInfo::ConnectionType::Serial)
        _device_connection=new DevicePort(this);
    else if(_device_info->GetConnectionType()!=DeviceInfo::ConnectionType::Serial)
        _device_connection=new DeviceUDP(this);
    _device_monitor->SetMonitorOptions(DeviceMonitor::PrintProgress);
    Setup();
}

void SLADevice::Setup()
{
    Device::Setup();
}


void SLADevice::UpdateDeviceStats()
{
    if(_device_connection->IsOpen())
    {
        GCode::Chitu::ChituDeviceStats* ds=new GCode::Chitu::ChituDeviceStats(this);
        QObject::connect(ds,&GCode::Chitu::ChituDeviceStats::Finished,this,&SLADevice::WhenStatsUpdated);
        ds->setParent(nullptr);
        ds->moveToThread(_port_thread);
        ds->Start();
    }
}

void SLADevice::WhenStatsUpdated()
{
    GCodeCommand* command=qobject_cast<GCodeCommand*>(this->sender());
    if(command->IsSuccess())
    {
        GCode::Chitu::ChituDeviceStats* stats=qobject_cast<GCode::Chitu::ChituDeviceStats*>(this->sender());
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


SLAFilesSystem *SLADevice::GetFileSystem() const
{
    return (SLAFilesSystem *)_fileSystem;
}

SLADeviceMonitor *SLADevice::GetDeviceMonitor()
{
    return (SLADeviceMonitor *)_device_monitor;
}

