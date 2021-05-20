#include "slaprintcontroller.h"
#include <QJsonDocument>
#include "../device.h"
#include "slafilessystem.h"
#include "sladevicemonitor.h"
SLAPrintController::SLAPrintController(Device *dev):PrintController(dev)
{
    _start_stop_printing_command=nullptr;
    _resume_pause_printing_command=nullptr;
    _file="";
    _timer_id=-1;
}


QJsonDocument SLAPrintController::ToJson()
{
    QVariantHash vh;
    vh.insert("WantedStatus",this->_wanted_status);
    vh.insert("CurrentStatus",this->_current_status);
    vh.insert("PrintedBytes",this->_printed_bytes);
    vh.insert("TotalBytes",this->_total_bytes);
    vh.insert("File",this->_file);
    if(!_start_printing_time.isNull())
        vh.insert("StartingTime",_start_printing_time.toString(Qt::DateFormat::ISODateWithMs));
    if(!_start_printing_time.isNull())
        vh.insert("FinishedTime",_finished_printing_time.toString(Qt::DateFormat::ISODateWithMs));
    return QJsonDocument(QJsonObject::fromVariantHash(vh));
}

void SLAPrintController::FromJson(QJsonDocument json)
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
    if(jo.contains("StartingTime")){
        _start_printing_time=QDateTime::fromString(jo["StartingTime"].toString(),Qt::DateFormat::ISODateWithMs);
    }
    if(jo.contains("FinishedTime")){
        _finished_printing_time=QDateTime::fromString(jo["FinishedTime"].toString(),Qt::DateFormat::ISODateWithMs);
    }
}

void SLAPrintController::AfterLoad()
{
    connect(_device,&Device::Closed,this,&SLAPrintController::WhenPortClosed);
    connect(_device,&Device::Opened,this,&SLAPrintController::WhenPortOpened);
    connect(_device,&Device::CommandFinished,this,&SLAPrintController::WhenCommandFinished);
    connect(_device->GetDeviceMonitor(),&DeviceMonitor::updated,this,&SLAPrintController::WhenMonitorUpdated);
}

void SLAPrintController::SetCurrentStatus(PrintController::Status status)
{
    bool changed=_current_status!=status;
    _current_status=status;
    if(changed)
    {
        emit StatusChanged();
    }
}

void SLAPrintController::WhenPrintingFinished()
{
    if(_wanted_status!=Paused)
        _finished_printing_time=QDateTime::currentDateTime();
    SetCurrentStatus(Nothing);
    emit PrintingFinished();
}

void SLAPrintController::timerEvent(QTimerEvent *event)
{
    if(_wanted_status==Status::Printing || _wanted_status==Status::Resuming)
        PrintUpdate();
    else if(_wanted_status==Status::Stopped || _wanted_status==Status::Paused)
        StopUpdate();
}

void SLAPrintController::PrintUpdate()
{

    if(!_device->IsOpen() || _device->GetStatus()!=Device::DeviceStatus::Ready)
        return;
    else if(_current_status==Status::SendPrintCommand){
        if(_start_stop_printing_command==nullptr &&  _resume_pause_printing_command==nullptr){
            if(_wanted_status==Printing)
            {
                this->_start_stop_printing_command= new GCode::Chitu::ChituStartStopPrint(_device,_file,true);
                _device->AddGCodeCommand(_start_stop_printing_command);
            }
            if(_wanted_status==Resuming)
            {
                this->_resume_pause_printing_command= new GCode::Chitu::ChituResumePausePrint(_device,true);
                _device->AddGCodeCommand(_resume_pause_printing_command);
            }
        }
        else{

        }
    }
    else if(_current_status==Printing)
    {
        SetWantedStatus(Nothing);
        emit PrintStarted();
    }
}

void SLAPrintController::StopUpdate()
{if(!_device->IsOpen() || _device->GetStatus()!=Device::DeviceStatus::Ready)
        return;
    if(_current_status==SendStopCommand){
        if(_start_stop_printing_command==nullptr){
            if(_wanted_status==Paused)
            {
                this->_resume_pause_printing_command= new GCode::Chitu::ChituResumePausePrint(_device,false);
                _device->AddGCodeCommand(_resume_pause_printing_command);
            }
            if(_wanted_status==Stopped)
            {
                this->_start_stop_printing_command= new GCode::Chitu::ChituStartStopPrint(_device,_file,false);
                _device->AddGCodeCommand(_start_stop_printing_command);
            }
        }
    }
    else if(_current_status==Stopped || _current_status==Paused)
    {
        SetWantedStatus(Nothing);
        emit PrintStopped();
    }
}

