#include "deviceinfo.h"
#include <QString>

DeviceInfo::DeviceInfo(QByteArray name,QObject* parent):_name(name),QObject(parent)
{

}

uint32_t DeviceInfo::GetBaudRate() const
{
    return _baud_rate;
}

const QByteArray& DeviceInfo::GetDeviceName() const
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
}

void DeviceInfo::SetFilamentMaterial(QByteArray material)
{
    _filament_material=material;
}

void DeviceInfo::SetNetworkID(QByteArray network)
{
    _network_id=network;
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
    return vmap;
}

QByteArray DeviceInfo::GetID()const{
    return _id;
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

void DeviceInfo::FromJSON(const QJsonObject &json)
{
    this->_name=json["name"].toString().toUtf8();
    this->_filament_material=json["material"].toString().toUtf8();
    this->_nozzle=json["nozzle"].toDouble();
    this->_x=json["x"].toInt();
    this->_y=json["y"].toInt();
    this->_z=json["z"].toInt();
    this->_id=json["_id"].toString().toUtf8();
    this->_baud_rate=json["baudrate"].toInt();
    this->_network_id=json["network"].toString().toUtf8();

}

DeviceInfo::~DeviceInfo()
{
}

DeviceInfo::operator QByteArray()const
{
    return this->_name;
}




