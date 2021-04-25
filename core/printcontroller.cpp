#include "printcontroller.h"
#include "device.h"
#include "devicefilessystem.h"
#include "regex"
#include "../config.h"
#include "devicemonitor.h"
#include "QJsonObject"
#include <string>
PrintController::PrintController(Device *dev):DeviceComponent(dev)
{
    _start_printing_command=nullptr;
    _stop_printing_command=nullptr;
    _set_temperatures_command=nullptr;
    _preprint_command=nullptr;
    _timer_id=-1;
    _wanted_status=Status::Nothing;
    _continue_print=false;
    _can_heatup=true;
}

void PrintController::Setup()
{

    connect(_device,&Device::BeforeSaveDeviceData,this,&PrintController::Save);
    connect(_device,&Device::DeviceDataLoaded,this,&PrintController::Load);
    connect(_device,&Device::DeviceDataLoaded,this,&PrintController::AfterLoad);
}

void PrintController::StartPrint(QByteArray file)
{
    if(_wanted_status==Printing || !_device->GetFileSystem()->GetFileList().contains(FileInfo(file)))
        return;
    else
    {
        _device->GetFileSystem()->ReadLocaleFile(QByteArray("files/")+file,[this,file](const QByteArray& ba)->void{
            _bed_temperature=0;
            _hotend_temperature=0;
            _continue_print=false;
            _start_printing_time=QDateTime::currentDateTime();
            _finished_printing_time=QDateTime();
            _file_content=ba;
            _printed_bytes=0;
            _total_bytes=0;
            CalculateWantedTempratures(0);
            SetCurrentStatus(Status::SendHeatUpBedCommands);
            _file=file;
            _wanted_status=Printing;
            emit WantedStatusChanged();
        });
    }
}

void PrintController::ContinuePrint()
{
    if(CanContinuePrinting()){
        _continue_print=true;
        _bed_temperature=0;
        _hotend_temperature=0;
        _finished_printing_time=QDateTime();
        if(_file_content.isEmpty()){
            _device->GetFileSystem()->ReadLocaleFile(QByteArray("files/")+_file,[this](const QByteArray& ba)->void{
                _file_content=ba;
                _printed_bytes=GetLastLayer(_printed_bytes);
                CalculateWantedTempratures(_printed_bytes);
                _wanted_fan_speed=GetLastFanSpeed(_printed_bytes);
                _wanted_extruder= GetLastEValue(_printed_bytes);
                _wanted_acceleration= GetLastAcceleration(_printed_bytes);
                _wanted_jerk=GetLastJerk(_printed_bytes);
                _max_acceleration=GetMaxAcceleration(_printed_bytes);
                _max_feedrate=GetMaxFeedRate(_printed_bytes);
                SetCurrentStatus(Status::PreprintPrepare);
                _wanted_status=Printing;
                emit WantedStatusChanged();
            });
        }
        else{
            _printed_bytes=GetLastLayer(_printed_bytes);
            CalculateWantedTempratures(_printed_bytes);
            _wanted_fan_speed=GetLastFanSpeed(_printed_bytes);
            _wanted_extruder= GetLastEValue(_printed_bytes);
            _wanted_acceleration= GetLastAcceleration(_printed_bytes);
            _wanted_jerk=GetLastJerk(_printed_bytes);
            _max_acceleration=GetMaxAcceleration(_printed_bytes);
            _max_feedrate=GetMaxFeedRate(_printed_bytes);
            SetCurrentStatus(Status::PreprintPrepare);
            _wanted_status=Printing;
            emit WantedStatusChanged();
        }
    }
}

void PrintController::StopPrint()
{
    _wanted_status=Stopped;
    SetCurrentStatus(SendStopCommand);
    emit WantedStatusChanged();
}

