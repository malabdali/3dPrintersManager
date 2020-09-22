#include "device.h"
#include<functional>
#include <QDebug>
#include <algorithm>
#include <QTimerEvent>
#include "../config.cpp"
#include <QMetaMethod>
Device::Device(DeviceInfo* device_info,QObject *parent) : QObject(parent),_port(""),_device_info(device_info),_serial_port(new QSerialPort()),
_fileSystem(new DeviceFilesSystem(this,this)){
    qRegisterMetaType<Errors>();
    _write_task=new WriteTask();
    _write_task->dev=this;
    _write_task->setAutoDelete(false);
    device_info->setParent(this);

    QObject::connect(_serial_port,&QSerialPort::readyRead,this,&Device::OnAvailableData);
    QObject::connect(&_lookfor_ports_signals_mapper,SIGNAL(mapped(int)),this,SLOT(OnChecPortsDataAvailableMapped(int)));
    QObject::connect(_serial_port,&QSerialPort::errorOccurred,this,&Device::OnErrorOccurred);
    //QObject::connect(this->_serial_port,&QSerialPort::bytesWritten,this,&Device::OnWrittenData);
    QObject::connect(_serial_port,&QSerialPort::aboutToClose,this,&Device::OnClosed);
    //QObject::connect();
}

void Device::DetectDevicePort()
{
    if(QThread::currentThread()!=this->thread()){
        CallFunction("DetectDevicePort");
        return;
    }
    this->_port="";
    this->ClosePort();
    CleanDetectPortsProcessing();
    for(QSerialPortInfo& port:QSerialPortInfo::availablePorts())
    {
        if(port.isBusy())continue;
        QSerialPort *sp=new QSerialPort(port);

        if(sp->open(QIODevice::ReadWrite)){
            _lookfor_available_ports.append(sp);
            _lookfor_available_ports_data.append("");
            sp->setBaudRate(this->_device_info->GetBaudRate());
            QObject::connect(sp,SIGNAL(readyRead()),&_lookfor_ports_signals_mapper,SLOT(map()));
            _lookfor_ports_signals_mapper.setMapping(sp,_lookfor_available_ports.length()-1);
        }
        else{
            sp->setParent(nullptr);
            delete sp;
        }
    }
    if(_lookfor_available_ports.length()==0){
        WhenEndDetectPort();
    }
    else{
        if(_lookfor_timer_id==0){
            _lookfor_timer_id=this->startTimer(DETECT_PORT_WAIT_TIME*5);
        }
        else{
            killTimer(_lookfor_timer_id);
            _lookfor_timer_id=this->startTimer(DETECT_PORT_WAIT_TIME*5);
        }
        CalculateAndSetStatus();
    }
}

QByteArray Device::GetPort(){
    return this->_port;
}

void Device::SetPort(const QByteArray &port)
{
    _port=port;
}

void Device::Device::SetDeviceInfo(DeviceInfo* device)
{
    _device_info=device;
}

DeviceInfo *Device::GetDeviceInfo(){
    return this->_device_info;
}

void Device::SetStatus(Device::DeviceStatus status){
    DeviceStatus old=_current_status;
    _current_status=status;
    if(status!=old)
        emit StatusChanged(status);
}

void Device::WhenEndDetectPort()
{
    CleanDetectPortsProcessing();
    CalculateAndSetStatus();
    if(_current_status==DeviceStatus::PortDetected){
        emit DetectPortSucceed();
    }
    else{
        emit DetectPortFailed();
    }
}

void Device::CleanDetectPortsProcessing()
{
    for(int i=0;i<_lookfor_available_ports.length();i++){
        _lookfor_available_ports[i]->close();
        _lookfor_ports_signals_mapper.removeMappings(_lookfor_available_ports[i]);
        _lookfor_available_ports[i]->setParent(nullptr);
        delete _lookfor_available_ports[i];
    }
    _lookfor_available_ports.clear();
    _lookfor_available_ports_data.clear();
}

