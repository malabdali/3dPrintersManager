#include "stopsdprint.h"
#include "../device.h"
#include "../deviceport.h"

GCode::StopSDPrint::StopSDPrint(Device *device):GCodeCommand(device,"M524")
{
}

void GCode::StopSDPrint::InsideStart()
{
    _device->GetDevicePort()->Write(QByteArray("M524 \n"));
}

void GCode::StopSDPrint::InsideStop()
{
}

void GCode::StopSDPrint::OnAvailableData(const QByteArray &ba)
{
    if(ba=="ok")
        Finish(true);
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

void GCode::StopSDPrint::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::StopSDPrint::Finish(bool b)
{
    GCodeCommand::Finish(b);
}
