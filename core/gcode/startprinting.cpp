#include "startprinting.h"
#include "../device.h"

GCode::StartPrinting::StartPrinting(Device *_device, std::function<void (bool)> callback, QByteArray fileName):GCodeCommand(_device,"M23"),_callback(callback),
    _file_name(fileName)
{

}


void GCode::StartPrinting::Start()
{
    _device->Write(QByteArray("M23 ")+_file_name+"\n");
}

void GCode::StartPrinting::Stop()
{
}

void GCode::StartPrinting::OnAvailableData(const QByteArray &ba)
{
}

void GCode::StartPrinting::OnDataWritten()
{
}

void GCode::StartPrinting::OnAllDataWritten(bool)
{
}

void GCode::StartPrinting::Finish(bool)
{
}
