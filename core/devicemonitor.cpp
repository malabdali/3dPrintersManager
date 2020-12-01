#include "devicemonitor.h"
#include "device.h"
#include "deviceport.h"
#include "../config.h"
#include <regex>
#include <string>
#include "devicefilessystem.h"
#include "gcode/startprinting.h"
DeviceMonitor::DeviceMonitor(Device *dev) : QObject(dev),_device(dev)
{
    _printing_stats=nullptr;
    _report_temprature=nullptr;
    _wait_device_stats=false;
    connect(_device,&Device::CommandFinished,this,&DeviceMonitor::WhenCommandFinished);
    connect(_device,&Device::DeviceStatsUpdated,this,&DeviceMonitor::WhenDeviceStatsUpdated);
    connect(_device,&Device::DeviceStatsUpdateFailed,this,&DeviceMonitor::WhenDeviceStatsUpdated);
    _last_update_during_busy=std::chrono::steady_clock::now();
    this->startTimer(DEVICE_MONITOR_TIMER);
}

bool DeviceMonitor::IsPrinting() const
{
    return _data.contains("IS_PRINTING")&&_data["IS_PRINTING"].toInt();
}

bool DeviceMonitor::IsBussy() const
{
    return _data.contains("IS_BUSY")&&_data["IS_BUSY"].toInt();
}

bool DeviceMonitor::IsWasPrinting() const
{
    return _data.contains("IS_WAS_PRINTING")&&_data["IS_WAS_PRINTING"].toInt();
}

bool DeviceMonitor::PrintingFinished() const
{
    return (_data.contains("IS_WAS_PRINTING")&&_data["IS_WAS_PRINTING"].toInt())&&(!_data.contains("IS_PRINTING")||!_data["IS_PRINTING"].toInt());
}

bool DeviceMonitor::GetFilamentState() const
{

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

QByteArray DeviceMonitor::GetPrintingFile() const
{
    if(_data.contains("PRINTING_FILE"))
        return _data["PRINTING_FILE"];
    return 0;
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
    if(_device->IsReady())
    {
        if(_printing_stats==nullptr)
        {
            _printing_stats=new GCode::PrintingStats(_device);
            _device->AddGCodeCommand(_printing_stats);
        }
        else if(_printing_stats->IsStarted() && !_printing_stats->IsFinished())
            _printing_stats->Stop();
        if(_report_temprature==nullptr)
        {
            _report_temprature=new GCode::ReportTemperature(_device);
            _device->AddGCodeCommand(_report_temprature);
        }
        else if(_report_temprature->IsStarted() && !_report_temprature->IsFinished())
            _report_temprature->Stop();
    }
}

void DeviceMonitor::Save()
{
    _device->GetFileSystem()->SaveLocaleFile("DeviceMonitor.txt",this->ToJson().toJson(),[](bool success)->void{});
}

void DeviceMonitor::Load()
{
    _device->GetFileSystem()->ReadLocaleFile("DeviceMonitor.txt",[this](QByteArray ba)->void{
        QJsonDocument jd=QJsonDocument::fromJson(ba);
        for(int i=0;i<jd.object().toVariantMap().count();i++){
            this->_data.insert(jd.object().toVariantMap().keys()[i].toUtf8(),jd.object().toVariantMap().values()[i].value<QByteArray>());
        }

        emit updated();
    });
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
    qDebug()<<command->GetGCode();
    bool updated=false;
    if(_printing_stats && command==_printing_stats){
        this->_data.insert("IS_PRINTING",QByteArray::number(_printing_stats->IsPrinting()));
        this->_data.insert("PRINT_PERCENT",QByteArray::number(_printing_stats->GetPercent()));
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
        this->_data.insert("IS_WAS_PRINTING","1");
        this->_data.insert("PRINTING_FILE",sp->GetFileName());
        return true;
    }
    return false;
}

void DeviceMonitor::timerEvent(QTimerEvent *event)
{
    if(!_device->IsOpen() || _wait_device_stats)
        return;
    bool _is_updated=false;
    if(IsBussy()||!_device->IsReady()){
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
            else if(!_device->IsReady())
                _device->GetDevicePort()->ReadAllLines();
        }
        else{
            int64_t duration=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now()-_last_update_during_busy).count();
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

}

QJsonDocument DeviceMonitor::ToJson() const
{
    QVariantHash vh;
    for(auto& [k,v]:_data.toStdMap()){
        vh.insert(k,v);
    }
    return QJsonDocument(QJsonObject::fromVariantHash(vh));
}
