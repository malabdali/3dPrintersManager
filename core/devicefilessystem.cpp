#include "devicefilessystem.h"
#include "device.h"
#include "gcode/fileslist.h"
#include "gcode/uploadfile.h"
#include "gcode/deletefile.h"
#include "gcode/linenumber.h"
#include "utilities/loadfilefuture.h"
#include<algorithm>
#include <QUrl>


DeviceFilesSystem::DeviceFilesSystem(Device *device, QObject *parent): QObject(parent),_device(device)
{
    _sd_supported=false;
    _line_number=0;
    _uploading_file=nullptr;
    QObject::connect(device,&Device::ReadyFlagChanged,this,&DeviceFilesSystem::WhenDeviceReady);
    QObject::connect(device,&Device::PortClosed,this,&DeviceFilesSystem::WhenPortClosed);
    QObject::connect(device,&Device::CommandFinished,this,&DeviceFilesSystem::WhenCommandFinished);
    QObject::connect(device,&Device::DeviceStatsUpdated,this,&DeviceFilesSystem::WhenStatsUpdated);
}

bool DeviceFilesSystem::SdSupported() const
{
    return _sd_supported;
}

void DeviceFilesSystem::UpdateFileList()
{
    if(!_sd_supported) return;
    GCode::FilesList* command=new GCode::FilesList(this->_device);
    this->_device->AddGCodeCommand(command);
}

void DeviceFilesSystem::DeleteFile(const QByteArray &file)
{
    if(!_sd_supported) return;
    GCode::DeleteFile* deleteCommand=new GCode::DeleteFile(this->_device,[](bool)->void{},file);
    _device->AddGCodeCommand(deleteCommand);
}

void DeviceFilesSystem::UploadFile(QByteArray fileName)
{

    if(!_sd_supported) return;
    _failed_uploads.removeAll(fileName);
    if(_wait_for_upload.empty())
        UpdateLineNumber();
    _wait_for_upload.append(fileName);
}

bool DeviceFilesSystem::IsStillUploading()
{
    if(_uploading_file!=nullptr)
        return true;
    return false;
}

double DeviceFilesSystem::GetUploadProgress()
{
    if(this->IsStillUploading())
        return _uploading_file->GetProgress();
    return 0;
}

QList<QByteArray> DeviceFilesSystem::GetUploadedFiles()
{
    return _uploaded_files;
}

QList<QByteArray> DeviceFilesSystem::GetFailedUploads()
{
    return _failed_uploads;
}

QMap<QByteArray,size_t> DeviceFilesSystem::GetFileList(){
    return _files;
}

void DeviceFilesSystem::StopUpload(QByteArray ba)
{
    auto files=_wait_for_upload;
    if(_uploading_file!=nullptr && _uploading_file->GetFileName()==ba)
    {
        _device->ClearCommand(_uploading_file);
    }
    else{
        for(QByteArray& f:files){
            QByteArray name=QUrl(f).fileName().replace("gcode","GCO").toUpper().toUtf8();
            if(name==ba){
                _wait_for_upload.removeAll(f);
            }
        }
    }
}

QList<QByteArray> DeviceFilesSystem::GetWaitUploadingList()
{
    QList<QByteArray> ba;
    for(QByteArray& f:_wait_for_upload){
        auto name=QUrl(f).fileName().replace("gcode","GCO").toUpper().toUtf8();
        ba.append(name);
    }
    return ba;
}

uint64_t DeviceFilesSystem::GetLineNumber()
{
    return _line_number;
}

void DeviceFilesSystem::WhenDeviceReady(bool b)
{
    if(b && _files.size()==0)
    {
        UpdateFileList();
    }
}

void DeviceFilesSystem::WhenPortClosed()
{
}


void DeviceFilesSystem::WhenCommandFinished(GCodeCommand *command,bool b)
{
    if(dynamic_cast<GCode::UploadFile*>(command))
    {
        auto* uf=dynamic_cast<GCode::UploadFile*>(command);
        WhenFileUploaded(uf);
    }
    else if(dynamic_cast<GCode::DeleteFile*>(command)){
        if(command->IsSuccess())
        {
            auto* df=dynamic_cast<GCode::DeleteFile*>(command);
            this->_files.remove(df->GetFileName());
            emit FileDeleted(df->GetFileName());
            emit FileListUpdated();
        }
    }
    else if(dynamic_cast<GCode::FilesList*>(command)){
        auto* fs=dynamic_cast<GCode::FilesList*>(command);
        if(command->IsSuccess())
        {
            auto list=fs->GetFilesList();
            _files.clear();
            for(int i=0;i<list.size();i++)
                _files.insert(list.keys()[i],list.values()[i]);
            emit FileListUpdated();
        }
    }
    else if(dynamic_cast<GCode::LineNumber*>(command)){
        auto* ln=dynamic_cast<GCode::LineNumber*>(command);
        WhenLineNumberUpdated(ln);
    }
}

void DeviceFilesSystem::WhenStatsUpdated()
{
    if(_device->GetStats().contains("SDCARD")&&_device->GetStats()["SDCARD"]=="1"){
        SetSdSupported(true);
    }
    else{
        SetSdSupported(false);
    }
}

void DeviceFilesSystem::WhenLineNumberUpdated(GCode::LineNumber * lineNumber)
{
    if(lineNumber->IsSuccess())
    {
        _line_number=lineNumber->GetLineNumber();
        emit LineNumberUpdated(true);
        if(_wait_for_upload.length()>0){
            QString filename=_wait_for_upload[0];
            LoadFileFuture* lff=new LoadFileFuture(filename,[this,filename](QList<QByteArray> array)->void{
                QByteArray filename2=QUrl(filename).fileName().replace("gcode","GCO").toUpper().toUtf8();
                GCode::UploadFile* gcode=new GCode::UploadFile(this->_device,[](bool)->void{},filename2,array,
                    _line_number);
                this->_uploading_file=gcode;
                _device->AddGCodeCommand(gcode);
                emit FileListUpdated();
        },_line_number,this);
        }
    }
    else{
        _line_number=0;
        emit LineNumberUpdated(false);
    }
}

void DeviceFilesSystem::WhenFileUploaded(GCode::UploadFile *uploadFile)
{
    if(uploadFile->IsSuccess()){
        _files.insert(uploadFile->GetFileName(),uploadFile->GetSize());
        _uploaded_files.append(uploadFile->GetFileName());
        emit FileUploaded(uploadFile->GetFileName());
    }
    else
    {
        _failed_uploads.append(uploadFile->GetFileName());
        emit UploadFileFailed(uploadFile->GetFileName());
    }
    _wait_for_upload.erase(std::remove_if(_wait_for_upload.begin(),_wait_for_upload.end(),
                        [uploadFile](QByteArray ba)->bool{
                            return ba.toLower().contains(uploadFile->GetFileName().mid(0,uploadFile->GetFileName().indexOf(".")).toLower());
                        }),_wait_for_upload.end());
    _uploading_file=nullptr;
    emit this->FileListUpdated();
    UpdateLineNumber();
}

void DeviceFilesSystem::SetSdSupported(bool b){

    _sd_supported=b;
}

void DeviceFilesSystem::CallFunction(const char* function)
{
    this->metaObject()->invokeMethod(this,function);
}

void DeviceFilesSystem::UpdateLineNumber()
{
    if(!_sd_supported) return;
    GCode::LineNumber* gcode=new GCode::LineNumber(this->_device);
    _device->AddGCodeCommand(gcode);
}

