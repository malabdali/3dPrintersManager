#include "linenumber.h"
#include "../deviceport.h"
#include "../device.h"

GCode::LineNumber::LineNumber(Device *device):GCodeCommand(device,"M20")
{
    _line_number=1;
    _29_sent=false;
    _is_fail=false;
    _is_open=false;
}

void GCode::LineNumber::InsideStart()
{
    _device->GetDevicePort()->Write("M28 lnt.txt\n");
}

void GCode::LineNumber::InsideStop()
{
}

void GCode::LineNumber::OnAvailableData(const QByteArray &ba)
{
    if(ba.contains("Writing to file:"))
    {
        _is_open=true;
    }
    else if(ba.contains("open failed")){
        _is_fail=true;
    }
    else if(ba.contains("ok")){
        if(_29_sent)
            Finish(true);
        else if(_is_open)
            this->_device->GetDevicePort()->Write("test\n");
        else if(_is_fail)
            Finish(false);
    }
    else if(ba.contains("Line Number") || ba.contains("Error:No Checksum")){

    }
    else if(ba.contains("Resend: ")){
        _line_number=ba.mid(8).simplified().trimmed().toUInt();
        _29_sent=true;
        _device->GetDevicePort()->Write("M29 \n");
    }

    else{
        if(ba.toLower().contains("busy")||ba.toLower().startsWith("t:"))
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

void GCode::LineNumber::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::LineNumber::Finish(bool b)
{
    GCodeCommand::Finish(b);
}

quint64 GCode::LineNumber::GetLineNumber()
{
    return _line_number;
}
