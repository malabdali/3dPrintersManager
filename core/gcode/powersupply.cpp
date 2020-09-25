#include "powersupply.h"

GCode::PowerSupply::PowerSupply(Device *device,bool on, std::function<void (bool)> callback):GCodeCommand(device,"80")
{

}


void GCode::PowerSupply::Start()
{
}

void GCode::PowerSupply::Stop()
{
}

void GCode::PowerSupply::OnAvailableData(const QByteArray &ba)
{
}

void GCode::PowerSupply::OnDataWritten()
{
}

void GCode::PowerSupply::OnAllDataWritten(bool)
{
}

void GCode::PowerSupply::Finish(bool)
{
}
