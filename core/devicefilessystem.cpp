#include "devicefilessystem.h"
#include "device.h"
#include "gcode/fileslist.h"
#include "gcode/uploadfile.h"
#include "gcode/deletefile.h"
#include "gcode/linenumber.h"
#include "utilities/loadgcodefuture.h"
#include<algorithm>
#include <QUrl>
#include "../config.h"
#include <QDir>
#include "deviceinfo.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>


DeviceFilesSystem::DeviceFilesSystem(Device *device): DeviceComponent(device)
{
    _sd_supported=false;
    _line_number=0;
    _uploading_file=nullptr;
    _load_file=nullptr;
    _upload_speed=0;
}

void DeviceFilesSystem::Setup()
{
    QObject::connect(_device,&Device::StatusChanged,this,&DeviceFilesSystem::WhenDeviceReady);
    QObject::connect(_device,&Device::PortClosed,this,&DeviceFilesSystem::WhenPortClosed);
    QObject::connect(_device,&Device::CommandFinished,this,&DeviceFilesSystem::WhenCommandFinished);
    QObject::connect(_device,&Device::DeviceStatsUpdated,this,&DeviceFilesSystem::WhenStatsUpdated);
    connect(_device,&Device::BeforeSaveDeviceData,this,&DeviceFilesSystem::Save);
    connect(_device,&Device::DeviceDataLoaded,this,&DeviceFilesSystem::Load);
}

void DeviceFilesSystem::Initiate()
{
    QDir dir;
    if(!dir.exists(QStringLiteral(PRINTERS_FOLDER_PATH)+"/"+this->_device->GetDeviceInfo()->GetDeviceName()))
    {
        dir.mkpath(GetLocaleDirectory());
        dir.mkpath(GetLocaleDirectory(LOCALE_GCODE_PATH));
    }
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
    if(!_sd_supported){
        _failed_uploads.append(fileName);
        emit UploadFileFailed(fileName);
    }
    _failed_uploads.removeAll(fileName);
    _wait_for_upload.append(fileName);
    if(_wait_for_upload.length()==1)
    {
        UpdateLineNumber();
    }
}

bool DeviceFilesSystem::IsStillUploading()
{
    if(_uploading_file!=nullptr)
        return true;
    return false;
}

void DeviceFilesSystem::SetUploadSpeed(unsigned speed)
{
    _upload_speed=speed;
}

unsigned DeviceFilesSystem::GetUploadSpeed()
{
    return _upload_speed;
}

double DeviceFilesSystem::GetUploadProgress()
{
    if(this->IsStillUploading())
        return _uploading_file->GetProgress();
    return 0;
}

FileInfo DeviceFilesSystem::GetUploadedFileInfo(const QByteArray &ba)
{
    int i=_files.indexOf(FileInfo(ba));
    if(i>-1)
        return _files[i];
    return FileInfo("");
}


QList<QByteArray> DeviceFilesSystem::GetFailedUploads()
{
    return _failed_uploads;
}

QList<FileInfo>& DeviceFilesSystem::GetFileList()
{
    return _files;
}

void DeviceFilesSystem::StopUpload(QByteArray ba)
{
    ba=QUrl(ba).fileName().replace(".gcode","."+QStringLiteral(UPLOAD_SUFFIX)).toUpper().toUtf8();
    auto files=_wait_for_upload;
    if(_uploading_file!=nullptr && _uploading_file->GetFileName()==ba)
    {
        _device->ClearCommand(_uploading_file);
    }
    else{
        for(QByteArray& f:files){
            QByteArray name=QUrl(f).fileName().replace(".gcode","."+QStringLiteral(UPLOAD_SUFFIX)).toUpper().toUtf8();
            if(name==ba){
                _wait_for_upload.removeAll(f);
            }
        }
    }
}

void DeviceFilesSystem::Stop()
{
    for(QByteArray ba :_wait_for_upload)
        StopUpload(ba);
}