void SLAPrintController::WhenCommandFinished(GCodeCommand *command, bool success)
{
    if(command==_start_stop_printing_command){
        if(success)
        {
            if(_start_stop_printing_command->IsPrintCommand())
            {
                SetCurrentStatus(Printing);
            }
            else{
                SetCurrentStatus(Stopped);
            }
        }
        _start_stop_printing_command=nullptr;
    }
    else if(command==_resume_pause_printing_command){
        if(success)
        {
            if(_resume_pause_printing_command->IsResumCommand())
            {
                SetCurrentStatus(Printing);
            }
            else{
                SetCurrentStatus(Paused);
            }
        }
        _resume_pause_printing_command=nullptr;
    }
}

void SLAPrintController::WhenMonitorUpdated()
{
    if(_timer_id==-1){
        _timer_id=this->startTimer(PRINT_CONTROLLER_TIMER);
    }
    SLADeviceMonitor* monitor=dynamic_cast<SLADeviceMonitor*>(_device->GetDeviceMonitor());
    if(_wanted_status!=Nothing )
        return;
    if(!monitor->IsPrinting()&&_current_status==Printing){
        WhenPrintingFinished();
    }
    if(monitor->IsPrinting()){
        _printed_bytes=monitor->GetPrintedBytes();
        _total_bytes=monitor->GetTotalBytes();
        if(!monitor->PrintingIsPaused())
            this->SetCurrentStatus(Printing);
        else
            this->SetCurrentStatus(Paused);
    }
    else if(_wanted_status!=Printing && _wanted_status!=Stopped && _current_status==SendPrintCommand){
        StopPrint();
    }
    /*else if(_wanted_status==Nothing && _current_status!=Nothing){
        SetCurrentStatus(Nothing);
    }*/
}

void SLAPrintController::WhenPortClosed()
{

    this->killTimer(_timer_id);
    _timer_id=-1;

    if(_wanted_status==Printing || _wanted_status==Resuming)
    {
        this->SetCurrentStatus(Status::SendPrintCommand);
    }
    else if(_wanted_status==Stopped|| _wanted_status==Paused){
        SetCurrentStatus(SendStopCommand);
    }
}

void SLAPrintController::WhenPortOpened()
{

}


void SLAPrintController::Setup()
{
    connect(_device,&Device::DeviceDataLoaded,this,&SLAPrintController::AfterLoad);
    PrintController::Setup();
}

void SLAPrintController::Disable()
{
    disconnect(_device,&Device::DeviceDataLoaded,this,&SLAPrintController::AfterLoad);
    disconnect(_device,&Device::Closed,this,&SLAPrintController::WhenPortClosed);
    disconnect(_device,&Device::Opened,this,&SLAPrintController::WhenPortOpened);
    disconnect(_device,&Device::CommandFinished,this,&SLAPrintController::WhenCommandFinished);
    disconnect(_device->GetDeviceMonitor(),&DeviceMonitor::updated,this,&SLAPrintController::WhenMonitorUpdated);
    this->killTimer(_timer_id);
    PrintController::Disable();
}

void SLAPrintController::StartPrint(QByteArray file)
{
    if(_wanted_status==Resuming ||_wanted_status==Printing || !_device->GetFileSystem()->GetFileList().contains(FileInfo(file)))
        return;
    else
    {
        _file=file;
        _start_printing_time=QDateTime::currentDateTime();
        _finished_printing_time=QDateTime();
        SetWantedStatus(Printing);
        SetCurrentStatus(Status::SendPrintCommand);
        _printed_bytes=0;
        _total_bytes=0;
    }
}

void SLAPrintController::ContinuePrint()
{
    if(CanContinuePrinting())
    {
        _finished_printing_time=QDateTime();
        SetWantedStatus(Resuming);
        SetCurrentStatus(Status::SendPrintCommand);
        _finished_printing_time=QDateTime();
    }
}

void SLAPrintController::StopPrint()
{
    SetWantedStatus(Stopped);
    SetCurrentStatus(SendStopCommand);
}


bool SLAPrintController::CanContinuePrinting()
{
    DeviceMonitor* monitor=_device->GetDeviceMonitor();
    if(_current_status==Paused  && _wanted_status!=Printing && _wanted_status!=Resuming && monitor->IsPrinting())
        return true;
    else
        return false;
}


void SLAPrintController::PausePrint()
{
    SetWantedStatus(Paused);
    SetCurrentStatus(SendStopCommand);
}
