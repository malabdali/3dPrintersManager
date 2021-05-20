#include "fdmfilessystem.h"

#include "../devicefilessystem.h"
#include "../device.h"
#include "../gcode/fileslist.h"
#include "../gcode/uploadfile.h"
#include "../gcode/deletefile.h"
#include "../gcode/linenumber.h"
#include "../utilities/loadprintablefuture.h"
#include<algorithm>
#include <QUrl>
#include "../../config.h"
#include <QDir>
#include "../deviceinfo.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>

FDMFilesSystem::FDMFilesSystem(Device *dev):DeviceFilesSystem(dev,"G")
{
    _uploading_file=nullptr;
}



void FDMFilesSystem::Setup()
{

    QObject::connect(_device,&Device::StatusChanged,this,&FDMFilesSystem::WhenDeviceStatusChanged);
    QObject::connect(_device,&Device::Closed,this,&FDMFilesSystem::WhenPortClosed);
    QObject::connect(_device,&Device::CommandFinished,this,&FDMFilesSystem::WhenCommandFinished);
    QObject::connect(_device,&Device::DeviceStatsUpdated,this,&FDMFilesSystem::WhenStatsUpdated);
    DeviceFilesSystem::Setup();
}

void FDMFilesSystem::Disable()
{

    QObject::disconnect(_device,&Device::StatusChanged,this,&FDMFilesSystem::WhenDeviceStatusChanged);
    QObject::disconnect(_device,&Device::Closed,this,&FDMFilesSystem::WhenPortClosed);
    QObject::disconnect(_device,&Device::CommandFinished,this,&FDMFilesSystem::WhenCommandFinished);
    QObject::disconnect(_device,&Device::DeviceStatsUpdated,this,&FDMFilesSystem::WhenStatsUpdated);
    DeviceFilesSystem::Disable();
}

void FDMFilesSystem::UpdateFileList()
{
    DeviceFilesSystem::UpdateFileList();
}

void FDMFilesSystem::DeleteFile(const QByteArray &file)
{
    DeviceFilesSystem::DeleteFile(file);
}

void FDMFilesSystem::UploadFile(QByteArray fileName)
{
    DeviceFilesSystem::UploadFile(fileName);
}

void FDMFilesSystem::UpdateLineNumber()
{
    DeviceFilesSystem::UpdateLineNumber();
}


void FDMFilesSystem::WhenDeviceStatusChanged(Device::DeviceStatus status)
{
    if(status==Device::DeviceStatus::Ready && _files.size()==0 )
    {
        UpdateFileList();
    }
}

void FDMFilesSystem::WhenPortClosed()
{

}

void FDMFilesSystem::WhenCommandFinished(GCodeCommand *command,bool b)
{
    if(dynamic_cast<GCode::UploadFile*>(command))
    {
        auto* uf=dynamic_cast<GCode::UploadFile*>(command);
        WhenFileUploaded(uf);
    }
    else if(dynamic_cast<GCode::DeleteFile*>(command))
    {
        if(command->IsSuccess())
        {
            auto* df=dynamic_cast<GCode::DeleteFile*>(command);
            this->DeleteLocaleFile("files/"+df->GetFileName());

            _files.erase(std::remove_if(_files.begin(),_files.end(),[fn=df->GetFileName()](const FileInfo& fi)->bool{return fi.GetFileName()==fn;}));
            emit FileDeleted(df->GetFileName());
            emit FileListUpdated();
        }
    }
    else if(dynamic_cast<GCode::FilesList*>(command)){
        auto* fs=dynamic_cast<GCode::FilesList*>(command);
        if(command->IsSuccess())
        {
            WhenFileListUpdated(fs);
        }
    }
    else if(dynamic_cast<GCode::LineNumber*>(command)){
        auto* ln=dynamic_cast<GCode::LineNumber*>(command);
        WhenLineNumberUpdated(ln);
    }
}