QList<QByteArray> DeviceFilesSystem::GetWaitUploadingList()
{
    QList<QByteArray> ba;
    for(QByteArray& f:_wait_for_upload){
        auto name=QUrl(f).fileName().replace(".gcode","."+QStringLiteral(UPLOAD_SUFFIX)).toUpper().toUtf8();
        ba.append(name);
    }
    return ba;
}

uint64_t DeviceFilesSystem::GetLineNumber()
{
    return _line_number;
}

void DeviceFilesSystem::WhenDeviceReady(Device::DeviceStatus status)
{
    if(status==Device::DeviceStatus::Ready && _files.size()==0 )
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

void DeviceFilesSystem::WhenStatsUpdated()
{
    if(_device->GetStats().contains("SDCARD")&&_device->GetStats()["SDCARD"]=="1"){
        bool old=_sd_supported;
        SetSdSupported(true);
        if(!old && _files.size()==0 )
        {
            UpdateFileList();
        }
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
            QByteArray filename2=QUrl(filename).fileName().replace(".gcode","."+QStringLiteral(UPLOAD_SUFFIX)).toUpper().toUtf8();
            if(_files.contains(FileInfo(filename2)))
            {
                DeleteLocaleFile("files/"+filename2);
                _files.removeAll(FileInfo(filename2));
                emit FileListUpdated();
            }
            _load_file=new LoadGCodeFuture(filename,[this,filename](QList<QByteArray> array,QByteArray data)->void{
                _load_file=nullptr;
                _uploading_file_content=data;
                QByteArray filename2=QUrl(filename).fileName().replace(".gcode","."+QStringLiteral(UPLOAD_SUFFIX)).toUpper().toUtf8();
                GCode::UploadFile* gcode=new GCode::UploadFile(this->_device,filename2,array,_line_number,_upload_speed);
                this->_uploading_file=gcode;
                _device->AddGCodeCommand(gcode);
                emit FileListUpdated();
            },_line_number,this);
        }
    }
    else{
        _line_number=0;
        emit LineNumberUpdated(false);
        if(_wait_for_upload.length()>0)
        {
            for(QByteArray& ba:_wait_for_upload)
            {
                StopUpload(ba);
                _failed_uploads.append(QUrl(ba).fileName().replace(".gcode","."+QStringLiteral(UPLOAD_SUFFIX)).toUpper().toUtf8());

                emit UploadFileFailed(QUrl(ba).fileName().replace(".gcode","."+QStringLiteral(UPLOAD_SUFFIX)).toUpper().toUtf8());
            }
        }
    }
}

void DeviceFilesSystem::WhenFileUploaded(GCode::UploadFile *uploadFile)
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

