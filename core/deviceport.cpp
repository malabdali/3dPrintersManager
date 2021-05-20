#include "deviceport.h"
#include "device.h"
#include "../config.h"
#include <QTimerEvent>
#include "deviceinfo.h"
#include <QUdpSocket>
DevicePort::DevicePort(Device* device):DeviceConnection(device)
{
    _port=new QSerialPort(this);
    QObject::connect(_port,&QSerialPort::errorOccurred,this,&DevicePort::OnErrorOccurred);
    QObject::connect(_port,&QSerialPort::readyRead,this,&DevicePort::OnAvailableData);
    QObject::connect(_port,&QSerialPort::bytesWritten,this,&DevicePort::OnDataWritten);

}

void DevicePort::Open(){
    if(_device->GetPort().isEmpty()){
        emit Opened(false);
        return ;
    }
    if(QThread::currentThread()!=this->thread())
    {
        CallFunction("Open");
        return;
    }
    if(_port->isOpen())
        return;
    Clear();
    _port->setBaudRate(_device->GetDeviceInfo()->GetBaudRate());
    _port->setPortName(_device->GetPort());
    if(_port->open(QIODevice::ReadWrite)){
        _error_text="";
        _error=-1;
        //_serial_port->setCurrentReadChannel(0);
        _port->setDataTerminalReady(true);
        _is_was_open=true;
        emit Opened(true);
    }
    else
    {
        emit Opened(false);
    }
}

bool DevicePort::IsOpen()
{
    QMutexLocker locker(&_mutex);
    return _port->isOpen();
}

void DevicePort::Close()
{
    if(QThread::currentThread()!=this->thread())
    {
        CallFunction("Close");
        return;
    }
    if(!IsOpen())
        return;
    _is_was_open=false;
    Clear();
    _port->close();
    emit Closed();
}

/*void DevicePort::Reconnect()
{
    if(QThread::currentThread()!=this->thread())
    {
        CallFunction("Reconnect");
        return;
    }
    Clear();
    if(IsOpen())
    {
        _port->close();
        _is_was_open=false;
    }
    if(_port->open(QIODevice::ReadWrite)){
        _error_text="";
        _error=QSerialPort::NoError;
        _port->setDataTerminalReady(true);
        emit Reconnected(true);
    }
    else
    {
        emit Reconnected(false);
    }


}*/

DevicePort::~DevicePort()
{
    _port->close();
}

void DevicePort::OnAvailableData()
{
    QMutexLocker locker(&_mutex);
    if(_can_read)
    {
        this->_available_data+=_port->readAll();
    }
    else
    {
        _port->readAll();
        return;
    }
    locker.unlock();
    if(this->_available_data.contains('\n'))
    {
        int lin=this->_available_data.lastIndexOf('\n');
        QByteArray data=_available_data.mid(0,lin);
        _mutex.lock();
        _available_data.remove(0,lin+1);
        _mutex.unlock();
        QByteArrayList list=data.split('\n');
        for(QByteArray& ba:list){
            ba=ba.simplified();
            ba=ba.trimmed();
        }
        InputFilter(list);
        if(list.length()>0){
            _mutex.lock();
            _available_lines.append(list);
            _mutex.unlock();
            emit NewLinesAvailable(list);
        }
    }
}

void DevicePort::OnErrorOccurred(int error)
{
    if(error!=QSerialPort::SerialPortError::NoError && error!=QSerialPort::SerialPortError::NotOpenError)
    {
        _mutex.lock();
        if(!this->_port->errorString().isEmpty())
        {
            this->_error=_error;
            this->_error_text=this->_port->errorString().toUtf8();
        }
        else
        {
            this->_error=QSerialPort::SerialPortError::NoError;
            this->_error_text="";
        }
        _mutex.unlock();
        Clear();
        emit ErrorOccurred((int)error);
        if(!_port->isOpen() && _is_was_open)
        {
            _is_was_open=false;
            emit Closed();
        }
    }
}

void DevicePort::OnDataWritten(quint64 size)
{
    _mutex.lock();
    this->_writing_data_size-=size;
    _mutex.unlock();
    if(_writing_data_size<=0){
        if(_writing_timer!=-1)
        {
            this->killTimer(_writing_timer);
            _writing_timer=-1;
        }
        emit DataWritten(true);
    }
    else{
        if(_writing_timer!=-1)
        {
            this->killTimer(_writing_timer);
            _writing_timer=-1;
        }
        _writing_timer=this->startTimer(SERIAL_WRITE_WAIT);
    }

}

void DevicePort::Disable()
{
    QObject::disconnect(_port,&QSerialPort::errorOccurred,this,&DevicePort::OnErrorOccurred);
    QObject::disconnect(_port,&QSerialPort::readyRead,this,&DevicePort::OnAvailableData);
    QObject::disconnect(_port,&QSerialPort::bytesWritten,this,&DevicePort::OnDataWritten);
    Close();
}


void DevicePort::WriteFunction(const QByteArray & bytes)
{
    _port->write(bytes);
}
