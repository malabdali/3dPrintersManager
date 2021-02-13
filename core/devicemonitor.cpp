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
    _printing_stats=nullptr;
    _report_temprature=nullptr;
    _end_stops=nullptr;
    _wait_device_stats=false;
    this->setObjectName("Monitor");
}

void DeviceMonitor::Setup(){
    connect(_device,&Device::CommandFinished,this,&DeviceMonitor::WhenCommandFinished);
    connect(_device,&Device::DeviceStatsUpdated,this,&DeviceMonitor::WhenDeviceStatsUpdated);
    connect(_device,&Device::DeviceStatsUpdateFailed,this,&DeviceMonitor::WhenDeviceStatsUpdated);
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
    _monitor_timer=this->startTimer(DEVICE_MONITOR_TIMER);
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
    return _data.contains("IS_PAUSED")&&_data["IS_PAUSED"].toInt();
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

void DeviceMonitor::Update()
{
    if(_device->GetStatus()==Device::DeviceStatus::Ready)
    {
        _printing_stat_timer-=DEVICE_MONITOR_TIMER;
        _temperatures_timer-=DEVICE_MONITOR_TIMER;
        _endstops_timer-=DEVICE_MONITOR_TIMER;

        if(_printing_stats==nullptr && _printing_stat_timer<=0)
        {
            _printing_stat_timer=_printing_stat_interval;
            _printing_stats=new GCode::PrintingStats(_device);
            _device->AddGCodeCommand(_printing_stats);
        }
        else if(_printing_stats && _printing_stats->IsStarted() && !_printing_stats->IsFinished())
            _printing_stats->Stop();
        if(_report_temprature==nullptr && _temperatures_timer<=0)
        {
            _temperatures_timer=_temperatures_interval;
            _report_temprature=new GCode::ReportTemperature(_device);
            _device->AddGCodeCommand(_report_temprature);
        }
        else if(_report_temprature && _report_temprature->IsStarted() && !_report_temprature->IsFinished())
            _report_temprature->Stop();
        if(_end_stops==nullptr && _endstops_timer<=0)
        {
            _endstops_timer=_endstops_interval;
            _end_stops=new GCode::EndstopsStates(_device);
            _device->AddGCodeCommand(_end_stops);
        }
        else if(_end_stops && _end_stops->IsStarted() && !_end_stops->IsFinished())
            _end_stops->Stop();
    }
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
    this->_data.insert("IS_PAUSED","1");
    emit updated();
}

void DeviceMonitor::Play()
{
    this->_data.insert("IS_PAUSED","0");
    emit updated();
}

void DeviceMonitor::WhenDeviceStatsUpdated(GCodeCommand *command)
{
    _wait_device_stats=false;
    if(command->GetError()==GCodeCommand::Busy)
    {
        this->_data.insert("IS_BUSY","1");
        this->_data.insert("IS_WAS_PRINTING","1");
        _last_update_during_busy=std::chrono::steady_clock::now();
    }
    else
    {
        this->_data.insert("IS_BUSY","0");
    }
}

bool DeviceMonitor::ReadTemperatureStats(QByteArray &ba)
{
    if(ba.startsWith("T:")){
        std::match_results<QByteArray::iterator> matches;
        std::regex_search(ba.begin(),ba.end(),matches,std::regex(R"((T:\d+\.\d+))"));
        for(auto m:matches)
        {
            QList<QByteArray> list=QByteArray::fromStdString(m.str()).split(':');
            this->_data.insert("HOTEND_TEMPERATURE",list[1]);
        }
        std::regex_search(ba.begin(),ba.end(),matches,std::regex(R"((B:\d+\.\d+))"));
        for(auto m:matches)
        {
            QList<QByteArray> list=QByteArray::fromStdString(m.str()).split(':');
            this->_data.insert("BED_TEMPERATURE",list[1]);
        }
        return true;
    }
    return false;
}

bool DeviceMonitor::CommandReader(GCodeCommand *command)
{
    bool updated=false;
    if(_printing_stats && command==_printing_stats){
        this->_data.insert("IS_PRINTING",QByteArray::number(_printing_stats->IsPrinting()));
        this->_data.insert("PRINT_PERCENT",QByteArray::number(_printing_stats->GetPercent()));
        this->_data.insert("PRINTED_BYTES",QByteArray::number(_printing_stats->GetPrintedBytes()));
        this->_data.insert("TOTAL_BYTES",QByteArray::number(_printing_stats->GetTotalBytes()));
        this->_data.insert("IS_BUSY","0");
        if(this->IsPrinting())
        {
            this->_data.insert("IS_WAS_PRINTING","1");
        }
        _printing_stats=nullptr;
        return true;
    }
    else if(_report_temprature && command==_report_temprature){
        this->_data.insert("BED_TEMPERATURE",QByteArray::number(_report_temprature->GetBedTemperature()));
        this->_data.insert("HOTEND_TEMPERATURE",QByteArray::number(_report_temprature->GetHotendTemperature()));
        this->_data.insert("IS_BUSY","0");
        _report_temprature=nullptr;
        return true;
    }
    else if(GCode::StartPrinting* sp=dynamic_cast<GCode::StartPrinting*>(command)){
        this->_data.insert("IS_PRINTING","1");
        this->_data.insert("PRINT_PERCENT","0");
        this->_data.insert("PRINTED_BYTES","0");
        this->_data.insert("TOTAL_BYTES","0");
        this->_data.insert("IS_WAS_PRINTING","1");
        this->_data.insert("PRINTING_FILE",sp->GetFileName());
        this->_data.insert("IS_BUSY","0");
        return true;
    }
    else if(GCode::EndstopsStates* sp=dynamic_cast<GCode::EndstopsStates*>(command)){
        this->_data.insert("NO_FILAMENT",sp->FilamentExist()?"0":"1");
        this->_data.insert("IS_BUSY","0");
        return true;
    }
    return false;
}

void DeviceMonitor::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    if(!_device->IsOpen() || _wait_device_stats || IsPaused())
    {
        return;
    }
    bool _is_updated=false;
    if(IsBusy()||_device->GetStatus()!=Device::DeviceStatus::Ready){
        int64_t duration=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-_last_update_during_busy).count();
        if(_device->GetDevicePort()->IsThereAvailableLines())
        {
            uint32_t i=0;
            while(i<_device->GetDevicePort()->LinesCount())
            {
                QByteArray ba=_device->GetDevicePort()->PeakLine(i);
                _is_updated=ReadTemperatureStats(ba);
                i++;
            }
            if(_is_updated){
                _last_update_during_busy=std::chrono::steady_clock::now();
                _device->GetDevicePort()->ReadAllLines();
                emit updated();
            }
            else if(_device->GetStatus()!=Device::DeviceStatus::Ready)
                _device->GetDevicePort()->ReadAllLines();
            else if(duration>=BUSY_DURATION){
                Update();
            }
        }
        else{
            if(duration>=BUSY_DURATION)
            {
                if(_device->GetStatus()==Device::DeviceStatus::Ready)
                    Update();
                else
                {
                    _device->UpdateDeviceStats();
                    _wait_device_stats=true;
                }
            }
        }
    }
    else{
        Update();
    }

    Save();
}

void DeviceMonitor::WhenCommandFinished(GCodeCommand* command, bool success)
{

    if(success){
        if(CommandReader(command))
            emit updated();
    }
    else{
        if(command->GetError()==GCodeCommand::Busy)
        {
            this->_data.insert("IS_BUSY","1");
            _last_update_during_busy=std::chrono::steady_clock::now();
            this->_data.insert("IS_WAS_PRINTING","1");
        }
        else
            this->_data.insert("IS_BUSY","0");
    }
    if(command==this->_printing_stats)
        _printing_stats=nullptr;
    if(command==this->_report_temprature)
        _report_temprature=nullptr;
    if(command==this->_end_stops)
        _end_stops=nullptr;

}

void DeviceMonitor::WhenDeviceRemoved()
{
    this->killTimer(this->_monitor_timer);
}

QJsonDocument DeviceMonitor::ToJson() const
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
