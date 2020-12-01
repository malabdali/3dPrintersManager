#include "endstopsstates.h"
#include "../device.h"
#include "../deviceport.h"
#include <regex>

GCode::EndstopsStates::EndstopsStates(Device *device):GCodeCommand(device,"M119")
{
}

bool GCode::EndstopsStates::FilamentExist() const
{
    return _filament_is_ok;
}


void GCode::EndstopsStates::InsideStart()
{
    _device->GetDevicePort()->Write(QByteArray("M119 \n"));
}

void GCode::EndstopsStates::InsideStop()
{
}


void GCode::EndstopsStates::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().startsWith("ok")){
        if(_command_error!=CommandError::NoError)
        {
            Finish(false);
            return;
        }
        else
            Finish(true);
    }
    else if(ba.toLower().startsWith(QByteArray("reporting")))
    {
    }
    else if(ba.toLower().startsWith(QByteArray("x_")) || ba.toLower().startsWith(QByteArray("y_")) || ba.toLower().startsWith(QByteArray("z_")) ||
            ba.toLower().startsWith(QByteArray("filament"))){
        if(ba.toLower().startsWith(QByteArray("filament"))){
            auto res=ba.split(':');
            if(res[1].contains("TRIGGERED")){
                _filament_is_ok=true;
            }
            else{
                _filament_is_ok=false;
            }
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


void GCode::EndstopsStates::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::EndstopsStates::Finish(bool b)
{
    GCodeCommand::Finish(b);
}

