


#include "deviceinfo.h"
#include <QString>
#include <QVariantMap>
#include "remoteserver.h"

const QByteArray DeviceInfo::Types::FDM="FDM";
const QByteArray DeviceInfo::Types::SLA="SLA";

DeviceInfo::DeviceInfo(QByteArray name,QObject* parent):_name(name),QObject(parent)
{
    _save_changes_reply=nullptr;
}

uint32_t DeviceInfo::GetBaudRate() const
{
    return _baud_rate;
}

QByteArray DeviceInfo::GetDeviceName() const
{
    return _name;
}

void DeviceInfo::SetBaudRate(uint32_t br){
    _baud_rate=br;
    emit InfoChanged();
}

void DeviceInfo::SetDimensions(uint32_t x, uint32_t y, uint32_t z)
{
    _x=x;_y=y;_z=z;
    emit InfoChanged();
}

void DeviceInfo::SetNozzleDiameter(float diameter)
{
    _nozzle=diameter;
    emit InfoChanged();
}

void DeviceInfo::SetFilamentMaterial(QByteArray material)
{
    _filament_material=material;
    emit InfoChanged();
}

void DeviceInfo::SetNetworkID(QByteArray network)
{
    _network_id=network;
    emit InfoChanged();
}

void DeviceInfo::SetDeviceType(QByteArray device)
{
    _device_type=device;
    emit InfoChanged();
}

void DeviceInfo::SetDeviceModel(QByteArray model)
{
    _device_model=model;
    emit InfoChanged();
}

void DeviceInfo::SetDeviceIP(QByteArray ip)
{
    _device_ip=ip;
    emit InfoChanged();
}

void DeviceInfo::SetNetworkPort(int port)
{
    _device_network_port=port;
    emit InfoChanged();
}

void DeviceInfo::SetConnctionType(DeviceInfo::ConnectionType ct)
{
    _connection_type=ct;
    emit InfoChanged();

}

uint32_t DeviceInfo::GetX() const{
    return _x;
}

uint32_t DeviceInfo::GetY() const{
    return _y;
}

uint32_t DeviceInfo::GetZ() const{
    return _z;
}

bool DeviceInfo::operator==(const DeviceInfo &dev)const
{
    return dev._name==this->_name;
}

bool DeviceInfo::operator!=(const DeviceInfo &dev)const
{
    return dev._name!=this->_name;
}

QByteArray DeviceInfo::ToJson() const
{
    QVariantMap vmap=*this;
    QJsonDocument jdocument(QJsonObject::fromVariantMap(vmap));
    return jdocument.toJson();
}

DeviceInfo::operator QVariantMap() const
{
    QVariantMap vmap;
    vmap.insert("name",this->_name);
    vmap.insert("x",this->_x);
    vmap.insert("y",this->_y);
    vmap.insert("z",this->_z);
    vmap.insert("baudrate",this->_baud_rate);
    vmap.insert("material",this->_filament_material);
    vmap.insert("nozzle",this->_nozzle);
    vmap.insert("network",this->_network_id);
    vmap.insert("type",this->_device_type);
    vmap.insert("model",this->_device_model);
    vmap.insert("ip",this->_device_ip);
    vmap.insert("port",this->_device_network_port);
    vmap.insert("connection_type",this->_connection_type);
    return vmap;
}

QByteArray DeviceInfo::GetID()const{
    return _id;
}

QByteArray DeviceInfo::GetDeviceType() const
{
    return _device_type;
}

QByteArray DeviceInfo::GetDeviceModel() const
{
    return _device_model;
}

QByteArray DeviceInfo::GetDeviceIP() const
{
    return _device_ip;
}

int DeviceInfo::GetNetworkPort() const
{
    return _device_network_port;
}

float DeviceInfo::GetNozzleDiameter()
{
    return _nozzle;
}

QByteArray DeviceInfo::GetFilamentMaterial()
{
    return _filament_material;
}

QByteArray DeviceInfo::GetNetworkID()
{
    return _network_id;
}

DeviceInfo::ConnectionType DeviceInfo::GetConnectionType()
{
    return _connection_type;
}

void DeviceInfo::SaveChanges()
{
    RemoteServer::GetInstance()->SendUpdateQuery([this](QNetworkReply* rep)->void{
        _save_changes_reply=nullptr;
        if(RemoteServer::GetInstance()->IsSuccess(rep))
        {
            emit Saved(true);
        }
        else
            emit Saved(false);

        rep->deleteLater();
    },DEVICES_TABLE,*this,GetID());
}

void DeviceInfo::FromJSON(const QJsonObject &json)
{
    if(json.contains("name"))
        this->_name=json["name"].toString().toUtf8();
    if(json.contains("material"))
        this->_filament_material=json["material"].toString().toUtf8();
    if(json.contains("nozzle"))
        this->_nozzle=json["nozzle"].toDouble();
    if(json.contains("x"))
        this->_x=json["x"].toInt();
    if(json.contains("y"))
        this->_y=json["y"].toInt();
    if(json.contains("z"))
        this->_z=json["z"].toInt();
    if(json.contains("_id"))
        this->_id=json["_id"].toString().toUtf8();
    if(json.contains("baudrate"))
        this->_baud_rate=json["baudrate"].toInt();
    if(json.contains("network"))
        this->_network_id=json["network"].toString().toUtf8();
    if(json.contains("type"))
        this->_device_type=json["type"].toString().toUtf8();
    if(json.contains("model"))
        this->_device_model=json["model"].toString().toUtf8();
    if(json.contains("ip"))
        this->_device_ip=json["ip"].toString().toUtf8();
    if(json.contains("port"))
        this->_device_network_port=json["port"].toInt();
    if(json.contains("connection_type"))
        this->_connection_type=(ConnectionType)json["connection_type"].toInt();

}

DeviceInfo::~DeviceInfo()
{
    RemoteServer::GetInstance()->RemoveRequest(_save_changes_reply);
}

DeviceInfo::operator QByteArray()const
{
    return this->_name;
}




