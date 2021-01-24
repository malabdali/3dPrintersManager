#include "powersupply.h"
#include "../deviceport.h"
#include "../device.h"

GCode::PowerSupply::PowerSupply(Device *device,bool on):GCodeCommand(device,"80")
{

}


void GCode::PowerSupply::InsideStart()
{
}

void GCode::PowerSupply::InsideStop()
{
}

void GCode::PowerSupply::OnAvailableData(const QByteArray &ba)
{
}

void GCode::PowerSupply::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::PowerSupply::Finish(bool b)
{
    GCodeCommand::Finish(b);
}
