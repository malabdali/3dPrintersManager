#include "deletefile.h"
#include "../device.h"

GCode::DeleteFile::DeleteFile(Device *device, std::function<void(bool)> callback, QByteArray fileName) :GCodeCommand(device,"M30"),
    _callback(callback),_file(fileName)
{

}

void GCode::DeleteFile::Start()
{
    GCodeCommand::Start();
    _device->Write(QByteArray("M30 ")+_file+"\n");
}

void GCode::DeleteFile::Stop()
{
    Finish(false);
}

void GCode::DeleteFile::OnAvailableData(const QByteArray &ba)
{
    if(ba.contains("File deleted:")){
        _is_success=true;
        qDebug()<<"delete file success";
    }
    else if(ba.contains("Deletion failed")){
        _is_success=false;
        qDebug()<<"delete file failed";
    }
    else if("ok"){
        Finish(_is_success);
    }
}

void GCode::DeleteFile::OnDataWritten()
{

}

void GCode::DeleteFile::OnAllDataWritten(bool)
{

}

void GCode::DeleteFile::Finish(bool b)
{
    _callback(b);
    GCodeCommand::Finish(b);
}
