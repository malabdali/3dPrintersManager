#include "printtask.h"
#include "../remoteserver.h"
#include "../devicefilessystem.h"
#include "../devicemonitor.h"
#include"../gcode/startprinting.h"
#include <QUrlQuery>
#include <QJsonArray>
PrintTask::PrintTask(QJsonObject data, QObject *parent):Task(data,parent)
{
    _file=data["file"].toString().toUtf8();
    _wait=false;
    _printing_command=nullptr;
    NextStep();
}

void PrintTask::DownloadFile()
{
    if(_device->GetFileSystem()->GetLocaleFiles(DOWNLOAD_PATH,QByteArray{".G"}).contains(_file)){
        SetStatus(TaskStatus::Downloaded);
        emit DownloadFileSuccess();
        _wait=false;
        NextStep();
    }
    SetStatus(TaskStatus::Downloading);
    QUrlQuery query;
    query.addQueryItem("file",this->_file);
    RemoteServer::GetInstance()->SendRequest(query,"Files/FileInfo",[this](QNetworkReply* nr){
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
    });
}

void PrintTask::UploadFile()
{
    if(_device->GetDeviceMonitor()->IsPrinting() || !_device->IsOpen() || _device->GetDeviceMonitor()->IsBussy() || _device->GetDeviceMonitor()->IsPaused() ||
            _device->GetFileSystem()->GetWaitUploadingList().contains(_file)|| _device->GetStatus()!=Device::DeviceStatus::Ready)
    {
        _wait=false;
        return;
    }
    if(_device->GetFileSystem()->GetFileList().contains(FileInfo(_file)))
    {
        SetStatus(TaskStatus::Uploaded);
        _wait=false;
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
    if(monitor->IsWasPrinting() || monitor->IsPrinting() || _device->GetStatus()!=Device::DeviceStatus::Ready || _device->GetFileSystem()->IsStillUploading() ||_printing_command){

        _wait=false;
        return;
    }
    this->_printing_command=new GCode::StartPrinting(_device,_file);
    _device->AddGCodeCommand(_printing_command);
    connect(_device,&Device::CommandFinished,this,&PrintTask::WhenCommandFinished);
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


void PrintTask::NextStep()
{
    DeviceMonitor* monitor=this->_device->GetDeviceMonitor();
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
    else if((GetStaus()==TaskStatus::Printing) && !_wait && !monitor->IsPrinting() && monitor->IsWasPrinting()){
        this->SetStatus(Task::Printed);
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
    if(command==_printing_command){
        if(success)
            SetStatus(Printing);
        else
            _printing_command=nullptr;
        _wait=false;
    }
}

PrintTask::~PrintTask()
{
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
    Task::Cancel();
}


void PrintTask::Repeat()
{
    _printing_command=nullptr;
    Task::Repeat();
}
