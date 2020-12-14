#include "deviceport.h"
#include "device.h"
#include "../config.h"
#include <QTimerEvent>
DevicePort::DevicePort(Device* device):DeviceComponent(device)
{
    _serial_port=new QSerialPort(this);
    _writing_data_size=0;
    _can_read=false;
    _writing_timer=-1;
    _reconnect=false;

    QObject::connect(_serial_port,&QSerialPort::readyRead,this,&DevicePort::OnAvailableData);
    QObject::connect(_serial_port,&QSerialPort::errorOccurred,this,&DevicePort::OnErrorOccurred);
    QObject::connect(_serial_port,&QSerialPort::bytesWritten,this,&DevicePort::OnDataWritten);

}


void DevicePort::Write(QByteArray bytes)
{
    _mutex.lock();
    _can_read=true;
    _writing_data_size=bytes.length();
    _mutex.unlock();
    CallFunction("InsideWrite",Q_ARG(QByteArray,bytes));
}

void DevicePort::Open(QByteArray port, quint64 baud_rate){
    if(QThread::currentThread()!=this->thread())
    {
        CallFunction("Open",Q_ARG(QByteArray,port),Q_ARG(quint64,baud_rate));
        return;
    }
    if(_serial_port->isOpen())
        return;
    Clear();
    _serial_port->setBaudRate(baud_rate);
    _serial_port->setPortName(port);
    //_serial_port->setCurrentReadChannel(0);
    //_serial_port->setDataTerminalReady(true);
    if(_serial_port->open(QIODevice::ReadWrite)){
        //_serial_port->setCurrentReadChannel(0);
        _serial_port->setDataTerminalReady(true);
        emit PortOpened(true);
    }
    else
    {
        emit PortOpened(false);
    }
}

QByteArray DevicePort::ReadLine()
{
    QMutexLocker locker(&_mutex);
    if(_available_lines.length()>0)
        return _available_lines.takeAt(0);
    else
        return "";
}

QList<QByteArray> DevicePort::ReadAllLines()
{
    QMutexLocker locker(&_mutex);
    QList<QByteArray> allLines;
    allLines=std::move(_available_lines);
    return allLines;
}

uint32_t DevicePort::LinesCount()
{
    return _available_lines.length();
}

QByteArray DevicePort::PeakLine(int i)
{
    QMutexLocker locker(&_mutex);
    if(_available_lines.length()>i)
        return _available_lines.at(i);
    else
        return "";
}

bool DevicePort::IsThereAvailableLines()
{
    QMutexLocker locker(&_mutex);
    return _available_lines.length()>0;
}

void DevicePort::Clear()
{
    QMutexLocker locker(&_mutex);
    _available_data.clear();
    _available_lines.clear();
    _can_read=false;
    _writing_data_size=0;
}

bool DevicePort::IsOpen()
{
    QMutexLocker locker(&_mutex);
    return _serial_port->isOpen();
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
    Clear();
    _serial_port->close();
    emit PortClosed();
}

void DevicePort::Reconnect()
{
    if(QThread::currentThread()!=this->thread())
    {
        CallFunction("Reconnect");
        return;
    }
    _reconnect=true;
    Clear();
    if(IsOpen())
    {
        _serial_port->close();
    }
    _reconnect=false;
    if(_serial_port->open(QIODevice::ReadWrite)){
        _serial_port->setDataTerminalReady(true);
        emit Reconnected(true);
    }
    else
        emit Reconnected(false);


}

DevicePort::~DevicePort()
{
}

void DevicePort::OnAvailableData()
{
    QMutexLocker locker(&_mutex);
    if(_can_read)
    {
        this->_available_data+=_serial_port->readAll();
    }
    else
    {
        _serial_port->readAll();
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
        SerialInputFilter(list);
        if(list.length()>0){
            _mutex.lock();
            _available_lines.append(list);
            _mutex.unlock();
            emit NewLinesAvailable(list);
        }
    }
}

void DevicePort::OnErrorOccurred(QSerialPort::SerialPortError error)
{
    if(error!=QSerialPort::SerialPortError::NoError && error!=QSerialPort::SerialPortError::NotOpenError)
    {
        Clear();
        emit ErrorOccurred(error);
        if(!_serial_port->isOpen())
        {
            emit PortClosed();
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

void DevicePort::InsideWrite(QByteArray bytes)
{

    if(_writing_timer!=-1)
    {
        this->killTimer(_writing_timer);
        _writing_timer=-1;
    }
    _writing_timer=this->startTimer(SERIAL_WRITE_WAIT);
    _serial_port->write(bytes);
}

void DevicePort::timerEvent(QTimerEvent *event)
{
    if(event->timerId()==_writing_timer)
    {
        if(_writing_timer!=-1)
        {
            this->killTimer(_writing_timer);
            _writing_timer=-1;
        }
        emit DataWritten(false);
    }
}

void DevicePort::SerialInputFilter(QByteArrayList &list)
{
    list.erase(std::remove_if(list.begin(),list.end(),[](QByteArray& ba){
                   bool b= ba.contains("echo:") || ba.isEmpty() || ba.startsWith("Marlin");
                   return b;
               }),list.end());
}

void DevicePort::CallFunction(QByteArray function)
{
    QMetaObject::invokeMethod(this,function,Qt::ConnectionType::AutoConnection);
}

void DevicePort::CallFunction(QByteArray function, QGenericArgument argument)
{
    QMetaObject::invokeMethod(this,function,Qt::ConnectionType::AutoConnection,argument);
}

void DevicePort::CallFunction(QByteArray function, QGenericArgument argument1, QGenericArgument argument2)
{
    QMetaObject::invokeMethod(this,function,Qt::ConnectionType::AutoConnection,argument1,argument2);
}

void DevicePort::Setup(){

}
