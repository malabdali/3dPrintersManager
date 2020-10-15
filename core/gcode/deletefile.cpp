#include "deletefile.h"
#include "../device.h"
#include "../deviceport.h"

GCode::DeleteFile::DeleteFile(Device *device, std::function<void(bool)> callback, QByteArray fileName) :GCodeCommand(device,"M30"),
    _callback(callback),_file(fileName)
{

}

void GCode::DeleteFile::InsideStart()
{
    _device->GetDevicePort()->Write(QByteArray("M30 ")+_file+"\n");
}

void GCode::DeleteFile::InsideStop()
{
}

QByteArray GCode::DeleteFile::GetFileName()
{
    return _file;
}

void GCode::DeleteFile::OnAvailableData(const QByteArray &ba)
{
    if(ba.contains("File deleted:")){
        _is_success=true;
    }
    else if(ba.contains("Deletion failed")){
        _is_success=false;
    }
    else if("ok"){
        Finish(_is_success);
    }
}


void GCode::DeleteFile::OnAllDataWritten()
{

}

void GCode::DeleteFile::Finish(bool b)
{
    GCodeCommand::Finish(b);
}
