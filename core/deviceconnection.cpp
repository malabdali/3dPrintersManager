#include "deviceconnection.h"
#include "device.h"
#include "../config.h"
#include <QTimerEvent>
DeviceConnection::DeviceConnection(Device* device):DeviceComponent(device)
{
    _writing_data_size=0;
    _can_read=false;
    _writing_timer=-1;
    _is_was_open=false;
    _error=-1;
    _error_text="";
    QObject::connect(_device,&Device::destroyed,this,&DeviceConnection::WhenDeviceRemoved,Qt::ConnectionType::DirectConnection);

}


void DeviceConnection::Write(QByteArray bytes)
{
    _mutex.lock();
    _can_read=true;
    _writing_data_size=bytes.length();
    _mutex.unlock();
    CallFunction("InsideWrite",Q_ARG(QByteArray,bytes));
}



QByteArray DeviceConnection::ReadLine()
{
    QMutexLocker locker(&_mutex);
    if(_available_lines.length()>0)
        return _available_lines.takeAt(0);
    else
        return "";
}

QList<QByteArray> DeviceConnection::ReadAllLines()
{
    QMutexLocker locker(&_mutex);
    QList<QByteArray> allLines;
    allLines=std::move(_available_lines);
    return allLines;
}

uint32_t DeviceConnection::LinesCount()
{
    return _available_lines.length();
}

QByteArray DeviceConnection::PeakLine(int i)
{
    QMutexLocker locker(&_mutex);
    if(_available_lines.length()>i)
        return _available_lines.at(i);
    else
        return "";
}

bool DeviceConnection::IsThereAvailableLines()
{
    QMutexLocker locker(&_mutex);
    return _available_lines.length()>0;
}

void DeviceConnection::Clear()
{
    QMutexLocker locker(&_mutex);
    _available_data.clear();
    _available_lines.clear();
    _can_read=false;
    _writing_data_size=0;
    if(_writing_timer!=-1)
    {
        killTimer(_writing_timer);
        _writing_timer=-1;
    }
}

int DeviceConnection::GetError()
{
    QMutexLocker locker(&_mutex);
    return _error;
}

QByteArray DeviceConnection::GetErrorText()
{
    QMutexLocker locker(&_mutex);
    return _error_text;
}


DeviceConnection::~DeviceConnection()
{
    Clear();
}


/*void DeviceConnection::OnDataWritten(quint64 size)
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

}*/

void DeviceConnection::InsideWrite(QByteArray bytes)
{

    if(_writing_timer!=-1)
    {
        this->killTimer(_writing_timer);
        _writing_timer=-1;
    }
    _writing_timer=this->startTimer(SERIAL_WRITE_WAIT);
    WriteFunction(bytes);
}

void DeviceConnection::timerEvent(QTimerEvent *event)
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

void DeviceConnection::WhenDeviceRemoved()
{
    delete this;
}

void DeviceConnection::InputFilter(QByteArrayList &list)
{
    list.erase(std::remove_if(list.begin(),list.end(),[](QByteArray& ba){
                   bool b= ba.contains("echo:") || ba.isEmpty() || ba.startsWith("Marlin");
                   return b;
               }),list.end());
}

void DeviceConnection::CallFunction(QByteArray function)
{
    QMetaObject::invokeMethod(this,function,Qt::ConnectionType::AutoConnection);
}

void DeviceConnection::CallFunction(QByteArray function, QGenericArgument argument)
{
    QMetaObject::invokeMethod(this,function,Qt::ConnectionType::AutoConnection,argument);
}

void DeviceConnection::CallFunction(QByteArray function, QGenericArgument argument1, QGenericArgument argument2)
{
    QMetaObject::invokeMethod(this,function,Qt::ConnectionType::AutoConnection,argument1,argument2);
}

void DeviceConnection::Setup(){

}


void DeviceConnection::Disable()
{
}


QJsonDocument DeviceConnection::ToJson()
{
    return QJsonDocument();
}

void DeviceConnection::FromJson(QJsonDocument json)
{
}

void DeviceConnection::Save()
{
}

void DeviceConnection::Load()
{
}
