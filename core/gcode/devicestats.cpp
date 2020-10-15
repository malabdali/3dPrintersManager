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
    _device->GetDevicePort()->Write(QByteArray("M115 \n"));
}

void GCode::DeviceStats::InsideStop()
{
}


void GCode::DeviceStats::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().startsWith("ok")){
        if(_stats.size()>1)
            Finish(true);
        else
            Finish(false);
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
        if(ba.contains("Error:No Checksum"))
        {
            SetError(CommandError::NoChecksum);
        }
    }
}


void GCode::DeviceStats::OnAllDataWritten()
{

}

void GCode::DeviceStats::Finish(bool b)
{
    GCodeCommand::Finish(b);
}

