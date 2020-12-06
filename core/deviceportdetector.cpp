#include "deviceportdetector.h"
#include <QDebug>
#include "devices.h"

DevicePortDetector::DevicePortDetector(QByteArray deviceName, quint32 baudRate, Device *device):DeviceComponent(device),
    _device_name(deviceName),_baud_rate(baudRate)
{

    QObject::connect(&_lookfor_ports_signals_mapper,SIGNAL(mapped(int)),this,SLOT(OnChecPortsDataAvailableMapped(int)));
    _wait_for_send_m115=true;
    _wait_for_send_m29=true;
    _port="";
    _send_m115_counter=0;

}

void DevicePortDetector::Setup()
{
}

void DevicePortDetector::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    if(_wait_for_send_m29)
    {
        for(QSerialPort* sp : _lookfor_available_ports){
            sp->write("M29 \n");
        }
        _wait_for_send_m29=false;
    }
    else if(_wait_for_send_m115)
    {
        for(QSerialPort* sp : _lookfor_available_ports){
            sp->write("M115 \n");
        }
        _wait_for_send_m115=false;
        killTimer(_lookfor_timer_id);
        _lookfor_timer_id=this->startTimer(DETECT_PORT_WAIT_TIME*2);
    }
    else
    {
        WhenEndDetectPort();
    }
}

void DevicePortDetector::StartDetect()
{
    this->_port="";
    for(QSerialPortInfo& port:QSerialPortInfo::availablePorts())
    {
        if(port.isBusy() || Devices::GetInstance()->GetAllDevicesPort().contains(port.portName().toUtf8()))
            continue;
        QSerialPort *sp=new QSerialPort(port);
        sp->setBaudRate(_baud_rate);
        if(sp->open(QIODevice::ReadWrite)){
            sp->setDataTerminalReady(true);
            _lookfor_available_ports.append(sp);
            _lookfor_available_ports_data.append("");
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
        _lookfor_timer_id=this->startTimer(DETECT_PORT_WAIT_TIME);
    }
}

void DevicePortDetector::DetectPortProcessing()
{
    QByteArray device_name=QByteArray(":")+_device_name+" ";
    for(int i=0;i<_lookfor_available_ports.length();i++){
        if(_lookfor_available_ports_data[i].toLower().contains(device_name.toLower())){
            _port=_lookfor_available_ports[i]->portName().toUtf8();
            WhenEndDetectPort();
        }
    }
}

void DevicePortDetector::WhenEndDetectPort()
{
    killTimer(_lookfor_timer_id);
    _lookfor_ports_signals_mapper.blockSignals(true);
    for(QSerialPort* port:_lookfor_available_ports)
    {
        _lookfor_ports_signals_mapper.removeMappings(port);
        port->setParent(nullptr);
        port->close();
        delete  port;
    }
    emit DetectPortFinished(_port);
}

void DevicePortDetector::OnChecPortsDataAvailableMapped(int sid)
{
    QByteArray ba=_lookfor_available_ports[sid]->readAll();
    if(_wait_for_send_m115)
        return;
    killTimer(_lookfor_timer_id);
    _lookfor_timer_id=this->startTimer(DETECT_PORT_WAIT_TIME);
    _lookfor_available_ports_data[sid].append(ba);
    DetectPortProcessing();
}



