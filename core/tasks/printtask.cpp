#include "printtask.h"
#include "../remoteserver.h"
#include "../devicefilessystem.h"
#include "../devicemonitor.h"
#include"../gcode/startprinting.h"
#include <QUrlQuery>
#include <QJsonArray>
#include "../gcode/stopsdprint.h"
#include <QJsonObject>
#include "../devices.h"
#include "../printcontroller.h"
PrintTask::PrintTask(QJsonObject data, QObject *parent):Task(data,parent)
{
    _file=data["file"].toString().toUtf8();
    _wait=false;
    _want_to_cancel=false;
    _want_to_cancel_finished=false;
    _download=nullptr;
    NextStep();
}

void PrintTask::DownloadFile()
{
    if(_device->GetFileSystem()->GetLocaleFiles(DOWNLOAD_PATH,QByteArray{".G"}).contains(_file)){
        SetStatus(TaskStatus::Downloaded);
        emit DownloadFileSuccess();
        _wait=false;
        NextStep();
        return;
    }
    SetStatus(TaskStatus::Downloading);
    QUrlQuery query;
    query.addQueryItem("file",this->_file);
    _download=RemoteServer::GetInstance()->SendRequest(query,"Files/FileInfo",[this](QNetworkReply* nr){
        if(RemoteServer::GetInstance()->IsSuccess(nr)){
            QJsonArray array=RemoteServer::GetInstance()->GetJSONValue(nr).toArray();
            if(array.size()>0){
                _file_info=array[0].toObject();
                RemoteServer::GetInstance()->Download([this](QNetworkReply *reply)->void{
                    if(RemoteServer::GetInstance()->DownloadIsSuccess(reply))
                    {
                        WhenDownloadFinished(reply->readAll());
                    }
                    else
                    {
                        _wait=false;
                        emit DownloadFileFailed();
                        NextStep();
                    }
                },_file);
            }
            else{
                _wait=false;
                emit FileNotExist();
                Finish();
            }
        }
        else{
            _wait=false;
            emit DownloadFileFailed();
            NextStep();
        }
        _download=nullptr;
        nr->deleteLater();
    });
}

void PrintTask::UploadFile()
{
    if(!_device->GetFileSystem()->SdSupported()){
        _device->UpdateDeviceStats();
        _wait=false;
        return;
    }
    if(_device->GetDeviceMonitor()->IsPrinting() || !_device->IsOpen() || _device->GetDeviceMonitor()->IsBusy() || _device->GetDeviceMonitor()->IsPaused() ||
            _device->GetFileSystem()->GetWaitUploadingList().contains(_file)|| _device->GetStatus()!=Device::DeviceStatus::Ready)
    {
        _wait=false;
        return;
    }
    if(_device->GetFileSystem()->GetFileList().contains(FileInfo(_file)))
    {
        SetStatus(TaskStatus::Uploaded);
        _wait=false;
        NextStep();
        return;
    }
    connect(_device->GetFileSystem(),&DeviceFilesSystem::FileUploaded,this,&PrintTask::WhenFileUploadSuccess,Qt::ConnectionType::UniqueConnection);
    connect(_device->GetFileSystem(),&DeviceFilesSystem::UploadFileFailed,this,&PrintTask::WhenFileUploadFailed,Qt::ConnectionType::UniqueConnection);
    SetStatus(TaskStatus::Uploading);

    _device->GetFileSystem()->UploadFile(_device->GetFileSystem()->GetLocaleDirectory(DOWNLOAD_PATH+_file));
}

void PrintTask::Print()
{
    DeviceMonitor* monitor=this->_device->GetDeviceMonitor();
    if(monitor->IsWasPrinting() || monitor->IsPrinting() || _device->GetStatus()!=Device::DeviceStatus::Ready ||
            _device->GetFileSystem()->IsStillUploading() || _device->GetPrintController()->GetWantedStatus()==PrintController::Printing){

        _wait=false;
        return;
    }
    _device->GetPrintController()->StartPrint(_file);
    connect(_device->GetPrintController(),&PrintController::WantedStatusChanged,this,&PrintTask::WhenPrintControllerWantedChanged);
}

QByteArray PrintTask::GetFile(){
    return _file;
}

void PrintTask::WhenDownloadFinished(const QByteArray &data)
{
    this->_device->GetFileSystem()->SaveLocaleFile(DOWNLOAD_PATH+_file,data,[this](bool s)->void{
        _wait=false;
        if(s){
            SetStatus(TaskStatus::Downloaded);
            emit DownloadFileSuccess();
        }
        else{
            emit DownloadFileFailed();
        }
        NextStep();
    });
}