void Device::CalculateAndSetStatus()
{
    DeviceStatus ds=DeviceStatus::Non;
    if(_lookfor_available_ports.length()>0)
        ds=DeviceStatus::DetectDevicePort;
    if(_serial_port->isOpen())
        ds=DeviceStatus::Connected;
    if(!_port.isEmpty() && !_serial_port->isOpen())
        ds=DeviceStatus::PortDetected;
    SetStatus(ds);
}

void Device::SerialInputFilter(QByteArrayList &list)
{
    if(list.contains("echo:SD card ok"))
        _fileSystem->SetSdSupported(true);
    list.erase(std::remove_if(list.begin(),list.end(),[](QByteArray& ba){
                   bool b= ba.contains("echo:") || ba.isEmpty() || ba.startsWith("Marlin");
                   return b;
               }),list.end());
}

Device::DeviceStatus Device::GetStatus() const{
    return _current_status;
}

bool Device::IsOpen() const
{
    return _serial_port->isOpen();
}

bool Device::OpenPort(){
    if(_serial_port->isOpen())
        ClosePort();
    if(!this->_port.isEmpty())
    {
        _serial_port->setBaudRate(_device_info->GetBaudRate());
        _serial_port->setPortName(_port);
        if(_serial_port->open(QIODevice::ReadWrite)){
            CalculateAndSetStatus();
            emit PortOpened();
            return true;
        }
        else{
            return false;
        }
    }
    return false;
}

void Device::ClosePort(){
    if(_serial_port->isOpen())
    {
        ClearCommands();
        StopWrite();
        _availableData.clear();
        ClearLines();
        _serial_port->close();
        CalculateAndSetStatus();
    }
}

void Device::Clear()
{
    ClosePort();
    _port="";
    CalculateAndSetStatus();
}

void Device::StopWrite()
{
    _write_task->StopWrite();
}

bool Device::IsThereAvailableLines() const
{
    return _availableLines.length()>0;
}

void Device::Write(QByteArray bytes)
{
    _write_task->StartWrite(QByteArrayList({bytes}));
}

void Device::Write(QByteArrayList bytes)
{
    _write_task->StartWrite(QByteArrayList({bytes}));
}

QByteArray Device::ReadLine()
{
    QMutexLocker locker(&mutex);
    if(_availableLines.length()>0)
    {
        return _availableLines.takeAt(0);
    }
    return QByteArray();
}

bool Device::WriteIsBusy()
{
    return _write_task->IsBussy();
}

void Device::ClearLines()
{
    QMutexLocker locker(&mutex);
    _availableLines.clear();
}

void Device::AddGCodeCommand(GCodeCommand *command)
{
    QMutexLocker locker(&mutex);
    if(!IsOpen())
        return;
    command->moveToThread(this->thread());
    command->setParent(this);
    _commands.append(command);
    QObject::connect(command,&GCodeCommand::Finished,this,&Device::WhenCommandFinished);
    if(_commands.length()<2){
        CallFunction("StartNextCommand");
    }
}


void Device::ClearCommands()
{
    QMutexLocker locker(&mutex);
    if(_commands.length()>0){
        if(_commands[0]->IsStarted())
            _commands[0]->Stop();
        for(GCodeCommand* command :_commands){
            command->deleteLater();
        }
        _commands.clear();
    }
}

GCodeCommand *Device::GetCurrentCommand()
{
    QMutexLocker locker(&mutex);
    GCodeCommand* command=nullptr;
    if(_commands.length()>0)
        command = _commands[0];
    return command;
}

DeviceFilesSystem *Device::GetFileSystem() const
{
    return _fileSystem;
}

Device::~Device()
{
    _serial_port->close();
    ClearCommands();
    delete _serial_port;
    delete _write_task;
}

void Device::timerEvent(QTimerEvent *event){
    if(_lookfor_timer_id==event->timerId()){
        DetectPortProcessing();
    }

}

void Device::DetectPortProcessing(){
    this->killTimer(_lookfor_timer_id);
    _lookfor_timer_id=0;
    if(_current_status==DeviceStatus::DetectDevicePort){
        QByteArray device_name=_device_info->GetDeviceName()+"\n";
        for(int i=0;i<_lookfor_available_ports.length();i++){
            if(_lookfor_available_ports_data[i].toLower().contains(device_name.toLower())){
                this->SetPort(_lookfor_available_ports[i]->portName().toUtf8());
            }
        }
        WhenEndDetectPort();
    }
}

