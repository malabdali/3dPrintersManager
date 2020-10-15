#include "powersupply.h"
#include "../deviceport.h"
#include "../device.h"

GCode::PowerSupply::PowerSupply(Device *device,bool on, std::function<void (bool)> callback):GCodeCommand(device,"80")
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

void GCode::PowerSupply::OnAllDataWritten()
{
}

void GCode::PowerSupply::Finish(bool b)
{
    GCodeCommand::Finish(b);
}
