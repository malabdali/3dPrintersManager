#include "slafilessystem.h"

#include "../devicefilessystem.h"
#include "sladevice.h"
#include "../utilities/loadprintablefuture.h"
#include<algorithm>
#include <QUrl>
#include "../../config.h"
#include <QDir>
#include "../deviceinfo.h"
#include "../devicefilessystem.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

SLAFilesSystem::SLAFilesSystem(Device *dev):DeviceFilesSystem(dev,"ctb")
{
    _uploading_file=nullptr;
}



void SLAFilesSystem::Setup()
{

    QObject::connect(_device,&Device::StatusChanged,this,&SLAFilesSystem::WhenDeviceStatusChanged);
    QObject::connect(_device,&Device::Closed,this,&SLAFilesSystem::WhenPortClosed);
    QObject::connect(_device,&Device::CommandFinished,this,&SLAFilesSystem::WhenCommandFinished);
    QObject::connect(_device,&Device::DeviceStatsUpdated,this,&SLAFilesSystem::WhenStatsUpdated);
    DeviceFilesSystem::Setup();
}

void SLAFilesSystem::Disable()
{

    QObject::disconnect(_device,&Device::StatusChanged,this,&SLAFilesSystem::WhenDeviceStatusChanged);
    QObject::disconnect(_device,&Device::Closed,this,&SLAFilesSystem::WhenPortClosed);
    QObject::disconnect(_device,&Device::CommandFinished,this,&SLAFilesSystem::WhenCommandFinished);
    QObject::disconnect(_device,&Device::DeviceStatsUpdated,this,&SLAFilesSystem::WhenStatsUpdated);
    DeviceFilesSystem::Disable();
}

void SLAFilesSystem::UpdateFileList()
{
    GCode::Chitu::ChituFilesList* command=new GCode::Chitu::ChituFilesList(this->_device);
    this->_device->AddGCodeCommand(command);
}

void SLAFilesSystem::DeleteFile(const QByteArray &file)
{
    GCode::Chitu::ChituDeleteFile* deleteCommand=new GCode::Chitu::ChituDeleteFile(this->_device,file);
    _device->AddGCodeCommand(deleteCommand);
}

void SLAFilesSystem::UploadFile(QByteArray fileName)
{

    _failed_uploads.removeAll(fileName);
    if(!fileName.isEmpty())
    _wait_for_upload.append(fileName);
    if(_wait_for_upload.length()==1 && !IsStillUploading())
    {
        qDebug()<<"SLAFilesSystem::UploadFile 1";
        QString filename=_wait_for_upload[0];
        QByteArray filename2=QUrl(filename).fileName().replace(QRegularExpression(R"(\.\w+$)"),"."+_extension).toUtf8();
        if(_files.contains(FileInfo(filename2)))
        {
            qDebug()<<"SLAFilesSystem::UploadFile 2";
            DeleteLocaleFile("files/"+filename2);
            _files.removeAll(FileInfo(filename2));
            emit FileListUpdated();
        }
        _load_file=new LoadPrintableFuture(filename,[this,filename](QList<QByteArray> array,QByteArray data)->void{
            qDebug()<<"SLAFilesSystem::UploadFile 3";
            _load_file=nullptr;
            _uploading_file_content=data;
            QByteArray filename2=QUrl(filename).fileName().replace(QRegularExpression(R"(\.\w+$)"),"."+_extension).toUtf8();
            GCode::Chitu::ChituUploadFile* gcode=new GCode::Chitu::ChituUploadFile(this->_device,filename2,array,_upload_speed);
            this->_uploading_file=gcode;
            qDebug()<<filename2;
            _device->AddGCodeCommand(gcode);
            emit FileListUpdated();
        },0,false,0x500,this);
    }
}

void SLAFilesSystem::UpdateLineNumber()
{
}

void SLAFilesSystem::UploadNext()
{
    if(_wait_for_upload.length()>0 && !IsStillUploading())
    {
        qDebug()<<"SLAFilesSystem::UploadFile 1";
        QString filename=_wait_for_upload[0];
        QByteArray filename2=QUrl(filename).fileName().replace(QRegularExpression(R"(\.\w+$)"),"."+_extension).toUtf8();
        if(_files.contains(FileInfo(filename2)))
        {
            qDebug()<<"SLAFilesSystem::UploadFile 2";
            DeleteLocaleFile("files/"+filename2);
            _files.removeAll(FileInfo(filename2));
            emit FileListUpdated();
        }
        _load_file=new LoadPrintableFuture(filename,[this,filename](QList<QByteArray> array,QByteArray data)->void{
            qDebug()<<"SLAFilesSystem::UploadFile 3";
            _load_file=nullptr;
            _uploading_file_content=data;
            QByteArray filename2=QUrl(filename).fileName().replace(QRegularExpression(R"(\.\w+$)"),"."+_extension).toUtf8();
            GCode::Chitu::ChituUploadFile* gcode=new GCode::Chitu::ChituUploadFile(this->_device,filename2,array,_upload_speed);
            this->_uploading_file=gcode;
            qDebug()<<filename2;
            _device->AddGCodeCommand(gcode);
            emit FileListUpdated();
        },0,false,0x500,this);
    }
}


