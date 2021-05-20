#include "devicefilessystem.h"
#include "device.h"
#include "gcode/fileslist.h"
#include "gcode/uploadfile.h"
#include "gcode/deletefile.h"
#include "gcode/linenumber.h"
#include "utilities/loadprintablefuture.h"
#include<algorithm>
#include <QUrl>
#include "../config.h"
#include <QDir>
#include "deviceinfo.h"
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrent>


DeviceFilesSystem::DeviceFilesSystem(Device *device, QByteArray extension): DeviceComponent(device),_extension(extension)
{
    _sd_supported=false;
    _line_number=0;
    _load_file=nullptr;
    _upload_speed=0;
}

void DeviceFilesSystem::Setup()
{
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
    GCode::DeleteFile* deleteCommand=new GCode::DeleteFile(this->_device,file);
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


void DeviceFilesSystem::SetUploadSpeed(unsigned speed)
{
    _upload_speed=speed;
}

unsigned DeviceFilesSystem::GetUploadSpeed()
{
    return _upload_speed;
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


void DeviceFilesSystem::Stop()
{
    for(QByteArray ba :_wait_for_upload)
        StopUpload(ba);
}

QList<QByteArray> DeviceFilesSystem::GetWaitUploadingList()
{
    QList<QByteArray> ba;
    for(QByteArray& f:_wait_for_upload){
        auto name=QUrl(f).fileName().replace(QRegularExpression(R"(\.\w+$)"),"."+_extension).toUtf8();
        ba.append(name);
    }
    return ba;
}

uint64_t DeviceFilesSystem::GetLineNumber()
{
    return _line_number;
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

QByteArray DeviceFilesSystem::GetFileExtension()
{
    return _extension;
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
}


void DeviceFilesSystem::SetSdSupported(bool b){
    bool old=_sd_supported;
    _sd_supported=b;
    if(old!=_sd_supported)
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

