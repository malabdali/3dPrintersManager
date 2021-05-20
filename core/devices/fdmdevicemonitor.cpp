#include "fdmdevicemonitor.h"
#include "../deviceconnection.h"
#include "../device.h"
#include "../deviceport.h"
#include "../../config.h"
#include <regex>
#include <string>
#include "../devicefilessystem.h"
#include "../gcode/startprinting.h"

FDMDeviceMonitor::FDMDeviceMonitor(Device *dev):DeviceMonitor(dev)
{
    _printing_stats=nullptr;
    _report_temprature=nullptr;
    _end_stops=nullptr;
}


void FDMDeviceMonitor::Update()
{
    if(_device->GetStatus()==Device::DeviceStatus::Ready)
    {
        _printing_stat_timer-=DEVICE_MONITOR_TIMER;
        _temperatures_timer-=DEVICE_MONITOR_TIMER;
        _endstops_timer-=DEVICE_MONITOR_TIMER;

        if(_printing_stats==nullptr && _printing_stat_timer<=0 && _monitor_options&MonitorOptions::PrintProgress)
        {
            _printing_stat_timer=_printing_stat_interval;
            _printing_stats=new GCode::PrintingStats(_device);
            _device->AddGCodeCommand(_printing_stats);
        }
        /*else if(_printing_stats && _printing_stats->IsStarted() && !_printing_stats->IsFinished())
            _printing_stats->Stop();*/
        if(_report_temprature==nullptr && _temperatures_timer<=0 && _monitor_options&MonitorOptions::HotendAndBedTemperature)
        {
            _temperatures_timer=_temperatures_interval;
            _report_temprature=new GCode::ReportTemperature(_device);
            _device->AddGCodeCommand(_report_temprature);
        }
        /*else if(_report_temprature && _report_temprature->IsStarted() && !_report_temprature->IsFinished())
            _report_temprature->Stop();*/
        if(_end_stops==nullptr && _endstops_timer<=0 && _monitor_options&MonitorOptions::FilamentRunout)
        {
            _endstops_timer=_endstops_interval;
            _end_stops=new GCode::EndstopsStates(_device);
            _device->AddGCodeCommand(_end_stops);
        }
        /*else if(_end_stops && _end_stops->IsStarted() && !_end_stops->IsFinished())
            _end_stops->Stop();*/
    }
}

void FDMDeviceMonitor::WhenDeviceStatsUpdated(GCodeCommand *command)
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

bool FDMDeviceMonitor::ReadTemperatureStats(QByteArray &ba)
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

bool FDMDeviceMonitor::CommandReader(GCodeCommand *command)
{
    this->_data.insert("IS_BUSY","0");
    if(_printing_stats && command==_printing_stats){
        this->_data.insert("IS_PRINTING",QByteArray::number(_printing_stats->IsPrinting()));
        this->_data.insert("PRINT_PERCENT",QByteArray::number(_printing_stats->GetPercent()));
        this->_data.insert("PRINTED_BYTES",QByteArray::number(_printing_stats->GetPrintedBytes()));
        this->_data.insert("TOTAL_BYTES",QByteArray::number(_printing_stats->GetTotalBytes()));
        if(this->IsPrinting())
        {
            this->_data.insert("IS_WAS_PRINTING","1");
        }
        return true;
    }
    else if(_report_temprature && command==_report_temprature){
        this->_data.insert("BED_TEMPERATURE",QByteArray::number(_report_temprature->GetBedTemperature()));
        this->_data.insert("HOTEND_TEMPERATURE",QByteArray::number(_report_temprature->GetHotendTemperature()));
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
    else if(command==_end_stops){
        this->_data.insert("NO_FILAMENT",_end_stops->FilamentExist()?"0":"1");
        return true;
    }
    return false;
}

void FDMDeviceMonitor::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    //qDebug()<<"DeviceMonitor::timerEvent 0"<<!_device->IsOpen() << _wait_device_stats << IsPaused();
    if(!_device->IsOpen() || _wait_device_stats || IsPaused())
    {
        return;
    }
    bool _is_updated=false;
    if(IsBusy()||_device->GetStatus()!=Device::DeviceStatus::Ready){
        qDebug()<<"DeviceMonitor::timerEvent 1";
        int64_t duration=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-_last_update_during_busy).count();
        if(_device->GetDeviceConnection()->IsThereAvailableLines())
        {
            qDebug()<<"DeviceMonitor::timerEvent 2";
            uint32_t i=0;
            while(i<_device->GetDeviceConnection()->LinesCount())
            {
                QByteArray ba=_device->GetDeviceConnection()->PeakLine(i);
                _is_updated=ReadTemperatureStats(ba);
                i++;
            }
            if(_is_updated){
                _last_update_during_busy=std::chrono::steady_clock::now();
                _device->GetDeviceConnection()->ReadAllLines();
                emit updated();
            }
            else if(_device->GetStatus()!=Device::DeviceStatus::Ready)
                _device->GetDeviceConnection()->ReadAllLines();
            else if(duration>=BUSY_DURATION){
                Update();
            }
        }
        else{
            qDebug()<<"DeviceMonitor::timerEvent 3";
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

void FDMDeviceMonitor::WhenCommandFinished(GCodeCommand* command, bool success)
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


void FDMDeviceMonitor::Setup()
{
    connect(_device,&Device::CommandFinished,this,&FDMDeviceMonitor::WhenCommandFinished);
    connect(_device,&Device::DeviceStatsUpdated,this,&FDMDeviceMonitor::WhenDeviceStatsUpdated);
    connect(_device,&Device::DeviceStatsUpdateFailed,this,&FDMDeviceMonitor::WhenDeviceStatsUpdated);
    DeviceMonitor::Setup();
}

void FDMDeviceMonitor::Disable()
{
    disconnect(_device,&Device::CommandFinished,this,&FDMDeviceMonitor::WhenCommandFinished);
    disconnect(_device,&Device::DeviceStatsUpdated,this,&FDMDeviceMonitor::WhenDeviceStatsUpdated);
    disconnect(_device,&Device::DeviceStatsUpdateFailed,this,&FDMDeviceMonitor::WhenDeviceStatsUpdated);
    DeviceMonitor::Disable();
}

void FDMDeviceMonitor::Play()
{
    connect(_device,&Device::CommandFinished,this,&FDMDeviceMonitor::WhenCommandFinished);
    connect(_device,&Device::DeviceStatsUpdated,this,&FDMDeviceMonitor::WhenDeviceStatsUpdated);
    connect(_device,&Device::DeviceStatsUpdateFailed,this,&FDMDeviceMonitor::WhenDeviceStatsUpdated);
    DeviceMonitor::Play();
}

void FDMDeviceMonitor::Pause()
{
    disconnect(_device,&Device::CommandFinished,this,&FDMDeviceMonitor::WhenCommandFinished);
    disconnect(_device,&Device::DeviceStatsUpdated,this,&FDMDeviceMonitor::WhenDeviceStatsUpdated);
    disconnect(_device,&Device::DeviceStatsUpdateFailed,this,&FDMDeviceMonitor::WhenDeviceStatsUpdated);
    DeviceMonitor::Pause();
}
