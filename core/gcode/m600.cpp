#include "m600.h"
#include "../device.h"
#include "../deviceport.h"
GCode::M600::M600(Device *device):GCodeCommand(device,"M600")
{

}

void GCode::M600::InsideStart()
{
    _device->GetDevicePort()->Write(QByteArray("M600 \n"));
}

void GCode::M600::InsideStop()
{
}

void GCode::M600::OnAvailableData(const QByteArray &ba)
{
}

void GCode::M600::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
    else
        Finish(true);
}

void GCode::M600::Finish(bool b)
{
    GCodeCommand::Finish(b);
}