void PrintController::CalculateWantedTempratures(int bytes)
{
    _wanted_bed_temprature=0;
    _wanted_hottend_temperature=0;
    QByteArray hotend="";
    QByteArray bed="";
    if(bytes==0){
        hotend=this->LookForFirstLine(0,_file_content.size(),"M104");
        if(hotend.isEmpty())
            hotend=this->LookForFirstLine(0,_file_content.size(),"M109");

        bed=this->LookForFirstLine(0,_file_content.size(),"M140");
        if(hotend.isEmpty())
            bed=this->LookForFirstLine(0,_file_content.size(),"M190");


    }
    else{
        hotend=this->LookForLastLine(0,bytes,"M104");
        if(hotend.isEmpty())
            hotend=this->LookForFirstLine(0,bytes,"M109");
        bed=this->LookForLastLine(0,bytes,"M140");
        if(hotend.isEmpty())
            bed=this->LookForFirstLine(0,bytes,"M190");
    }
    std::match_results<QByteArray::iterator> res;
    std::regex regex(R"(S\d+)");
    if(!hotend.isEmpty())
    {
        std::regex_search(hotend.begin(),hotend.end(),res,regex);
        for(auto v:res){
            QByteArray ba=QByteArray::fromStdString(v.str());
            _wanted_hottend_temperature=ba.mid(1).toUInt();
        }
    }
    else
    {
        _wanted_hottend_temperature=0;
    }
    if(!bed.isEmpty())
    {
        std::regex_search(bed.begin(),bed.end(),res,regex);
        for(auto v:res){
            QByteArray ba=QByteArray::fromStdString(v.str());
            _wanted_bed_temprature=ba.mid(1).toUInt();
        }
    }
    else
    {
        _wanted_bed_temprature=0;
    }

}

PrintController::Status PrintController::GetWantedStatus()
{
    return _wanted_status;
}

PrintController::Status PrintController::GetCurrentStatus()
{
    return _current_status;
}

uint PrintController::GetWantedBedTemperature()
{
    return _wanted_bed_temprature;
}

uint PrintController::GetWantedHotendTemperature()
{
    return _wanted_hottend_temperature;
}

uint PrintController::GetLastLayer(int last)
{
    if((int)last>=_file_content.size())
        return 0;
    QByteArray ba=_file_content.left(last+1);
    int zindex=ba.lastIndexOf('Z');
    int nindex=ba.left(zindex).lastIndexOf('\n');
    return nindex;
}

double PrintController::GetLastEValue(int index)
{
    double evalue=0;
    if((int)index>=_file_content.size())
        return 0;
    QByteArray ba=_file_content.left(index+1);
    int eindex=ba.lastIndexOf('E');
    int nlindex=ba.indexOf('\n',eindex);
    QByteArray val=ba.mid(eindex,nlindex-eindex);
    std::match_results<QByteArray::iterator> res;
    std::regex regex(R"([0-9\.]+)");
    std::regex_search(val.begin(),val.end(),res,regex);
    if(res.length()>0){
        evalue=std::stod(res[0].str());
    }
    return evalue;
}

int PrintController::GetLastFanSpeed(int index)
{
    double fsvalue=-1;
    QByteArray val=this->LookForLastLine(0,index,"M106");
    if(val.length()>0){
        std::match_results<QByteArray::iterator> res;
        std::regex regex(R"(S\d+)");
        std::regex_search(val.begin(),val.end(),res,regex);
        if(res.length()>0){
            fsvalue=std::stod(res[0].str().substr(1));
        }
    }
    return fsvalue;
}

QByteArray PrintController::GetLastAcceleration(int index)
{
    QByteArray val=this->LookForLastLine(0,index,"M204");
    if(val.length()>0){
        return val.mid(5);
    }
    return "";
}

QByteArray PrintController::GetLastJerk(int index)
{
    QByteArray val1=this->LookForFirstLine(0,index,"M205");
    QByteArray val2=this->LookForLastLine(0,index,"M205");
    if(val1.length()>0){
        if(val1!=val2)
        {
            return val1.mid(5) + val2.mid(5);
        }
    }
    return "";
}

QByteArray PrintController::GetMaxFeedRate(int index)
{

    QByteArray val=this->LookForFirstLine(0,index,"M203");
    if(val.length()>0){
        return val.mid(5);
    }
    return "";
}

QByteArray PrintController::GetMaxAcceleration(int index)
{
    QByteArray val=this->LookForFirstLine(0,index,"M201");
    if(val.length()>0){
        return val.mid(5);
    }
    return "";
}


