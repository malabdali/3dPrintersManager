#include "devices.h"
#include <QtDebug>
#include "deviceinfo.h"

Devices* Devices::_INSTANCE=nullptr;

Devices::Devices(QObject *parent) : QObject(parent)
{
}

void Devices::WhenEndDevicesPortDetection()
{
    emit DetectPortsEnded();
}

Devices *Devices::GetInstance(){
    if(_INSTANCE==nullptr)
        _INSTANCE=new Devices(qApp);
    return _INSTANCE;
}

Device* Devices::AddDevice(DeviceInfo* dev)
{
    //QThread* thr=new QThread();
    //thr->start();
    Device* device=this->GetDevice(dev);
    device=new Device(dev);
    //device->moveToThread(thr);
    _devices.append(device);
    //qDebug()<<this->thread()<<device->thread()<<device->GetDeviceInfo()->thread()<<dev->thread();

    qDebug()<<"Devices::AddDevice 1";
    emit DeviceAdded(device);
    qDebug()<<"Devices::AddDevice 2";
    return device;
}

void Devices::RemoveDevice(const DeviceInfo *name)
{
    Device* dev=GetDevice(name);
    this->_devices.removeAll(dev);
    emit DeviceRemoved(dev);
    dev->ClosePort();
    dev->setParent(nullptr);
    /*QThread* thr=dev->thread();
    thr->quit();
    thr->wait();
    delete thr;*/
    delete dev;
}

Device *Devices::GetDeviceByPort(const QByteArray &port) const
{
    auto res=std::find_if(this->_devices.begin(),this->_devices.end(),[&port](Device* dev)->bool{return dev->GetPort()==port;});
    if(res==this->_devices.end())
        return nullptr;
    return *res;
}

Device *Devices::GetDevice(const DeviceInfo* name)const{
    auto res=std::find_if(this->_devices.begin(),this->_devices.end(),[&name](Device* dev)->bool{
            return dev->GetDeviceInfo()->GetDeviceName()==*name;
    });
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
    if(_detect_ports_wait_list.length()>0)
    {
        QObject::connect(_detect_ports_wait_list[0],&Device::DetectPortSucceed,this,&Devices::OnDevicePortDetected);
        QObject::connect(_detect_ports_wait_list[0],&Device::DetectPortFailed,this,&Devices::OnDevicePortDetected);
        _detect_ports_wait_list[0]->DetectDevicePort();
    }
    else
        WhenEndDevicesPortDetection();
}

void Devices::LoadDevicesFromRemoteServer(){
    auto rf=[this](QNetworkReply* reply)->void{
        if(RemoteServer::GetInstance()->IsSuccess(reply))
        {
            QJsonArray jarray=RemoteServer::GetInstance()->GetJSONValue(reply).toArray();
            qDebug()<<"devices count : "<<jarray.size();
            for(QJsonValue value:jarray){
                DeviceInfo *di=new DeviceInfo("",nullptr);
                di->FromJSON(value.toObject());
                this->AddDevice(di);
            }
            emit DevicesLoaded(true);
        }
        else{
            emit DevicesLoaded(false);
        }
    };
    RemoteServer::GetInstance()->SendSelectQuery(rf,"Printers");
}

QList<Device *> Devices::GetAllDevices()
{
    return _devices;
}

QList<QByteArray> Devices::GetAllDevicesPort()
{
    QList<QByteArray> ports;
    for(Device* dev :this->_devices)
    {
        ports.append(dev->GetPort());
    }
    return ports;
}


Devices::~Devices()
{
    //QThread* thr;
    for(Device* dev:_devices){
        //thr=dev->thread();
        //thr->quit();
        //thr->wait();
        //delete thr;
        delete dev;
    }
}

void Devices::OnDevicePortDetected()
{
    Device* dev=qobject_cast<Device*>(this->sender());
    dev->OpenPort();

    QObject::disconnect(dev,&Device::DetectPortSucceed,this,&Devices::OnDevicePortDetected);
    QObject::disconnect(dev,&Device::DetectPortFailed,this,&Devices::OnDevicePortDetected);
    _detect_ports_wait_list.removeAll(dev);
    if(_detect_ports_wait_list.length()>0)
    {
        QObject::connect(_detect_ports_wait_list[0],&Device::DetectPortSucceed,this,&Devices::OnDevicePortDetected);
        QObject::connect(_detect_ports_wait_list[0],&Device::DetectPortFailed,this,&Devices::OnDevicePortDetected);
        _detect_ports_wait_list[0]->DetectDevicePort();
    }
    else
        WhenEndDevicesPortDetection();
}
