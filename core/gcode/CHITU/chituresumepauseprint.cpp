#include "chituresumepauseprint.h"
#include "../../device.h"
#include "../../deviceport.h"
#include "../../device.h"

GCode::Chitu::ChituResumePausePrint::ChituResumePausePrint(Device *_device, bool start):ChituGcodeCommand(_device,"M24")
{
    _resume_print_command=start;
    _cannot_resum_print=false;
}


bool GCode::Chitu::ChituResumePausePrint::IsResumCommand() const
{
    return _resume_print_command;
}


void GCode::Chitu::ChituResumePausePrint::InsideStart()
{
    if(_resume_print_command)
    {
        _device->GetDeviceConnection()->Write(QByteArray("M24\n"));
    }
    else{
        _device->GetDeviceConnection()->Write(QByteArray("M25\n"));
    }
}

void GCode::Chitu::ChituResumePausePrint::InsideStop()
{
}

void GCode::Chitu::ChituResumePausePrint::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().contains("ok")){

        if(_resume_print_command){
            if(_cannot_resum_print)
                Finish(false);
            else
                Finish(true);
        }
        else{
            Finish(true);
        }
    }
    else if(ba.toLower().contains("error:"))
    {
        _cannot_resum_print=false;
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

void GCode::Chitu::ChituResumePausePrint::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::Chitu::ChituResumePausePrint::Finish(bool b)
{
    GCodeCommand::Finish(b);
}