bool PrintController::CanContinuePrinting()
{
    DeviceMonitor* monitor=_device->GetDeviceMonitor();
    if(_printed_bytes>0 && _total_bytes>0 && !_file.isEmpty() && _current_status!=Printing && _wanted_status!=Printing && monitor->IsWasPrinting())
        return true;
    else
        return false;
}

bool PrintController::IsPrinting()
{
    return _current_status==Printing || _wanted_status==Printing;
}

void PrintController::SetHeatupAbility(bool b)
{
    _can_heatup=b;
}

bool PrintController::GetHeatupAbility() const
{
    return _can_heatup;
}

QByteArrayList PrintController::LookForLines(int from, int to, QByteArray command)
{
    QByteArrayList list=_file_content.mid(from,to-from+1).split('\n');
    if(command.isEmpty())
        return list;
    QByteArrayList list2;
    for(int i=0;i<=list.length()-1;i++) {
        if(!list[i].isEmpty()&&list[i].startsWith(command))
            list2.append(list[i]);
    }
    return list2;
}

QByteArray PrintController::LookForLastLine(int from, int to, QByteArray command)
{
    QByteArrayList list=_file_content.mid(from,to-from+1).split('\n');
    for(int i=list.length()-1;i>=0;i--) {
        if(!list[i].isEmpty()&&list[i].startsWith(command))
            return list[i];
    }
    return "";
}

QByteArray PrintController::LookForFirstLine(int from, int to, QByteArray command)
{
    QByteArrayList list=_file_content.mid(from,to-from+1).split('\n');
    for(int i=from;i<=to;i++) {
        if(list[i].startsWith(command))
            return list[i];
    }
    return "";
}


void PrintController::timerEvent(QTimerEvent *event)
{
    if(_wanted_status==Status::Printing)
        PrintUpdate();
    else if(_wanted_status==Status::Stopped)
        StopUpdate();
}

void PrintController::PrintUpdate()
{
    if(!_device->IsOpen() || _device->GetStatus()!=Device::DeviceStatus::Ready)
        return;
    if(_current_status==Status::PreprintPrepare){
        if(_preprint_command==nullptr){
            _preprint_command=new GCode::PrePrint(_device,_wanted_fan_speed,_wanted_acceleration,_wanted_jerk,_max_acceleration,_max_feedrate,_wanted_extruder,true,true);
            _device->AddGCodeCommand(_preprint_command);
        }
    }
    if(_current_status==Status::SendHeatUpBedCommands && _can_heatup){
        if(_set_temperatures_command==nullptr){
            _set_temperatures_command=new GCode::SetTemperatures(_device,_wanted_bed_temprature,-1);
            _device->AddGCodeCommand(_set_temperatures_command);
        }
    }
    else if(_current_status==Status::HeatUpBed){
        DeviceMonitor* monitor=_device->GetDeviceMonitor();
        if(_bed_temperature>_wanted_bed_temprature*0.95)
        {
            SetCurrentStatus(Status::SendHeatUpNozzleCommands);
        }
    }
    else if(_current_status==Status::SendHeatUpNozzleCommands && _can_heatup){
        if(_set_temperatures_command==nullptr){
            _set_temperatures_command=new GCode::SetTemperatures(_device,-1,_wanted_hottend_temperature);
            _device->AddGCodeCommand(_set_temperatures_command);
        }
    }
    else if(_current_status==Status::HeatUpNozzle){
        DeviceMonitor* monitor=_device->GetDeviceMonitor();
        if(_hotend_temperature>_wanted_hottend_temperature*0.95 && _bed_temperature>_wanted_bed_temprature*0.95)
        {
            SetCurrentStatus(Status::SendPrintCommand);
        }
    }
    else if(_current_status==Status::SendPrintCommand){
        if(_start_printing_command==nullptr){
            this->_start_printing_command= new GCode::StartPrinting(_device,_file,_printed_bytes,0);
            this->_device->AddGCodeCommand(_start_printing_command);
        }
        else{

        }
    }
    else if(_current_status==Printing)
    {
        PrintStarted();
        _wanted_status=Nothing;
    }
}

