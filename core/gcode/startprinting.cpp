#include "startprinting.h"
#include "../device.h"
#include "../deviceport.h"
#include "../device.h"

GCode::StartPrinting::StartPrinting(Device *_device, QByteArray fileName):GCodeCommand(_device,"M23"),
    _file_name(fileName)
{
    _m24_sent=false;
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
    if(ba.toLower().contains(QByteArray("open failed").toLower()))
    {
        _file_selected=false;
    }
    else if(ba.toLower().contains(QByteArray("File opened").toLower()) || ba.toLower().contains(QByteArray("File selected").toLower()))
    {
        _file_selected=true;
    }
    else{
        if(ba.contains("Error:No Checksum"))
        {
            SetError(CommandError::NoChecksum);
        }
    }
}

void GCode::StartPrinting::OnAllDataWritten(bool success)
{
}

void GCode::StartPrinting::Finish(bool b)
{
    GCodeCommand::Finish(b);
}
