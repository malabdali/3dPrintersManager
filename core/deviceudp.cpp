#include "deviceudp.h"
#include "device.h"
#include "../config.h"
#include <QTimerEvent>
#include "deviceinfo.h"
#include <QUdpSocket>
#include <QNetworkDatagram>
DeviceUDP::DeviceUDP(Device* device):DeviceConnection(device)
{
    _udp=new QUdpSocket(this);
    _ping=new QProcess(this);
    _check_ip_timer=new QTimer();
    _check_ip_timer->setInterval(PING_IP_TIMER);
    _check_ip_timer->start();
    _is_open=false;
    _want_to_open=false;
    QObject::connect(_udp,&QUdpSocket::errorOccurred,this,&DeviceUDP::OnErrorOccurred);
    QObject::connect(_udp,&QUdpSocket::readyRead,this,&DeviceUDP::OnAvailableData);
    QObject::connect(_udp,&QUdpSocket::bytesWritten,this,&DeviceUDP::OnDataWritten);
    QObject::connect(_ping,(void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished,this,&DeviceUDP::OnPingFinished);
    QObject::connect(_check_ip_timer,&QTimer::timeout,this,&DeviceUDP::OnChekcIPTimeout);


}

void DeviceUDP::Open(){
    if(QThread::currentThread()!=this->thread())
    {
        CallFunction("Open");
        return;
    }
    if(IsOpen())
        return;
    Clear();
    _want_to_open=true;
    CheckIP();
}

bool DeviceUDP::IsOpen()
{
    QMutexLocker locker(&_mutex);
    return  _is_open;
}

void DeviceUDP::Close()
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
    _udp->close();
    _is_open=false;
    emit Closed();
}

void DeviceUDP::CheckIP()
{
    _ping->start("ping -n 1 "+_device->GetDeviceInfo()->GetDeviceIP());
}

/*void DeviceUDP::Reconnect()
{
    if(QThread::currentThread()!=this->thread())
    {
        CallFunction("Reconnect");
        return;
    }
    _want_to_reconnect=true;
    Clear();
    if(IsOpen())
    {
        _udp->disconnectFromHost();
        _is_was_open=false;
    }



}*/

DeviceUDP::~DeviceUDP()
{
}

void DeviceUDP::OnAvailableData()
{
    qDebug()<<"DeviceUDP::OnAvailableData()";
    QMutexLocker locker(&_mutex);
    if(_can_read)
    {
        while (_udp->hasPendingDatagrams()) {
            QNetworkDatagram datagram = _udp->receiveDatagram();
            this->_available_data+=datagram.data();
        }
    }
    else
    {
        while (_udp->hasPendingDatagrams()) {
            QNetworkDatagram datagram = _udp->receiveDatagram();
            datagram.data();
        }
        return;
    }
    locker.unlock();
    qDebug()<<"DeviceUDP::OnAvailableData()"<<_available_data;
    if(!this->_available_data.isEmpty())
    {
        QByteArray data=_available_data;
        _mutex.lock();
        _available_data.clear();
        _mutex.unlock();
        QByteArrayList list=data.split('\n');
        for(QByteArray& ba:list){
            ba=ba.simplified();
            ba=ba.trimmed();
        }
        InputFilter(list);
        qDebug()<<list;
        if(list.length()>0){
            _mutex.lock();
            _available_lines.append(list);
            _mutex.unlock();
            emit NewLinesAvailable(list);
        }
    }
}

void DeviceUDP::OnErrorOccurred(int error)
{
    if(error!=-1)
    {
        _mutex.lock();
        if(!this->_udp->errorString().isEmpty())
        {
            this->_error=_error;
            this->_error_text=this->_udp->errorString().toUtf8();
        }
        else
        {
            this->_error=-1;
            this->_error_text="";
        }
        _mutex.unlock();
        Clear();
        emit ErrorOccurred((int)error);
        if(!IsOpen() && _is_was_open)
        {
            _is_was_open=false;
            emit Closed();
        }
    }
}

void DeviceUDP::OnDataWritten(quint64 size)
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


void DeviceUDP::OnPingFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QMutexLocker locker(&_mutex);
    if(exitStatus==QProcess::ExitStatus::NormalExit && _ping->readAll().contains("Approximate round trip times")){
        //_udp->connectToHost(_device->GetDeviceInfo()->GetDeviceIP(),_device->GetDeviceInfo()->GetNetworkPort());
        if(_want_to_open)
        {
            _want_to_open=false;
            _is_open=true;
            _is_was_open=true;
            this->_error=-1;
            this->_error_text="";
            emit Opened(true);
        }
    }
    else{
        _is_open=false;
        this->_error=99;
        this->_error_text="cannot access to device";
        emit ErrorOccurred(_error);
        if(_want_to_open)
        {
            _want_to_open=false;
            emit Opened(false);
        }
        if(_is_was_open){
            _is_was_open=false;
            emit Closed();
        }
    }
}

void DeviceUDP::OnChekcIPTimeout()
{
    if(_is_open)
        this->CheckIP();
}

void DeviceUDP::Disable()
{
    QObject::disconnect(_udp,&QUdpSocket::errorOccurred,this,&DeviceUDP::OnErrorOccurred);
    QObject::disconnect(_udp,&QUdpSocket::readyRead,this,&DeviceUDP::OnAvailableData);
    QObject::disconnect(_udp,&QUdpSocket::bytesWritten,this,&DeviceUDP::OnDataWritten);
    QObject::disconnect(_ping,(void (QProcess::*)(int,QProcess::ExitStatus))&QProcess::finished,this,&DeviceUDP::OnPingFinished);
    Close();
}


void DeviceUDP::WriteFunction(const QByteArray & bytes)
{
    _udp->writeDatagram(bytes,QHostAddress(QString(_device->GetDeviceInfo()->GetDeviceIP())),_device->GetDeviceInfo()->GetNetworkPort());
}


void DeviceUDP::Clear()
{

    _udp->flush();
    DeviceConnection::Clear();

}