void PrintController::StopUpdate()
{
    if(!_device->IsOpen() || _device->GetStatus()!=Device::DeviceStatus::Ready)
        return;
    if(_current_status==SendStopCommand){
        if(_stop_printing_command==nullptr){
            this->_stop_printing_command=new GCode::StopSDPrint(_device);
            _device->AddGCodeCommand(_stop_printing_command);
        }
    }
    else if(_current_status==SendHeatOffCommand){
        if(_set_temperatures_command==nullptr){
            this->_set_temperatures_command=new GCode::SetTemperatures(_device,0,0);
            _device->AddGCodeCommand(_set_temperatures_command);
        }
    }
    else if(_current_status==Stopped)
    {
        emit PrintStopped();
        _wanted_status=Nothing;
    }

}

void PrintController::SetCurrentStatus(PrintController::Status status)
{
    bool changed=_current_status!=status;
    _current_status=status;
    if(changed)
    {
        switch (status) {
        case PrintController::Nothing:
            _device->GetDeviceMonitor()->ResetIntervals();
            break;
        case PrintController::SendHeatUpBedCommands:
            break;
        case PrintController::HeatUpBed:
            _device->GetDeviceMonitor()->SetUpdateIntervals(60000,3000,60000);
            break;

        case PrintController::SendHeatUpNozzleCommands:
            break;
        case PrintController::HeatUpNozzle:
            _device->GetDeviceMonitor()->SetUpdateIntervals(60000,3000,60000);
            break;
        case PrintController::SendPrintCommand:
            break;
        case PrintController::Printing:
            _device->GetDeviceMonitor()->SetUpdateIntervals(2000,60000,20000);
            break;
        case PrintController::SendStopCommand:
            break;
        case SendHeatOffCommand:
            break;
        case PrintController::Stopped:
            _device->GetDeviceMonitor()->ResetIntervals();
            break;
        }
        emit StatusChanged();
    }
}


void PrintController::WhenCommandFinished(GCodeCommand *command, bool success)
{
    if(command==_start_printing_command){
        if(success)
        {
            _file=_start_printing_command->GetFileName();
            SetCurrentStatus(Printing);
        }
        _start_printing_command=nullptr;
    }
    else if(_preprint_command==command){
        if(success)
        {
            SetCurrentStatus(SendHeatUpBedCommands);
        }
        _preprint_command=nullptr;
    }
    else if(command==_stop_printing_command){
        if(success)
        {
            emit PrintStopped();
            WhenPrintingFinished();
            SetCurrentStatus(SendHeatOffCommand);
        }
        _stop_printing_command=nullptr;
    }
    else if(command==_set_temperatures_command){
        if(success){
            if(_current_status==SendHeatUpBedCommands)
                SetCurrentStatus(Status::HeatUpBed);
            if(_current_status==SendHeatUpNozzleCommands)
                SetCurrentStatus(Status::HeatUpNozzle);
            else if(_current_status==SendHeatOffCommand)
                SetCurrentStatus(Status::Stopped);
        }
        _set_temperatures_command=nullptr;
    }
    else{
        GCode::StartPrinting* spc=dynamic_cast<GCode::StartPrinting*>(command);
        if(spc){
            if(success)
            {
                _file=spc->GetFileName();
                SetCurrentStatus(Printing);
            }
        }
    }

}

void PrintController::WhenMonitorUpdated()
{
    DeviceMonitor* monitor=_device->GetDeviceMonitor();
    _bed_temperature=monitor->GetBedTemperature();
    _hotend_temperature=monitor->GetHotendTemperature();
    if(_wanted_status==Stopped)
        return;
    if(!monitor->IsPrinting()&&_current_status==Printing){
        WhenPrintingFinished();
    }

    if(monitor->IsPrinting()){
        _printed_bytes=monitor->GetPrintedBytes();
        _total_bytes=monitor->GetTotalBytes();
        this->SetCurrentStatus(Printing);
    }
    else if(_wanted_status!=Printing && (_current_status==HeatUpBed||_current_status==SendPrintCommand||_current_status==SendHeatUpBedCommands||
                                         _current_status==HeatUpNozzle||_current_status==SendHeatUpNozzleCommands)){
        if(_wanted_status!=Stopped)
            StopPrint();
    }
    else if(_wanted_status==Nothing && _current_status!=Nothing){
        SetCurrentStatus(Nothing);
    }
}

