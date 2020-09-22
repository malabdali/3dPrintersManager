#include "devicefilessystem.h"
#include "device.h"
#include "gcode/fileslist.h"
#include "gcode/uploadfile.h"
#include "gcode/deletefile.h"


DeviceFilesSystem::DeviceFilesSystem(Device *device, QObject *parent): QObject(parent),_device(device)
{
    _sd_supported=false;
}

bool DeviceFilesSystem::SdSupported() const
{
    return _sd_supported;
}

void DeviceFilesSystem::UpdateFileList()
{
    if(!_sd_supported) return;
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
        emit UpdateFileList();
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
    GCode::UploadFile* gcode=new GCode::UploadFile(this->_device,[this,fileName,data](bool success)->void{
        if(success){
            _mutex.lock();
            _files.insert(fileName,data.length());
            _mutex.unlock();
            emit FileUploaded(fileName);
            emit UpdateFileList();
        }
        else{
            emit UploadFileFailed();
        }
    },fileName,lines);
    _device->AddGCodeCommand(gcode);
}

bool DeviceFilesSystem::IsStillUploading()
{
    if(dynamic_cast<GCode::UploadFile*>(_device->GetCurrentCommand()))
        return true;
    return false;
}

QMap<QByteArray,size_t> DeviceFilesSystem::GetFileList(){
    QMutexLocker locker(&_mutex);
    return _files;
}

void DeviceFilesSystem::SetSdSupported(bool b){

    _sd_supported=b;
}

