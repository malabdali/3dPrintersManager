#include "printcontroller.h"
#include "device.h"
#include "devicefilessystem.h"
#include "regex"
#include "../config.h"
#include "devicemonitor.h"
#include "QJsonObject"
PrintController::PrintController(Device *dev):DeviceComponent(dev)
{
    _start_printing_command=nullptr;
    _stop_printing_command=nullptr;
    _set_temperatures_command=nullptr;
    _timer_id=-1;
    _wanted_status=Status::Nothing;
}

void PrintController::Setup()
{

    connect(_device,&Device::BeforeSaveDeviceData,this,&PrintController::Save);
    connect(_device,&Device::DeviceDataLoaded,this,&PrintController::Load);
    connect(_device,&Device::DeviceDataLoaded,this,&PrintController::AfterLoad);
    connect(_device,&Device::PortClosed,this,&PrintController::WhenPortClosed);
    connect(_device,&Device::PortOpened,this,&PrintController::WhenPortOpened);
    connect(_device,&Device::CommandFinished,this,&PrintController::WhenCommandFinished);
    connect(_device->GetDeviceMonitor(),&DeviceMonitor::updated,this,&PrintController::WhenMonitorUpdated);
    _timer_id=this->startTimer(PRINT_CONTROLLER_TIMER);
}

void PrintController::StartPrint(QByteArray file)
{
    if(_wanted_status==Printing || !_device->GetFileSystem()->GetFileList().contains(FileInfo(file)))
        return;
    else
    {
        _device->GetFileSystem()->ReadLocaleFile(QByteArray("files/")+file,[this,file](const QByteArray& ba)->void{
            _file_lines=QByteArrayList(ba.split('\n'));
            _printed_bytes=0;
            _total_bytes=0;
            CalculateWantedTempratures(0);
            SetCurrentStatus(Status::SendHeatUpCommands);
            _file=file;
            _wanted_status=Printing;
            emit WantedStatusChanged();
        });
    }
}

void PrintController::ContinuePrint()
{
}

void PrintController::StopPrint()
{
    _wanted_status=Stopped;
    SetCurrentStatus(SendStopCommand);
    emit WantedStatusChanged();
}

void PrintController::CalculateWantedTempratures(int line)
{
    _wanted_bed_temprature=0;
    _wanted_hottend_temperature=0;
    QByteArray hotend="";
    QByteArray bed="";
    if(line==0){
        hotend=this->LookForFirstLine(0,_file_lines.length(),"M104");
        bed=this->LookForFirstLine(0,_file_lines.length(),"M140");
    }
    else{
        hotend=this->LookForLastLine(0,line,"M104");
        bed=this->LookForLastLine(0,line,"M140");
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

bool PrintController::CanContinuePrinting()
{
    return false;
}

bool PrintController::IsPrinting()
{
    return _current_status==Printing;
}

QByteArrayList PrintController::LookForLines(int from, int to, QByteArray command)
{
    if(command.isEmpty())
        return _file_lines.mid(from,to-from+1);
    QByteArrayList list;
    for(int i=from;i<=to;i++) {
        if(_file_lines[i].startsWith(command))
            list.append(_file_lines[i]);
    }
    return list;
}

QByteArray PrintController::LookForLastLine(int from, int to, QByteArray command)
{
    for(int i=to;i>=from;i--) {
        if(_file_lines[i].startsWith(command))
            return _file_lines[i];
    }
    return "";
}

QByteArray PrintController::LookForFirstLine(int from, int to, QByteArray command)
{
    for(int i=from;i<=to;i++) {
        if(_file_lines[i].startsWith(command))
            return _file_lines[i];
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
    if(_current_status==Status::SendHeatUpCommands){
        if(_set_temperatures_command==nullptr){
            _set_temperatures_command=new GCode::SetTemperatures(_device,_wanted_bed_temprature,_wanted_hottend_temperature);
            _device->AddGCodeCommand(_set_temperatures_command);
        }
    }
    else if(_current_status==Status::HeatUp){
        DeviceMonitor* monitor=_device->GetDeviceMonitor();
        if(monitor->GetHotendTemperature()>_wanted_hottend_temperature*0.95 && monitor->GetBedTemperature()>_wanted_bed_temprature*0.95)
        {
            SetCurrentStatus(Status::SendPrintCommand);
        }
    }
    else if(_current_status==Status::SendPrintCommand){
        if(_start_printing_command==nullptr){
            this->_start_printing_command= new GCode::StartPrinting(_device,_file);
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
    qDebug()<<status;
    switch (status) {
    case PrintController::Nothing:
        _device->GetDeviceMonitor()->ResetIntervals();
        break;
    case PrintController::SendHeatUpCommands:
        break;
    case PrintController::HeatUp:
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
    if(changed)
    {
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
    else if(command==_stop_printing_command){
        if(success)
        {
            PrintStopped();
            SetCurrentStatus(SendHeatOffCommand);
        }
        _stop_printing_command=nullptr;
    }
    else if(command==_set_temperatures_command){
        if(success){
            if(_current_status==SendHeatUpCommands)
                SetCurrentStatus(Status::HeatUp);
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
    if(_wanted_status==Stopped)
        return;
    DeviceMonitor* monitor=_device->GetDeviceMonitor();
    if(monitor->IsPrinting()){
        _printed_bytes=monitor->GetPrintedBytes();
        _total_bytes=monitor->GetTotalBytes();
        this->SetCurrentStatus(Printing);
        qDebug()<<_printed_bytes<<_total_bytes;
    }
    else if(_wanted_status!=Printing && (_current_status==HeatUp||_current_status==SendPrintCommand||_current_status==SendHeatUpCommands)){
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
        this->SetCurrentStatus(SendHeatUpCommands);
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
    return QJsonDocument(QJsonObject::fromVariantHash(vh));
}

void PrintController::FromJson(QJsonDocument json)
{
    QJsonObject jo=json.object();

    /*if(jo.contains("WantedStatus")){
        _wanted_status=(Status)jo["WantedStatus"].toInt();
    }*/
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
    /*if(_wanted_status==Printing && _current_status!=Printing && !_file.isEmpty()){
        _wanted_status=Nothing;
        StartPrint(_file);
    }
    else if(_wanted_status==Stopped)
        StopPrint();*/
}
