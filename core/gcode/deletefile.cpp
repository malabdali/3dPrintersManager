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

        if(_command_error!=CommandError::NoError)
        {
            Finish(false);
            return;
        }
        Finish(_is_success);
    }
    else{
        if(ba.contains("Error:No Checksum") || ba.contains("Resend:"))
        {
            SetError(CommandError::NoChecksum);
        }
        else if(ba.toLower().contains("busy")||ba.toLower().startsWith("t:"))
        {
            SetError(CommandError::Busy);
            Finish(false);
        }
        else{
            SetError(CommandError::UnknownError);
            Finish(false);
        }
    }
}


void GCode::DeleteFile::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::DeleteFile::Finish(bool b)
{
    GCodeCommand::Finish(b);
}
