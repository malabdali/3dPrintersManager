#include "chitudevicestats.h"
#include "../../device.h"
#include "../../deviceconnection.h"
GCode::Chitu::ChituDeviceStats::ChituDeviceStats(Device *device):ChituGcodeCommand(device,"M115")
{

}

QMap<QByteArray, QByteArray> GCode::Chitu::ChituDeviceStats::GetStats()
{
    return _stats;
}

void GCode::Chitu::ChituDeviceStats::InsideStart()
{
    _device->GetDeviceConnection()->Write(QByteArray("M115 \n"));
}

void GCode::Chitu::ChituDeviceStats::InsideStop()
{
}


void GCode::Chitu::ChituDeviceStats::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().startsWith("ok")){
        if(_command_error!=CommandError::NoError)
            Finish(false);
        else
            Finish(true);
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


void GCode::Chitu::ChituDeviceStats::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::Chitu::ChituDeviceStats::Finish(bool b)
{
    GCodeCommand::Finish(b);
}

