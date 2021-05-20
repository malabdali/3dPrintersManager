#include "sladevicemonitor.h"
#include "../gcode/CHITU/chituprintingstats.h"
#include "../deviceconnection.h"
#include "../device.h"
#include "../deviceport.h"
#include "../../config.h"
#include <regex>
#include <string>
#include "../devicefilessystem.h"
#include "../gcode/CHITU/chitustartstopprint.h"
#include "../gcode/CHITU/chituresumepauseprint.h"

SLADeviceMonitor::SLADeviceMonitor(Device *dev):DeviceMonitor(dev)
{
    _printing_stats=nullptr;
}


void SLADeviceMonitor::Update()
{
    if(_device->GetStatus()==Device::DeviceStatus::Ready)
    {
        _printing_stat_timer-=DEVICE_MONITOR_TIMER;

        if(_printing_stats==nullptr && _printing_stat_timer<=0 && _monitor_options&MonitorOptions::PrintProgress)
        {
            _printing_stat_timer=_printing_stat_interval;
            _printing_stats=new GCode::Chitu::ChituPrintingStats(_device);
            _device->AddGCodeCommand(_printing_stats);
        }
    }
}

bool SLADeviceMonitor::PrintingIsPaused()
{
    qDebug()<<"SLADeviceMonitor::PrintingIsPaused()"<<(_data.contains("PRINTING_IS_PAUSED")&&_data["PRINTING_IS_PAUSED"].toInt());
    return _data.contains("PRINTING_IS_PAUSED")&&_data["PRINTING_IS_PAUSED"].toInt();

}

void SLADeviceMonitor::WhenDeviceStatsUpdated(GCodeCommand *command)
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

bool SLADeviceMonitor::ReadTemperatureStats(QByteArray &ba)
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

bool SLADeviceMonitor::CommandReader(GCodeCommand *command)
{
    this->_data.insert("IS_BUSY","0");
    if(_printing_stats && command==_printing_stats){
        if(this->GetPrintedBytes()!=_printing_stats->GetPrintedBytes()){
            _last_change_in_printing_progress=std::chrono::steady_clock::now();
        }
        this->_data.insert("IS_PRINTING",QByteArray::number(_printing_stats->IsPrinting()));
        this->_data.insert("PRINT_PERCENT",QByteArray::number(_printing_stats->GetPercent()));
        this->_data.insert("PRINTED_BYTES",QByteArray::number(_printing_stats->GetPrintedBytes()));
        this->_data.insert("TOTAL_BYTES",QByteArray::number(_printing_stats->GetTotalBytes()));
        if(this->IsPrinting())
        {
            this->_data.insert("IS_WAS_PRINTING","1");
        }
        CheckPauseState();
        return true;
    }
    else if(GCode::Chitu::ChituStartStopPrint* sp=dynamic_cast<GCode::Chitu::ChituStartStopPrint*>(command)){
        if(sp->IsPrintCommand()){
            _last_change_in_printing_progress=std::chrono::steady_clock::now()+std::chrono::milliseconds(DEVICE_MONITOR_TIMER_OF_PAUSED_PRINTER);
            this->_data.insert("IS_PRINTING","1");
            this->_data.insert("PRINT_PERCENT","0");
            this->_data.insert("PRINTED_BYTES","0");
            this->_data.insert("TOTAL_BYTES","0");
            this->_data.insert("IS_WAS_PRINTING","1");
            this->_data.insert("PRINTING_FILE",sp->GetFileName());
            this->_data.insert("PRINTING_IS_PAUSED","0");
        }
        return true;
    }
    else if(GCode::Chitu::ChituResumePausePrint* resume=dynamic_cast<GCode::Chitu::ChituResumePausePrint*>(command)){
        if(!resume->IsResumCommand()){
            _last_change_in_printing_progress=std::chrono::steady_clock::now()-std::chrono::milliseconds(DEVICE_MONITOR_TIMER_OF_PAUSED_PRINTER);
            this->_data.insert("IS_PRINTING","1");
            this->_data.insert("PRINT_PERCENT","0");
            this->_data.insert("PRINTED_BYTES","0");
            this->_data.insert("TOTAL_BYTES","0");
            this->_data.insert("IS_WAS_PRINTING","1");
            this->_data.insert("PRINTING_IS_PAUSED","1");
        }
        else if(resume->IsResumCommand()){
            _last_change_in_printing_progress=std::chrono::steady_clock::now()+std::chrono::milliseconds(DEVICE_MONITOR_TIMER_OF_PAUSED_PRINTER);
            this->_data.insert("IS_PRINTING","1");
            this->_data.insert("PRINT_PERCENT","0");
            this->_data.insert("PRINTED_BYTES","0");
            this->_data.insert("TOTAL_BYTES","0");
            this->_data.insert("IS_WAS_PRINTING","1");
            this->_data.insert("PRINTING_IS_PAUSED","0");
        }
        return true;
    }
    return false;
}

