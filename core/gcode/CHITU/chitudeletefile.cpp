#include "chitudeletefile.h"

#include "../../device.h"
#include "../../deviceconnection.h"

GCode::Chitu::ChituDeleteFile::ChituDeleteFile(Device *device, QByteArray fileName) :ChituGcodeCommand(device,"M30"), _file(fileName)
{
    _is_success=false;
}

void GCode::Chitu::ChituDeleteFile::InsideStart()
{
    _device->GetDeviceConnection()->Write(QByteArray("M30 ")+_file+"\n");
}

void GCode::Chitu::ChituDeleteFile::InsideStop()
{
}

QByteArray GCode::Chitu::ChituDeleteFile::GetFileName()
{
    return _file;
}

void GCode::Chitu::ChituDeleteFile::OnAvailableData(const QByteArray &ba)
{
    if(ba.contains("File deleted")){
        _is_success=true;
    }
    else if(ba.contains("Delete failed")){
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


void GCode::Chitu::ChituDeleteFile::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::Chitu::ChituDeleteFile::Finish(bool b)
{
    GCodeCommand::Finish(b);
}