void FDMFilesSystem::WhenStatsUpdated()
{
    if(_device->GetStats().contains("SDCARD")&&_device->GetStats()["SDCARD"]=="1"){
        bool old=_sd_supported;
        SetSdSupported(true);
        if(!old && _files.size()==0 )
        {
        }
    }
    else{
        SetSdSupported(false);
    }
}

void FDMFilesSystem::WhenLineNumberUpdated(GCode::LineNumber * lineNumber)
{
    if(lineNumber->IsSuccess())
    {
        _line_number=lineNumber->GetLineNumber();
        emit LineNumberUpdated(true);
        if(_wait_for_upload.length()>0){
            QString filename=_wait_for_upload[0];
            QByteArray filename2=QUrl(filename).fileName().replace(QRegularExpression(R"(\.\w+$)"),"."+_extension).toUpper().toUtf8();
            if(_files.contains(FileInfo(filename2)))
            {
                DeleteLocaleFile("files/"+filename2);
                _files.removeAll(FileInfo(filename2));
                emit FileListUpdated();
            }
            _load_file=new LoadPrintableFuture(filename,[this,filename](QList<QByteArray> array,QByteArray data)->void{
                _load_file=nullptr;
                _uploading_file_content=data;
                QByteArray filename2=QUrl(filename).fileName().replace(QRegularExpression(R"(\.\w+$)"),"."+_extension).toUpper().toUtf8();
                GCode::UploadFile* gcode=new GCode::UploadFile(this->_device,filename2,array,_line_number,_upload_speed);
                this->_uploading_file=gcode;
                _device->AddGCodeCommand(gcode);
                emit FileListUpdated();
            },_line_number,true,0,this);
        }
    }
    else{
        _line_number=0;
        emit LineNumberUpdated(false);
        if(_wait_for_upload.length()>0)
        {
            auto wfu=_wait_for_upload;
            for(QByteArray& ba:wfu)
            {
                StopUpload(ba);
                _failed_uploads.append(QUrl(ba).fileName().replace(QRegularExpression(R"(\.\w+$)"),"."+_extension).toUpper().toUtf8());

                emit UploadFileFailed(QUrl(ba).fileName().replace(QRegularExpression(R"(\.\w+$)"),"."+_extension).toUpper().toUtf8());
            }
        }
    }
}

void FDMFilesSystem::WhenFileUploaded(GCode::UploadFile *uploadFile)
{
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
            _uploading_file=nullptr;
            _uploading_file_content.clear();
            emit this->FileListUpdated();
            UpdateLineNumber();
        });
    }
    else
    {
        _uploading_file_content.clear();
        _failed_uploads.append(uploadFile->GetFileName());
        emit UploadFileFailed(uploadFile->GetFileName());
        _wait_for_upload.erase(res);
        _uploading_file=nullptr;
        emit this->FileListUpdated();
        UpdateLineNumber();
    }
}

void FDMFilesSystem::WhenFileListUpdated(GCode::FilesList* fs)
{
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


bool FDMFilesSystem::IsStillUploading()
{
    if(_uploading_file!=nullptr)
        return true;
    return false;
}

double FDMFilesSystem::GetUploadProgress()
{
    if(this->IsStillUploading())
        return _uploading_file->GetProgress();
    return 0;
}

void FDMFilesSystem::StopUpload(QByteArray ba)
{
    ba=QUrl(ba).fileName().replace(QRegularExpression(R"(\.\w+$)"),"."+_extension).toUpper().toUtf8();
    auto files=_wait_for_upload;
    if(_uploading_file!=nullptr && _uploading_file->GetFileName()==ba)
    {
        _device->ClearCommand(_uploading_file);
    }
    else{
        for(QByteArray& f:files){
            QByteArray name=QUrl(f).fileName().replace(QRegularExpression(R"(\.\w+$)"),"."+_extension).toUpper().toUtf8();
            if(name==ba){
                _wait_for_upload.removeAll(f);
            }
        }
    }
}
