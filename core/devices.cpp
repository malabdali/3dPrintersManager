#include "devices.h"
#include <QtDebug>
#include "deviceinfo.h"
#include <QDir>
#include <QSettings>
#include "./system.h"
Devices* Devices::_INSTANCE=nullptr;

Devices::Devices(QObject *parent) : QObject(parent)
{

    QSettings settings;
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
    emit DeviceAdded(device);
    return device;
}

void Devices::RemoveDevice(const DeviceInfo *name)
{
    Device* dev=GetDevice(name);
    this->_devices.removeAll(dev);
    emit DeviceRemoved(dev);
    /*dev->ClosePort();
    dev->setParent(nullptr);
    dev->ClearCommands();
    dev->PauseCommands();

    QThread* thr=dev->thread();
    thr->quit();
    thr->wait();
    delete thr;*/
    dev->Remove();
}

Device *Devices::GetDeviceByPort(const QByteArray &port) const
{
    auto res=std::find_if(this->_devices.begin(),this->_devices.end(),[&port](Device* dev)->bool{return dev->GetPort()==port;});
    if(res==this->_devices.end())
        return nullptr;
    return *res;
}

Device *Devices::GetDeviceByIP(const QByteArray &ip) const
{
    auto res=std::find_if(_devices.begin(),_devices.end(),[ip](Device* dev){return ip==dev->GetDeviceInfo()->GetDeviceIP();});
    if(res!=_devices.end())
        return *res;
    else return nullptr;
}

QList<Device *> Devices::GetDevicesByType(QByteArray type)
{
    QList<Device*> devices;
    for(Device* dev : _devices){
        if(dev->GetDeviceInfo()->GetDeviceType()==type){
            devices.append(dev);
        }
    }
    return devices;
}

QList<Device *> Devices::GetDevicesByModel(QByteArray model)
{
    QList<Device*> devices;
    for(Device* dev : _devices){
        if(dev->GetDeviceInfo()->GetDeviceModel()==model){
            devices.append(dev);
        }
    }
    return devices;
}

Device *Devices::GetDevice(const DeviceInfo* name)const{
    auto res=std::find_if(this->_devices.begin(),this->_devices.end(),[&name](Device* dev)->bool{
            return dev->GetDeviceInfo()->GetDeviceName()==*name;
    });
    if(res==this->_devices.end())
        return nullptr;
    return *res;
}

Device *Devices::GetDevice(const QByteArray &name) const
{
    DeviceInfo dev(name);
    return GetDevice(&dev);
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
            reply->deleteLater();
            QJsonArray jarray=RemoteServer::GetInstance()->GetJSONValue(reply).toArray();
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
    RemoteServer::GetInstance()->SendSelectQuery(rf,DEVICES_TABLE,QString("{\"network\":\"%1\"}").arg(QString(System::GetInstance()->GetNetworkID())));
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

void Devices::CreateDevice(const DeviceInfo &di)
{
    RemoteServer::GetInstance()->SendInsertQuery([this,name=di.GetDeviceName()](QNetworkReply* rep)->void{
        if(RemoteServer::GetInstance()->IsSuccess(rep))
        {

            DeviceInfo* device=new DeviceInfo(name);
            device->FromJSON(RemoteServer::GetInstance()->GetJSONValue(rep).toObject()["ops"].toArray()[0].toObject());
            Device* dev=Devices::GetInstance()->AddDevice(device);
            emit DeviceCreated(dev);
        }
        else
            emit DeviceCreated(nullptr);

        rep->deleteLater();
    },DEVICES_TABLE,di);
}

void Devices::DeleteDevice(const DeviceInfo& di)
{

    RemoteServer::GetInstance()->SendDeleteQuery([this,name=di.GetDeviceName(),id=di.GetID()](QNetworkReply* rep)->void{
        if(RemoteServer::GetInstance()->IsSuccess(rep))
        {
            Device* dev=GetDevice(name);
            emit DeviceDeleted(dev);
            Devices::GetInstance()->RemoveDevice(dev->GetDeviceInfo());
        }
        else
            emit DeviceDeleted(nullptr);

        rep->deleteLater();
    },DEVICES_TABLE,di.GetID());
}


void Devices::Clear()
{
    for(Device* di:_devices.toVector())
        RemoveDevice(di->GetDeviceInfo());

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