void PrintController::WhenPortClosed()
{
    if(_wanted_status==Printing)
    {
        if(_continue_print)
            this->SetCurrentStatus(Status::PreprintPrepare);
        else{
            this->SetCurrentStatus(SendHeatUpBedCommands);
        }
    }
}

void PrintController::WhenPortOpened()
{

}

QJsonDocument PrintController::ToJson() const
{
    QVariantHash vh;
    vh.insert("WantedStatus",this->_wanted_status);
    vh.insert("CurrentStatus",this->_current_status);
    vh.insert("PrintedBytes",this->_printed_bytes);
    vh.insert("TotalBytes",this->_total_bytes);
    vh.insert("File",this->_file);
    vh.insert("WantedBedTemp",this->_wanted_bed_temprature);
    vh.insert("WantedHotendTemp",this->_wanted_hottend_temperature);
    if(!_start_printing_time.isNull())
        vh.insert("StartingTime",_start_printing_time.toString(Qt::DateFormat::ISODateWithMs));
    if(!_start_printing_time.isNull())
        vh.insert("FinishedTime",_finished_printing_time.toString(Qt::DateFormat::ISODateWithMs));
    return QJsonDocument(QJsonObject::fromVariantHash(vh));
}

void PrintController::FromJson(QJsonDocument json)
{
    QJsonObject jo=json.object();

    if(jo.contains("WantedStatus")){
        _wanted_status=(Status)jo["WantedStatus"].toInt();
    }
    if(jo.contains("CurrentStatus")){
        SetCurrentStatus((Status)jo["CurrentStatus"].toInt());
    }
    if(jo.contains("PrintedBytes")){
        _printed_bytes=jo["PrintedBytes"].toInt();
    }
    if(jo.contains("TotalBytes")){
        _total_bytes=jo["TotalBytes"].toInt();
    }
    if(jo.contains("File")){
        _file=jo["File"].toString().toUtf8();
    }
    if(jo.contains("WantedBedTemp")){
        _wanted_bed_temprature=jo["WantedBedTemp"].toInt();
    }
    if(jo.contains("WantedHotendTemp")){
        _wanted_hottend_temperature=jo["WantedHotendTemp"].toInt();
    }
    if(jo.contains("StartingTime")){
        _start_printing_time=QDateTime::fromString(jo["StartingTime"].toString(),Qt::DateFormat::ISODateWithMs);
    }
    if(jo.contains("FinishedTime")){
        _finished_printing_time=QDateTime::fromString(jo["FinishedTime"].toString(),Qt::DateFormat::ISODateWithMs);
    }
}

void PrintController::Save()
{
    _device->AddData("PrintController",this->ToJson().object());
}

void PrintController::Load()
{
    this->FromJson(QJsonDocument(_device->GetData("PrintController")));
}

void PrintController::AfterLoad()
{

    connect(_device,&Device::PortClosed,this,&PrintController::WhenPortClosed);
    connect(_device,&Device::PortOpened,this,&PrintController::WhenPortOpened);
    connect(_device,&Device::CommandFinished,this,&PrintController::WhenCommandFinished);
    connect(_device->GetDeviceMonitor(),&DeviceMonitor::updated,this,&PrintController::WhenMonitorUpdated);
    _timer_id=this->startTimer(PRINT_CONTROLLER_TIMER);
    /*if(_wanted_status==Printing && _current_status!=Printing && !_file.isEmpty()){
        _wanted_status=Nothing;
        StartPrint(_file);
    }
    else if(_wanted_status==Stopped)
        StopPrint();*/
}

void PrintController::WhenPrintingFinished()
{
    _finished_printing_time=QDateTime::currentDateTime();
    SetCurrentStatus(Nothing);
    emit PrintingFinished();
}



void PrintController::Disable()
{
    disconnect(_device,&Device::BeforeSaveDeviceData,this,&PrintController::Save);
    disconnect(_device,&Device::DeviceDataLoaded,this,&PrintController::Load);
    disconnect(_device,&Device::DeviceDataLoaded,this,&PrintController::AfterLoad);
}
