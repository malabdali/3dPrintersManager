#include "chitugcodecommand.h"


ChituGcodeCommand::ChituGcodeCommand(Device *device, QByteArray command, uint32_t noResponseTimeout):GCodeCommand(device,command,noResponseTimeout)
{

}


void ChituGcodeCommand::OnAvailableData(const QByteArray &ba)
{
}

void ChituGcodeCommand::OnAllDataWritten(bool)
{
}

void ChituGcodeCommand::InsideStart()
{
}

void ChituGcodeCommand::InsideStop()
{
}