void SLAFilesSystem::WhenDeviceStatusChanged(Device::DeviceStatus status)
{
    if(status==Device::DeviceStatus::Ready && _files.size()==0 )
    {
        UpdateFileList();
    }
}

void SLAFilesSystem::WhenPortClosed()
{

}

void SLAFilesSystem::WhenCommandFinished(GCodeCommand *command,bool b)
{
    if(dynamic_cast<GCode::Chitu::ChituUploadFile*>(command))
    {
        auto* uf=dynamic_cast<GCode::Chitu::ChituUploadFile*>(command);
        WhenFileUploaded(uf);
    }
    else if(dynamic_cast<GCode::Chitu::ChituDeleteFile*>(command))
    {
        if(command->IsSuccess())
        {
            auto* df=dynamic_cast<GCode::Chitu::ChituDeleteFile*>(command);
            this->DeleteLocaleFile("files/"+df->GetFileName());

            _files.erase(std::remove_if(_files.begin(),_files.end(),[fn=df->GetFileName()](const FileInfo& fi)->bool{return fi.GetFileName()==fn;}));
            emit FileDeleted(df->GetFileName());
            emit FileListUpdated();
        }
    }
    else if(dynamic_cast<GCode::Chitu::ChituFilesList*>(command)){
        auto* fs=dynamic_cast<GCode::Chitu::ChituFilesList*>(command);
        if(command->IsSuccess())
        {
            WhenFileListUpdated(fs);
        }
    }
}

void SLAFilesSystem::WhenStatsUpdated()
{
    bool old=_sd_supported;

    if(!old)
    {
        SetSdSupported(true);
    }
    _sd_supported=true;
}



void SLAFilesSystem::WhenFileUploaded(GCode::Chitu::ChituUploadFile *uploadFile)
{
    _uploading_file=nullptr;
    auto res=std::find_if(_wait_for_upload.begin(),_wait_for_upload.end(),[uploadFile](QByteArray ba)->bool{
            return ba.toLower().contains(uploadFile->GetFileName().mid(0,uploadFile->GetFileName().indexOf(".")).toLower());
    });
    if(uploadFile->IsSuccess()){
        DeleteLocaleFile("files/"+uploadFile->GetFileName());
        SaveLocaleFile("files/"+uploadFile->GetFileName(),_uploading_file_content,[res,fileName=uploadFile->GetFileName(),this](bool success){
            if(success)
            {
                //uploaded_files.append(fileName);
                FileInfo fi(fileName);
                fi.SetLocalePath(GetLocaleDirectory(LOCALE_GCODE_PATH+fileName));
                fi.SetIsUploaded(true);
                _files.removeAll(fi);
                _files.append(fi);
                emit FileUploaded(fileName);
            }
            else{
                _failed_uploads.append(fileName);
                emit UploadFileFailed(fileName);
            }
            _wait_for_upload.erase(res);
            _uploading_file_content.clear();
            emit this->FileListUpdated();
            UploadNext();
        });
    }
    else
    {
        _uploading_file_content.clear();
        _failed_uploads.append(uploadFile->GetFileName());
        emit UploadFileFailed(uploadFile->GetFileName());
        _wait_for_upload.erase(res);
        emit this->FileListUpdated();
        UploadNext();
    }
}


void SLAFilesSystem::WhenFileListUpdated(GCode::Chitu::ChituFilesList * fs)
{
    qDebug()<<"SLAFilesSystem::WhenFileListUpdated";
    auto list=fs->GetFilesList();
    _files.clear();
    QStringList sl=this->GetLocaleFiles(LOCALE_GCODE_PATH,"."+_extension);
    for(int i=0;i<list.size();i++)
    {
        if(_files.contains(FileInfo(list.keys()[i])))
            continue;
        FileInfo fi=FileInfo(list.keys()[i]);
        if(sl.filter(fi.GetFileName(),Qt::CaseInsensitive).length()>0){
            fi.SetIsUploaded(true);
            fi.SetUploadPercent(100.0);
            fi.SetLocalePath(GetLocaleDirectory(LOCALE_GCODE_PATH+sl.filter(fi.GetFileName(),Qt::CaseInsensitive)[0].toUtf8()));
        }
        _files.append(fi);
    }
    emit FileListUpdated();
}


bool SLAFilesSystem::SdSupported() const
{
    return _sd_supported;
}

bool SLAFilesSystem::IsStillUploading()
{
    if(_uploading_file!=nullptr)
        return true;
    return false;
}

double SLAFilesSystem::GetUploadProgress()
{
    if(this->IsStillUploading())
        return _uploading_file->GetProgress();
    return 0;
}

void SLAFilesSystem::StopUpload(QByteArray ba)
{
    ba=QUrl(ba).fileName().replace(QRegularExpression(R"(\.\w+$)"),"."+_extension).toUtf8();
    auto files=_wait_for_upload;
    if(_uploading_file!=nullptr && _uploading_file->GetFileName()==ba)
    {
        _device->ClearCommand(_uploading_file);
    }
    else{
        for(QByteArray& f:files){
            QByteArray name=QUrl(f).fileName().replace(QRegularExpression(R"(\.\w+$)"),"."+_extension).toUtf8();
            if(name==ba){
                _wait_for_upload.removeAll(f);
            }
        }
    }
}
