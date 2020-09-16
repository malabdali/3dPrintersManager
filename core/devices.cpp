#include "devices.h"
#include <QtDebug>

Devices* Devices::_INSTANCE=nullptr;

Devices::Devices(QObject *parent) : QObject(parent)
{
}

void Devices::WhenEndDevicesPortDetection()
{
    qDebug()<<"WhenEndDetection";
    emit DetectPortsEnded();
}

Devices *Devices::GetInstance(){
    if(_INSTANCE==nullptr)
        _INSTANCE=new Devices(qApp);
    return _INSTANCE;
}

Device* Devices::AddDevice(DeviceInfo& dev)
{
    QThread* thr=new QThread();
    thr->start();
    Device* device=this->GetDevice(dev);
    device=new Device(dev);
    qDebug()<<QThread::currentThread();
    device->moveToThread(thr);
    _devices.append(device);
    device->SetDeviceInfo(dev);
    QObject::connect(device,&Device::DetectPortSucceed,this,&Devices::OnDevicePortDetected);
    QObject::connect(device,&Device::DetectPortFailed,this,&Devices::OnDevicePortDetected);
    return device;
}

void Devices::RemoveDevice(const DeviceInfo &name)
{
    Device* dev=GetDevice(name);
    this->_devices.removeAll(dev);
    dev->ClosePort();
    dev->setParent(nullptr);
    QThread* thr=dev->thread();
    thr->quit();
    thr->wait();
    delete thr;
    delete dev;
}

Device *Devices::GetDeviceByPort(const QByteArray &port) const
{
    auto res=std::find_if(this->_devices.begin(),this->_devices.end(),[&port](Device* dev)->bool{return dev->GetPort()==port;});
    if(res==this->_devices.end())
        return nullptr;
    return *res;
}

Device *Devices::GetDevice(const DeviceInfo &name)const{
    auto res=std::find_if(this->_devices.begin(),this->_devices.end(),[&name](Device* dev)->bool{return dev->GetDeviceInfo()==DeviceInfo(name);});
    if(res==this->_devices.end())
        return nullptr;
    return *res;
}

void Devices::DetectPortAndConnectForAllDevices(bool force_detect_all_ports)
{
    _detect_ports_wait_list.clear();
    for(Device* dev : _devices){
        if(dev->GetPort().isEmpty() || force_detect_all_ports)
            _detect_ports_wait_list.append(dev);
        else
            dev->OpenPort();
    }
    qDebug()<<_detect_ports_wait_list.size();
    if(_detect_ports_wait_list.length()>0)
        _detect_ports_wait_list[0]->DetectDevicePort();
    else
        WhenEndDevicesPortDetection();
}

Devices::~Devices()
{
    QThread* thr;
    for(Device* dev:_devices){
        thr=dev->thread();
        thr->quit();
        thr->wait();
        delete thr;
        delete dev;
    }
}

void Devices::OnDevicePortDetected()
{
    Device* dev=qobject_cast<Device*>(this->sender());
    dev->OpenPort();
    _detect_ports_wait_list.removeAll(dev);
    if(_detect_ports_wait_list.length()>0)
        _detect_ports_wait_list[0]->DetectDevicePort();
    else
        WhenEndDevicesPortDetection();
}
