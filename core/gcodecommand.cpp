#include "gcodecommand.h"
#include "device.h"
#include "deviceport.h"
#include <QTimer>

GCodeCommand::GCodeCommand(Device *device, QByteArray gcode, uint32_t noResponseTimeout):_device(device),_gcode(gcode)
{
    this->moveToThread(device->thread());
    this->setParent(device);
    _finished=false;
    _started=false;
    _is_success=false;
    _command_error=NoError;
    _no_response_time_out=noResponseTimeout;
}

GCodeCommand::~GCodeCommand()
{

}

void GCodeCommand::Start()
{
    _mutex.lock();
    _started=true;
    _mutex.unlock();
    if(QThread::currentThread()!=this->thread())
    {
        QMetaObject::invokeMethod(this,"Start");
        return;
    }
    if(_no_response_time_out){
        _no_response_timer=new QTimer(this);
        QObject::connect(_no_response_timer,&QTimer::timeout,this,&GCodeCommand::WhenTimeOut);
    }
    QObject::connect(_device->GetDevicePort(),&DevicePort::NewLinesAvailable,this,&GCodeCommand::WhenLineAvailable);
    QObject::connect(_device->GetDevicePort(),&DevicePort::DataWritten,this,&GCodeCommand::WhenWriteFinished);
    QObject::connect(_device->GetDevicePort(),&DevicePort::ErrorOccurred,this,&GCodeCommand::WhenErrorOccured);
    QObject::connect(_device->GetDevicePort(),&DevicePort::PortClosed,this,&GCodeCommand::WhenPortClosed);


    InsideStart();
}

bool GCodeCommand::IsFinished()
{
    return _finished;
}

void GCodeCommand::Stop()
{
    if(QThread::currentThread()!=this->thread())
    {
        QMetaObject::invokeMethod(this,"Stop");
        return;
    }
    InsideStop();
    Finish(false);

}

bool GCodeCommand::IsStarted() const
{
    return _started;
}

QByteArray GCodeCommand::GetGCode() const{
    return _gcode;
}

bool GCodeCommand::IsSuccess() const
{
    return _is_success;
}

GCodeCommand::CommandError GCodeCommand::GetError() const{
    return _command_error;
}

void GCodeCommand::WhenLineAvailable(QByteArrayList list)
{
    while(_device->GetDevicePort()->IsThereAvailableLines())
        this->OnAvailableData(_device->GetDevicePort()->ReadLine());
}

void GCodeCommand::WhenWriteFinished(bool success)
{
    if(_no_response_time_out>0){
        _no_response_timer->stop();
        _no_response_timer->start(_no_response_time_out);
    }
    this->OnAllDataWritten(success);
}

void GCodeCommand::WhenErrorOccured(int error)
{
    SetError(PortError);
    Finish(false);
}

void GCodeCommand::WhenPortClosed()
{
    SetError(PortClosed);
    Finish(false);
}

void GCodeCommand::Finish(bool b)
{
    if(_finished)
       return;
    _mutex.lock();
    _is_success=b;
    _finished=true;
    _mutex.unlock();
    QObject::disconnect(_device->GetDevicePort(),&DevicePort::NewLinesAvailable,this,&GCodeCommand::WhenLineAvailable);
    QObject::disconnect(_device->GetDevicePort(),&DevicePort::DataWritten,this,&GCodeCommand::WhenWriteFinished);
    QObject::disconnect(_device->GetDevicePort(),&DevicePort::ErrorOccurred,this,&GCodeCommand::WhenErrorOccured);
    QObject::disconnect(_device->GetDevicePort(),&DevicePort::PortClosed,this,&GCodeCommand::WhenPortClosed);
    emit Finished(b);
}

void GCodeCommand::WhenTimeOut()
{
    this->_no_response_timer->stop();
    this->SetError(TimeOut);
    this->Stop();
}

void GCodeCommand::SetError(GCodeCommand::CommandError error)
{
    _command_error=error;
}