void DeviceFilesSystem::WhenFileListUpdated(GCode::FilesList* fs)
{
    auto list=fs->GetFilesList();
    _files.clear();
    QStringList sl=this->GetLocaleFiles(LOCALE_GCODE_PATH,"."+QByteArray(UPLOAD_SUFFIX));
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

void DeviceFilesSystem::SetSdSupported(bool b){
    _sd_supported=b;
    SdSupportChanged(b);
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

void DeviceFilesSystem::SaveLocaleFile(const QString &path, const QByteArray &data, std::function<void (bool)> callback)
{
    QString path2=GetLocaleDirectory(path.toUtf8());
    QDir dir(path2.mid(0,path2.lastIndexOf("/")));
    if(!dir.exists())
    {
        dir.mkpath(".");
    }
    QFutureWatcher<bool>* fw=new QFutureWatcher<bool>(this);
    QObject::connect(fw,&QFutureWatcher<bool>::finished,[fw,callback]{
        fw->deleteLater();
        callback(fw->result());
    });
    QFuture<bool> _future=QtConcurrent::run([data,path=path2]()->bool{
        QFile file(path);
        if(file.open(QIODevice::WriteOnly))
        {
            file.write(data);
            while(file.waitForBytesWritten(100));
            return true;
        }
        else{
            return false;
        }
    });
    fw->setFuture(_future);
}

bool DeviceFilesSystem::DeleteLocaleFile(const QByteArray &path)
{
    QString path2=GetLocaleDirectory(path);
    QDir dir;
    return dir.remove(path2);
}

void DeviceFilesSystem::ReadLocaleFile(const QByteArray &path, std::function<void (QByteArray)> callback)
{
    QString path2=GetLocaleDirectory(path);
    QFutureWatcher<QByteArray>* fw=new QFutureWatcher<QByteArray>(this);
    QObject::connect(fw,&QFutureWatcher<bool>::finished,[fw,callback]{
        fw->deleteLater();
        callback(fw->result());
    });
    QFuture<QByteArray> _future=QtConcurrent::run([path2]()->QByteArray{
                                                      QFile file(path2);
                                                      if(file.open(QIODevice::ReadOnly))
                                                      {
                                                          QByteArray ba=file.readAll();
                                                          file.close();
                                                          return ba;
                                                      }
                                                      else{
                                                          return QByteArray();
                                                      }
                                                  });
    fw->setFuture(_future);
}

void DeviceFilesSystem::CopyLocaleFile(const QByteArray &fpath, const QByteArray &tpath, std::function<void (bool)> callback)
{
    QString path2=GetLocaleDirectory(tpath);
    QFutureWatcher<bool>* fw=new QFutureWatcher<bool>(this);
    QObject::connect(fw,&QFutureWatcher<bool>::finished,[fw,callback]{
        fw->deleteLater();
        callback(fw->result());
    });
    QFuture<bool> _future=QtConcurrent::run([path2,fpath]()->bool{
        return QFile::copy(fpath,path2);
    });
    fw->setFuture(_future);
}

QStringList DeviceFilesSystem::GetLocaleFiles(const QByteArray &path, const QByteArray &suffix)
{
    QDir dir(GetLocaleDirectory(path));
    dir.setFilter(QDir::Files);
    QStringList sl=dir.entryList();
    auto it=std::remove_if(sl.begin(),sl.end(),[suffix](QString name)->bool{return !name.contains(suffix);});
    if(it!=sl.end()){
        sl.erase(it,sl.end());
    }
    return sl;
}

QByteArray DeviceFilesSystem::GetLocaleDirectory(const QByteArray& subdir){
    QByteArray sub="";
    if(!subdir.isEmpty())
        sub="/"+subdir;
    return (QStringLiteral(PRINTERS_FOLDER_PATH)+"/"+this->_device->GetDeviceInfo()->GetDeviceName()).toUtf8()+sub;
}

QJsonDocument DeviceFilesSystem::ToJson()
{
    QVariantHash vh;
    vh.insert("IS_UPLOADING",this->IsStillUploading());
    vh.insert("UPLOAD_PROGRESS",this->GetUploadProgress());
    vh.insert("SD_SUPPORTED",this->SdSupported());
    vh.insert("UPLOAD_SPEED",_upload_speed);
    return QJsonDocument(QJsonObject::fromVariantHash(vh));
}

void DeviceFilesSystem::FromJson(QJsonDocument json)
{
    if(json.object().contains("UPLOAD_SPEED")){
        _upload_speed=json.object()["UPLOAD_SPEED"].toInt();
    }
}

void DeviceFilesSystem::Save()
{
    _device->AddData("FS",ToJson().object());
}

void DeviceFilesSystem::Load()
{
    QJsonDocument documetn=QJsonDocument(_device->GetData("FS"));
    FromJson(documetn);
}

void DeviceFilesSystem::Disable()
{
    if(_load_file!=nullptr)
        _load_file->Stop();
    Stop();
    QObject::disconnect(_device,&Device::StatusChanged,this,&DeviceFilesSystem::WhenDeviceReady);
    QObject::disconnect(_device,&Device::PortClosed,this,&DeviceFilesSystem::WhenPortClosed);
    QObject::disconnect(_device,&Device::CommandFinished,this,&DeviceFilesSystem::WhenCommandFinished);
    QObject::disconnect(_device,&Device::DeviceStatsUpdated,this,&DeviceFilesSystem::WhenStatsUpdated);
}
