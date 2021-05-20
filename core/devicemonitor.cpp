#include "devicemonitor.h"
#include "device.h"
#include "deviceport.h"
#include "../config.h"
#include <regex>
#include <string>
#include "devicefilessystem.h"
#include "gcode/startprinting.h"
DeviceMonitor::DeviceMonitor(Device *dev) : DeviceComponent(dev)
{
    _wait_device_stats=false;
    this->setObjectName("Monitor");
    _monitor_options=0;
    _is_baused=true;
}

void DeviceMonitor::Setup(){
    connect(_device,&Device::BeforeSaveDeviceData,this,&DeviceMonitor::Save);
    connect(_device,&Device::DeviceDataLoaded,this,&DeviceMonitor::Load);
    Load();
    _printing_stat_timer=DEVICE_MONITOR_PRINTING_STAT_TIMER;
    _endstops_timer=DEVICE_MONITOR_END_STOPS_STAT_TIMER;
    _temperatures_timer=DEVICE_MONITOR_TEMPERATURE_STAT_TIMER;
    _printing_stat_interval=DEVICE_MONITOR_PRINTING_STAT_TIMER;
    _endstops_interval=DEVICE_MONITOR_END_STOPS_STAT_TIMER;
    _temperatures_interval=DEVICE_MONITOR_TEMPERATURE_STAT_TIMER;
    _last_update_during_busy=std::chrono::steady_clock::now();
}
bool DeviceMonitor::IsPrinting() const
{
    return _data.contains("IS_PRINTING")&&_data["IS_PRINTING"].toInt();
}

bool DeviceMonitor::IsBusy() const
{
    return _data.contains("IS_BUSY")&&_data["IS_BUSY"].toInt();
}

bool DeviceMonitor::IsWasPrinting() const
{
    return _data.contains("IS_WAS_PRINTING")&&_data["IS_WAS_PRINTING"].toInt();
}

bool DeviceMonitor::IsPaused() const
{
    return _is_baused;
}

bool DeviceMonitor::PrintingFinished() const
{
    return (_data.contains("IS_WAS_PRINTING")&&_data["IS_WAS_PRINTING"].toInt())&&(!_data.contains("IS_PRINTING")||!_data["IS_PRINTING"].toInt());
}

bool DeviceMonitor::IsFilamentRunout() const
{
    return _data.contains("NO_FILAMENT")&&_data["NO_FILAMENT"].toInt();
}

double DeviceMonitor::GetHotendTemperature() const
{
    if(_data.contains("HOTEND_TEMPERATURE"))
        return _data["HOTEND_TEMPERATURE"].toDouble();
    return 0;
}

double DeviceMonitor::GetBedTemperature() const
{
    if(_data.contains("BED_TEMPERATURE"))
        return _data["BED_TEMPERATURE"].toDouble();
    return 0;
}

uint DeviceMonitor::GetPrintedBytes() const
{
    if(_data.contains("PRINTED_BYTES"))
        return _data["PRINTED_BYTES"].toUInt();
    return 0;
}

uint DeviceMonitor::GetTotalBytes() const
{
    if(_data.contains("TOTAL_BYTES"))
        return _data["TOTAL_BYTES"].toUInt();
    return 0;
}

QByteArray DeviceMonitor::GetPrintingFile() const
{
    if(_data.contains("PRINTING_FILE"))
        return _data["PRINTING_FILE"];
    return 0;
}

void DeviceMonitor::SetUpdateIntervals(uint printingStats, uint temperatures, uint endStops)
{
    _endstops_interval=endStops;
    _printing_stat_interval=printingStats;
    _temperatures_interval=temperatures;
    _printing_stat_timer=printingStats;
    _endstops_timer=endStops;
    _temperatures_timer=temperatures;
}

void DeviceMonitor::ResetIntervals()
{
    _printing_stat_interval=DEVICE_MONITOR_PRINTING_STAT_TIMER;
    _endstops_interval=DEVICE_MONITOR_END_STOPS_STAT_TIMER;
    _temperatures_interval=DEVICE_MONITOR_TEMPERATURE_STAT_TIMER;
    _printing_stat_timer=DEVICE_MONITOR_PRINTING_STAT_TIMER;
    _endstops_timer=DEVICE_MONITOR_END_STOPS_STAT_TIMER;
    _temperatures_timer=DEVICE_MONITOR_TEMPERATURE_STAT_TIMER;
}

void DeviceMonitor::Reset()
{
    _data.clear();
    Save();
}

double DeviceMonitor::GetPrintProgress() const
{
    if(_data.contains("PRINT_PERCENT"))
        return _data["PRINT_PERCENT"].toDouble();
    return 0;
}

void DeviceMonitor::Save()
{
    _device->AddData("Monitor",this->ToJson().object());
}

void DeviceMonitor::Load()
{
    this->FromJson(QJsonDocument(_device->GetData("Monitor")));
}

void DeviceMonitor::Pause()
{
    this->killTimer(this->_monitor_timer);
    _is_baused=true;
}

void DeviceMonitor::Play()
{
    _monitor_timer=this->startTimer(DEVICE_MONITOR_TIMER);
    _is_baused=false;
}

void DeviceMonitor::SetMonitorOptions(int options)
{
    _monitor_options=options;
}

int DeviceMonitor::GetMonitorOptions()
{
    return _monitor_options;
}




QJsonDocument DeviceMonitor::ToJson()
{
    QVariantHash vh;
    for(auto& [k,v]:_data.toStdMap()){
        vh.insert(k,v);
    }
    return QJsonDocument(QJsonObject::fromVariantHash(vh));
}

void DeviceMonitor::FromJson(QJsonDocument json)
{

    for(int i=0;i<json.object().toVariantMap().count();i++){
        this->_data.insert(json.object().toVariantMap().keys()[i].toUtf8(),json.object().toVariantMap().values()[i].value<QByteArray>());
    }

    emit updated();
}


void DeviceMonitor::Disable()
{
    Pause();
    disconnect(_device,&Device::BeforeSaveDeviceData,this,&DeviceMonitor::Save);
    disconnect(_device,&Device::DeviceDataLoaded,this,&DeviceMonitor::Load);
}
