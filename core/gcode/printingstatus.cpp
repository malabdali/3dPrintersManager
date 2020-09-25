#include "printingstatus.h"
#include "../device.h"

GCode::PrintingStatus::PrintingStatus(Device *device, std::function<void (bool)> callback):GCodeCommand(device,"M78"),_callback(callback)
{

}

void GCode::PrintingStatus::Start()
{
    GCodeCommand::Start();
    _device->Write(QByteArray("M78 \n"));
}

void GCode::PrintingStatus::Stop()
{
    Finish(false);
    _device->StopWrite();
}


void GCode::PrintingStatus::OnAvailableData(const QByteArray &ba)
{
    qDebug()<<ba;
}

void GCode::PrintingStatus::OnDataWritten()
{

}

void GCode::PrintingStatus::OnAllDataWritten(bool)
{

}

void GCode::PrintingStatus::Finish(bool b)
{
    _callback(b);
    GCodeCommand::Finish(b);
}

