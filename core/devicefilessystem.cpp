#include "devicefilessystem.h"
#include "device.h"
#include "gcode/fileslist.h"
#include "gcode/uploadfile.h"
#include "gcode/deletefile.h"


DeviceFilesSystem::DeviceFilesSystem(Device *device, QObject *parent): QObject(parent),_device(device)
{
    _sd_supported=false;
    QObject::connect(device,&Device::ReadyFlagChanged,this,&DeviceFilesSystem::WhenDeviceReady);
}

bool DeviceFilesSystem::SdSupported() const
{
    return _sd_supported;
}

void DeviceFilesSystem::UpdateFileList()
{
    if(!_sd_supported) return;
    qDebug()<<"UpdateFileList";
    GCode::FilesList* command=new GCode::FilesList(this->_device,[this](bool success,QByteArrayList result,QList<uint32_t> sizes)->void{
        if(success)
        {
            _mutex.lock();
            _files.clear();
            for(int i=0;i<result.length();i++)
                _files.insert(result[i],sizes[i]);
            _mutex.unlock();
            emit FileListUpdated();
        }
    });
    this->_device->AddGCodeCommand(command);
}

void DeviceFilesSystem::DeleteFile(const QByteArray &file)
{
    if(!_sd_supported) return;
    GCode::DeleteFile* deleteCommand=new GCode::DeleteFile(this->_device,[file,this](bool b)->void{
        _mutex.lock();
        this->_files.remove(file);
        _mutex.unlock();
        emit FileDeleted(file);
        emit FileListUpdated();
    },file);
    _device->AddGCodeCommand(deleteCommand);
}

void DeviceFilesSystem::UploadFile(QByteArray fileName, const QByteArray &data)
{
    QByteArrayList lines=data.split('\n');
    lines.erase(std::remove_if(lines.begin(),lines.end(),[](QByteArray& line)->bool{
        return !line.startsWith("M")&& !line.startsWith("G");
    }),lines.end());
    int i=1;
    for(QByteArray& line:lines){
        int index=line.indexOf(';');
        line.remove(index,line.length()-index);
        line.replace('\r',"");
        line=line.simplified();
        line=line.trimmed();
        line.prepend(QByteArray("N")+QByteArray::number(i)+" ");
        uint8_t checksum=std::accumulate(line.begin(),line.end(),0,[](uint8_t v,char c){return c^v;});
        line.append(QByteArray("*")+QByteArray::number(checksum)+"\n");
        i++;
    }
    _mutex.lock();
    _failed_uploads.removeAll(fileName);
    _mutex.unlock();
    GCode::UploadFile* gcode=new GCode::UploadFile(this->_device,[this,fileName,data](bool success)->void{
        _uploading_files.removeAt(0);
        if(success){
            qDebug()<<"upload finished and success";
            _mutex.lock();
            _files.insert(fileName,data.length());
            _uploaded_files.append(fileName);
            _mutex.unlock();
            emit FileUploaded(fileName);
            emit FileListUpdated();
        }
        else{
            _mutex.lock();
            _failed_uploads.append(fileName);
            _mutex.unlock();
            emit UploadFileFailed(fileName);
            emit FileListUpdated();
        }
        if(_device->ReopenPort())
            qDebug()<<"reopen port";
    },fileName,lines);
    //if(_uploading_files.length()==0)
    _uploading_files.append(gcode);
    _device->AddGCodeCommand(gcode);
    emit WaitListUpdated();
}

bool DeviceFilesSystem::IsStillUploading()
{
    if(_uploading_files.length()>0)
        return true;
    return false;
}

double DeviceFilesSystem::GetUploadProgress()
{
    if(this->IsStillUploading())
        return _uploading_files[0]->GetProgress();
    return 0;
}

QList<QByteArray> DeviceFilesSystem::GetUploadedFiles()
{
    QMutexLocker locker(&_mutex);
    return _uploaded_files;
}

QList<QByteArray> DeviceFilesSystem::GetFailedUploads()
{
    QMutexLocker locker(&_mutex);
    return _failed_uploads;
}

QMap<QByteArray,size_t> DeviceFilesSystem::GetFileList(){
    QMutexLocker locker(&_mutex);
    return _files;
}

void DeviceFilesSystem::StopUpload(QByteArray ba)
{
    for(GCode::UploadFile* f:_uploading_files){
        if(f->GetFileName()==ba){
            _device->ClearCommand(f);
            _uploading_files.removeAll(f);
            qDebug()<<"is stopped";
        }
    }
}

QList<QByteArray> DeviceFilesSystem::GetWaitUploadingList() const
{
    QList<QByteArray> ba;
    for(GCode::UploadFile* f:_uploading_files){
        ba.append(f->GetFileName());
    }
    return ba;
}

void DeviceFilesSystem::WhenDeviceReady(bool b)
{
    qDebug()<<"is ready : "<<b;
    if(b && _files.size()==0)
    {
        qDebug()<<"call update files";
        UpdateFileList();
    }
}

void DeviceFilesSystem::SetSdSupported(bool b){

    _sd_supported=b;
}

void DeviceFilesSystem::CallFunction(const char* function)
{
    this->metaObject()->invokeMethod(this,function);
}