void Device::OnAvailableData(){
    this->_availableData+=_serial_port->readAll();
    if(this->_availableData.contains('\n'))
    {
        int lin=this->_availableData.lastIndexOf('\n');
        QByteArray data=_availableData.mid(0,lin);
        _availableData.remove(0,lin+1);
        QByteArrayList list=data.split('\n');
        for(QByteArray& ba:list){
            ba=ba.simplified();
            ba=ba.trimmed();
        }
        SerialInputFilter(list);
        QMutexLocker locker(&mutex);
        _availableLines.append(list);
        locker.unlock();
        emit NewLinesAvailable(list);
    }
}

void Device::OnChecPortsDataAvailableMapped(int sid)
{
    killTimer(_lookfor_timer_id);
    _lookfor_timer_id=this->startTimer(DETECT_PORT_WAIT_TIME);
    _lookfor_available_ports_data[sid].append(_lookfor_available_ports[sid]->readAll());
}

void Device::OnClosed()
{
    emit PortClosed();
}


void Device::WhenCommandFinished(bool b)
{
    QMutexLocker locker(&mutex);
    _commands.removeAll((GCodeCommand*)this->sender());
    locker.unlock();
    this->sender()->deleteLater();
    emit CommandFinished((GCodeCommand*)this->sender(),b);
    if(_commands.length()>0){
        StartNextCommand();
    }
}

void Device::CallFunction(const char* function)
{
    //this->metaObject()->method(QMetaMethod::))
    QMetaObject::invokeMethod(this,function,Qt::ConnectionType::AutoConnection);
}



void Device::StartNextCommand()
{
    ClearLines();
    _commands[0]->Start();
    emit CommandStarted(_commands[0]);
}

void Device::OnErrorOccurred(Errors error)
{
    if(error!=QSerialPort::SerialPortError::NoError && error!=QSerialPort::SerialPortError::NotOpenError)
    {
        Clear();
        emit ErrorOccurred(error);
    }
}

//Write task class

void Device::WriteTask::run()
{
    bool stop=_stopWrite;
    while (_write_wait_list.length()>0) {
        _writ_safe_thread.lockForRead();
        stop=_stopWrite;
        _writ_safe_thread.unlock();
        if(stop){
            WhenEndWriteTask(false);
            return;
        }
        _writ_safe_thread.lockForWrite();
        QByteArray bytes=_write_wait_list.takeAt(0);
        _writ_safe_thread.unlock();
        dev->_serial_port->write(bytes);
        if(!dev->_serial_port->waitForBytesWritten(SERIAL_WRITE_WAIT))
        {
            WhenEndWriteTask(false);
            return;
        }
        while(dev->_serial_port->waitForBytesWritten(SERIAL_WRITE_WAIT))
            ;
        dev->_serial_port->flush();
        emit dev->BytesWritten();
    }
    WhenEndWriteTask(!_stopWrite);
}

void Device::WriteTask::WhenEndWriteTask(bool success)
{
    _writ_safe_thread.lockForWrite();
    _write_wait_list.clear();
    _write_task_is_busy=false;
    emit dev->EndWrite(success);
    _writ_safe_thread.unlock();
}

void Device::WriteTask::StartWrite(QByteArrayList list)
{
    _writ_safe_thread.lockForWrite();
    _stopWrite=false;
    _write_wait_list.append(list);
    if(!_write_task_is_busy)
        QThreadPool::globalInstance()->start(this);
    _write_task_is_busy=true;
    _writ_safe_thread.unlock();
}

void Device::WriteTask::StopWrite()
{
    _writ_safe_thread.lockForWrite();
    _stopWrite=true;
    _write_wait_list.clear();
    _writ_safe_thread.unlock();
}

bool Device::WriteTask::IsBussy()
{
    bool busy;
    this->_writ_safe_thread.lockForRead();
    busy=_write_task_is_busy;
    this->_writ_safe_thread.unlock();
    return busy;
}
