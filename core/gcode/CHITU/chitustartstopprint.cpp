#include "chitustartstopprint.h"
#include "../../device.h"
#include "../../deviceport.h"
#include "../../device.h"

GCode::Chitu::ChituStartStopPrint::ChituStartStopPrint(Device *_device, QByteArray fileName, bool start):ChituGcodeCommand(_device,"M6030"),
    _file_name(fileName)
{
    _start_print_command=start;
    _cannot_start_print=false;
}

QByteArray GCode::Chitu::ChituStartStopPrint::GetFileName() const
{
    return _file_name;
}

bool GCode::Chitu::ChituStartStopPrint::IsPrintCommand() const
{
    return _start_print_command;
}


void GCode::Chitu::ChituStartStopPrint::InsideStart()
{
    if(_start_print_command)
    {
        _device->GetDeviceConnection()->Write(QByteArray("M6030 ':")+_file_name+"'\n");
    }
    else{
        _device->GetDeviceConnection()->Write(QByteArray("M33 I5\n"));
    }
}

void GCode::Chitu::ChituStartStopPrint::InsideStop()
{
}

void GCode::Chitu::ChituStartStopPrint::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().contains("ok")){

        if(_start_print_command){
            if(_cannot_start_print)
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
        _cannot_start_print=false;
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

void GCode::Chitu::ChituStartStopPrint::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::Chitu::ChituStartStopPrint::Finish(bool b)
{
    GCodeCommand::Finish(b);
}
