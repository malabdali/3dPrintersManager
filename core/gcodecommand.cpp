#include "gcodecommand.h"
#include "device.h"

GCodeCommand::GCodeCommand(Device *device, QByteArray gcode):_device(device),_gcode(gcode)
{
    _finished=false;
    _started=false;
}

GCodeCommand::~GCodeCommand()
{

}

void GCodeCommand::Start()
{
    qDebug()<<this->thread()<<_device->thread();
    _started=true;
    QObject::connect(_device,&Device::NewLinesAvailable,this,&GCodeCommand::WhenLineAvailable);
    QObject::connect(_device,&Device::EndWrite,this,&GCodeCommand::WhenWriteFinished);
    QObject::connect(_device,&Device::BytesWritten,this,&GCodeCommand::WhenWriteLine);
    QObject::connect(_device,&Device::ErrorOccurred,this,&GCodeCommand::WhenErrorOcurre);
}

bool GCodeCommand::IsFinished()
{
    return _finished;
}

bool GCodeCommand::IsStarted() const
{
    return _started;
}

QByteArray GCodeCommand::GetGCode() const{
    return _gcode;
}

void GCodeCommand::WhenLineAvailable(QByteArrayList list)
{
    while(_device->IsThereAvailableLines())
        this->OnAvailableData(_device->ReadLine());
}

void GCodeCommand::WhenWriteFinished(bool b)
{
    this->OnAllDataWritten(b);
}

void GCodeCommand::WhenWriteLine()
{
    this->OnDataWritten();
}

void GCodeCommand::WhenErrorOcurre(int error)
{
    Finish(false);
}

void GCodeCommand::Finish(bool b)
{
    qDebug()<<"finish command";
    _finished=true;
    QObject::disconnect(_device,&Device::NewLinesAvailable,this,&GCodeCommand::WhenLineAvailable);
    QObject::disconnect(_device,&Device::EndWrite,this,&GCodeCommand::WhenWriteFinished);
    QObject::disconnect(_device,&Device::BytesWritten,this,&GCodeCommand::WhenWriteLine);
    QObject::disconnect(_device,&Device::ErrorOccurred,this,&GCodeCommand::WhenErrorOcurre);
    emit Finished(b);
}
