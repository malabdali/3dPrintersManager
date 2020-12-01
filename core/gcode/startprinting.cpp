#include "startprinting.h"
#include "../device.h"
#include "../deviceport.h"
#include "../device.h"

GCode::StartPrinting::StartPrinting(Device *_device, QByteArray fileName):GCodeCommand(_device,"M23"),
    _file_name(fileName)
{
    _m24_sent=false;
}

QByteArray GCode::StartPrinting::GetFileName() const
{
    return _file_name;
}


void GCode::StartPrinting::InsideStart()
{
    _device->GetDevicePort()->Write(QByteArray("M23 ")+_file_name+"\n");
}

void GCode::StartPrinting::InsideStop()
{
}

void GCode::StartPrinting::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().startsWith("ok")){
        if(_command_error!=CommandError::NoError)
        {
            Finish(false);
            return;
        }
        if(_m24_sent){
            Finish(true);
        }
        else{
            if(_file_selected)
            {
                _device->GetDevicePort()->Write(QByteArray("M24 ")+"\n");
                _m24_sent=true;
            }
            else{
                Finish(false);
            }
        }
    }
    else if(ba.toLower().startsWith(QByteArray("open failed")))
    {
        _file_selected=false;
    }
    else if(ba.toLower().startsWith(QByteArray("file opened")) || ba.toLower().startsWith(QByteArray("file selected")))
    {
        _file_selected=true;
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

void GCode::StartPrinting::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::StartPrinting::Finish(bool b)
{
    GCodeCommand::Finish(b);
}
