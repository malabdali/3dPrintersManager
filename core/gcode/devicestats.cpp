#include "devicestats.h"
#include "../device.h"
#include "../deviceport.h"
GCode::DeviceStats::DeviceStats(Device *device):GCodeCommand(device,"M115")
{

}

QMap<QByteArray, QByteArray> GCode::DeviceStats::GetStats()
{
    return _stats;
}

void GCode::DeviceStats::InsideStart()
{
    _device->GetDeviceConnection()->Write(QByteArray("M115 \n"));
}

void GCode::DeviceStats::InsideStop()
{
}


void GCode::DeviceStats::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().startsWith("ok")){
        if(_command_error!=CommandError::NoError)
            Finish(false);
        else
            Finish(true);
    }
    else if(ba.startsWith("Cap:")||ba.startsWith("FIRMWARE_NAME:"))
    {
        if(ba.startsWith("FIRMWARE_NAME:"))
        {
            _stats.insert("FIRMWARE_NAME",ba.mid(ba.indexOf(':')+1));
        }
        else{
            int mid=ba.indexOf(':',4);
            _stats.insert(ba.mid(4,mid-4),ba.mid(mid+1));
        }
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


void GCode::DeviceStats::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::DeviceStats::Finish(bool b)
{
    GCodeCommand::Finish(b);
}

