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
    _wanted_status=Status::Nothing;
}

void PrintController::Setup()
{

    connect(_device,&Device::BeforeSaveDeviceData,this,&PrintController::Save);
    connect(_device,&Device::DeviceDataLoaded,this,&PrintController::Load);
}



PrintController::Status PrintController::GetWantedStatus()
{
    return _wanted_status;
}

PrintController::Status PrintController::GetCurrentStatus()
{
    return _current_status;
}

void PrintController::SetWantedStatus(PrintController::Status status)
{
    bool changed=_wanted_status!=status;
    _wanted_status=status;
    if(changed)
        emit WantedStatusChanged();
}



bool PrintController::IsPrinting()
{
    return _current_status==Printing || _wanted_status==Printing;
}





void PrintController::Save()
{
    _device->AddData("PrintController",this->ToJson().object());
}

void PrintController::Load()
{
    this->FromJson(QJsonDocument(_device->GetData("PrintController")));
}





void PrintController::Disable()
{
    disconnect(_device,&Device::BeforeSaveDeviceData,this,&PrintController::Save);
    disconnect(_device,&Device::DeviceDataLoaded,this,&PrintController::Load);
}
