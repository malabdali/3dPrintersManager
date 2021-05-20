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
        _no_response_timer->setSingleShot(true);
        QObject::connect(_no_response_timer,&QTimer::timeout,this,&GCodeCommand::WhenTimeOut);
    }
    QObject::connect(_device->GetDeviceConnection(),&DeviceConnection::NewLinesAvailable,this,&GCodeCommand::WhenLineAvailable);
    QObject::connect(_device->GetDeviceConnection(),&DeviceConnection::DataWritten,this,&GCodeCommand::WhenWriteFinished);
    QObject::connect(_device->GetDeviceConnection(),&DeviceConnection::ErrorOccurred,this,&GCodeCommand::WhenErrorOccured);
    QObject::connect(_device->GetDeviceConnection(),&DeviceConnection::Closed,this,&GCodeCommand::WhenPortClosed);

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
    if(_device==nullptr)return;
    while(_device->GetDeviceConnection()->IsThereAvailableLines())
        this->OnAvailableData(_device->GetDeviceConnection()->ReadLine());
}

void GCodeCommand::WhenWriteFinished(bool success)
{
    if(_device==nullptr)return;
    if(_no_response_time_out>0){
        _no_response_timer->stop();
        _no_response_timer->start(_no_response_time_out);
    }
    this->OnAllDataWritten(success);
}

void GCodeCommand::WhenErrorOccured(int error)
{
    if(_device==nullptr)return;
    SetError(PortError);
    Finish(false);
}

void GCodeCommand::WhenPortClosed()
{
    if(_device==nullptr)return;
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
    if(!b){
        qDebug()<<"command finished with error : "<<GetError()<<_gcode;
    }
    QObject::disconnect(_device->GetDeviceConnection(),&DevicePort::NewLinesAvailable,this,&GCodeCommand::WhenLineAvailable);
    QObject::disconnect(_device->GetDeviceConnection(),&DevicePort::DataWritten,this,&GCodeCommand::WhenWriteFinished);
    QObject::disconnect(_device->GetDeviceConnection(),&DevicePort::ErrorOccurred,this,&GCodeCommand::WhenErrorOccured);
    QObject::disconnect(_device->GetDeviceConnection(),&DevicePort::Closed,this,&GCodeCommand::WhenPortClosed);
    emit Finished(b);
}

void GCodeCommand::WhenTimeOut()
{
    if(_device==nullptr)return;
    this->_no_response_timer->stop();
    this->SetError(TimeOut);
    this->Stop();
}

void GCodeCommand::SetError(GCodeCommand::CommandError error)
{
    _command_error=error;
}