void PrintTask::TryToCancel()
{
    if(this->IsStarted())
    {
        switch (GetStaus()) {
        case Task::Started:
            _want_to_cancel_finished=true;
            Cancel();
            break;
        case Task::Canceled:
            _want_to_cancel=false;
            break;
        case Task::Done:
            _want_to_cancel=false;
            break;
        case Task::Wait:
            _want_to_cancel_finished=true;
            Cancel();
            break;
        case Task::Downloaded:
            _want_to_cancel_finished=true;
            Cancel();
            break;
        case Task::Uploading:
            _device->GetFileSystem()->StopUpload(this->_file);
            _want_to_cancel_finished=true;
            Cancel();
            break;
        case Task::Uploaded:
            _want_to_cancel_finished=true;
            Cancel();
            break;
        case Task::Printing:
            this->StopPrinting();
            break;
        case Task::Printed:
            _want_to_cancel=false;
            break;

        }
    }
    else{
        Cancel();
    }
}

void PrintTask::StopPrinting()
{

    DeviceMonitor* monitor=this->_device->GetDeviceMonitor();
    PrintController* pc=_device->GetPrintController();
    if(!monitor->IsBusy() && _device->IsOpen() && _device->GetStatus()==Device::DeviceStatus::Ready){
        if(pc->GetCurrentStatus()==PrintController::Stopped || pc->GetCurrentStatus()==PrintController::Stopped ||
                (pc->GetCurrentStatus()==PrintController::Nothing && !monitor->IsPrinting()))
        {
            _want_to_cancel_finished=true;
            Cancel();
            return;
        }
        pc->StopPrint();
    }
}


void PrintTask::NextStep()
{
    DeviceMonitor* monitor=this->_device->GetDeviceMonitor();
    PrintController* pc=this->_device->GetPrintController();
    if(!_want_to_cancel){
        if((GetStaus()==TaskStatus::Downloading || GetStaus()==TaskStatus::Started)  && !_wait){
            _wait=true;
            DownloadFile();
        }
        else if((GetStaus()==TaskStatus::Downloaded || GetStaus()==TaskStatus::Uploading) && !_wait){
            _wait=true;
            UploadFile();
        }
        else if((GetStaus()==TaskStatus::Uploaded) && !_wait){
            _wait=true;
            Print();
        }
        else if((GetStaus()==TaskStatus::Printing) && !_wait && !monitor->IsPrinting()&&pc->GetWantedStatus()!=PrintController::Printing){
            this->SetStatus(Task::Printed);
        }
    }
    else{
        TryToCancel();
    }
    Task::NextStep();
}

void PrintTask::WhenFileUploadSuccess(QByteArray ba)
{
    _wait=false;
    if(ba==_file){
        disconnect(_device->GetFileSystem(),&DeviceFilesSystem::FileUploaded,this,&PrintTask::WhenFileUploadSuccess);
        disconnect(_device->GetFileSystem(),&DeviceFilesSystem::UploadFileFailed,this,&PrintTask::WhenFileUploadFailed);
        SetStatus(TaskStatus::Uploaded);
        NextStep();
    }
}

void PrintTask::WhenFileUploadFailed(QByteArray ba)
{
    _wait=false;
    if(ba==_file){
        disconnect(_device->GetFileSystem(),&DeviceFilesSystem::FileUploaded,this,&PrintTask::WhenFileUploadSuccess);
        disconnect(_device->GetFileSystem(),&DeviceFilesSystem::UploadFileFailed,this,&PrintTask::WhenFileUploadFailed);
        NextStep();
    }
}

void PrintTask::WhenCommandFinished(GCodeCommand *command, bool success)
{
}

void PrintTask::WhenPrintControllerWantedChanged()
{
    PrintController* pc=_device->GetPrintController();
    if(pc->GetWantedStatus()==PrintController::Printing)
    {
            SetStatus(Printing);
            disconnect(_device->GetPrintController(),&PrintController::WantedStatusChanged,this,&PrintTask::WhenPrintControllerWantedChanged);
    }
    else if(pc->GetWantedStatus()==PrintController::Stopped){

    }

    _wait=false;
}

PrintTask::~PrintTask()
{
    RemoteServer::GetInstance()->RemoveRequest(_download);
}


void PrintTask::Start()
{
    Task::Start();
}

void PrintTask::Finish()
{
    Task::Finish();
}


void PrintTask::Continue()
{
    Task::Continue();
}


bool PrintTask::NextStepIsFinished()
{
    if(this->GetStaus()==TaskStatus::Printed)
        return true;
    return false;
}

void PrintTask::Cancel()
{
    if(!_want_to_cancel)
    {
        WantToCancel();
        return;
    }
    else if(_want_to_cancel && !_want_to_cancel_finished){
        return;
    }
    Task::Cancel();
}


void PrintTask::Repeat()
{
    _want_to_cancel=false;
    _want_to_cancel_finished=false;
    Task::Repeat();
}

void PrintTask::WantToCancel()
{
    _want_to_cancel=true;
    this->NextStep();
}


void PrintTask::SetData(QJsonObject data)
{
    if(!_want_to_cancel && data["cancel"].toBool()){
        WantToCancel();
    }

    Task::SetData(data);
}
