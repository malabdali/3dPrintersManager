#include "startprinting.h"
#include "../device.h"
#include "../deviceport.h"
#include "../device.h"

GCode::StartPrinting::StartPrinting(Device *_device, std::function<void (bool)> callback, QByteArray fileName):GCodeCommand(_device,"M23"),_callback(callback),
    _file_name(fileName)
{

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
}

void GCode::StartPrinting::OnAllDataWritten()
{
}

void GCode::StartPrinting::Finish(bool b)
{
    GCodeCommand::Finish(b);
}
