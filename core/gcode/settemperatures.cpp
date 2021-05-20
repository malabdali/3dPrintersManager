#include "settemperatures.h"
#include "../device.h"
#include "../deviceport.h"
#include "../device.h"

GCode::SetTemperatures::SetTemperatures(Device *_device, int bed,int hotend):GCodeCommand(_device,"M104"),
    _bed(bed),_hotend(hotend)
{
    _hotend_sent=false;
    _bed_sent=false;
}

void GCode::SetTemperatures::InsideStart()
{
    if(_hotend>=0)
        _device->GetDeviceConnection()->Write(QByteArray("M104 S")+QByteArray::number(_hotend)+"\n");
    else if(_bed>=0)
    {
        _hotend_sent=true;
        _device->GetDeviceConnection()->Write(QByteArray("M140 S")+QByteArray::number(_bed)+"\n");
    }
    else{
        _hotend_sent=true;
        _bed_sent=true;
        Finish(true);
    }
}

void GCode::SetTemperatures::InsideStop()
{
}

void GCode::SetTemperatures::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().startsWith("ok")){
        if(!_hotend_sent)
        {
            _hotend_sent=true;
            if(_bed>=0)
            {
                _device->GetDeviceConnection()->Write(QByteArray("M140 S")+QByteArray::number(_bed)+"\n");
            }
            else
            {
                _bed_sent=true;
                Finish(true);
            }
        }
        else if(!_bed_sent){
            _bed_sent=true;
            Finish(true);
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

void GCode::SetTemperatures::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::SetTemperatures::Finish(bool b)
{
    GCodeCommand::Finish(b);
}