void SLADeviceMonitor::CheckPauseState()
{
    qDebug()<<"SLADeviceMonitor::CheckPauseState()";
    if(IsPrinting()){
        int v=std::chrono::duration_cast<std::chrono::milliseconds>((std::chrono::steady_clock::now()-_last_change_in_printing_progress)).count();

        if(v>DEVICE_MONITOR_TIMER_OF_PAUSED_PRINTER){
            this->_data.insert("PRINTING_IS_PAUSED","1");
            qDebug()<<"SLADeviceMonitor::PrintingIsPaused"<<true;
        }
        else{
            this->_data.insert("PRINTING_IS_PAUSED","0");
            qDebug()<<"SLADeviceMonitor::PrintingIsPaused"<<false;
        }
    }
    else{
        this->_data.insert("PRINTING_IS_PAUSED","0");
        qDebug()<<"SLADeviceMonitor::PrintingIsPaused"<<false;
    }
}

void SLADeviceMonitor::timerEvent(QTimerEvent *event)
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

void SLADeviceMonitor::WhenCommandFinished(GCodeCommand* command, bool success)
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

}

void SLADeviceMonitor::WhenDataLoaded()
{
    qDebug()<<"SLADeviceMonitor::WhenDataLoaded()";
    this->_last_change_in_printing_progress=std::chrono::steady_clock::now();
    if(PrintingIsPaused()){
        qDebug()<<"SLADeviceMonitor::WhenDataLoaded()2";
        _last_change_in_printing_progress=std::chrono::steady_clock::now()-std::chrono::milliseconds(DEVICE_MONITOR_TIMER_OF_PAUSED_PRINTER);

    }

}


void SLADeviceMonitor::Setup()
{
    //connect(_device,&Device::AfterDeviceDataLoaded,this,&SLADeviceMonitor::WhenDataLoaded);
    DeviceMonitor::Setup();
}

void SLADeviceMonitor::Disable()
{
    //disconnect(_device,&Device::AfterDeviceDataLoaded,this,&SLADeviceMonitor::WhenDataLoaded);
    DeviceMonitor::Disable();
}

void SLADeviceMonitor::Play()
{
    connect(_device,&Device::CommandFinished,this,&SLADeviceMonitor::WhenCommandFinished);
    connect(_device,&Device::DeviceStatsUpdated,this,&SLADeviceMonitor::WhenDeviceStatsUpdated);
    connect(_device,&Device::DeviceStatsUpdateFailed,this,&SLADeviceMonitor::WhenDeviceStatsUpdated);
    DeviceMonitor::Play();
    WhenDataLoaded();
}

void SLADeviceMonitor::Pause()
{
    disconnect(_device,&Device::CommandFinished,this,&SLADeviceMonitor::WhenCommandFinished);
    disconnect(_device,&Device::DeviceStatsUpdated,this,&SLADeviceMonitor::WhenDeviceStatsUpdated);
    disconnect(_device,&Device::DeviceStatsUpdateFailed,this,&SLADeviceMonitor::WhenDeviceStatsUpdated);
    DeviceMonitor::Pause();
}
