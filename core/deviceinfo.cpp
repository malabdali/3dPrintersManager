#include "deviceinfo.h"
#include <QString>

DeviceInfo::DeviceInfo(QByteArray name):_name(name)
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
}

bool DeviceInfo::operator==(const DeviceInfo &dev)
{
    return dev._name==this->_name;
}

bool DeviceInfo::operator!=(const DeviceInfo &dev)
{
    return dev._name!=this->_name;
}

QByteArray DeviceInfo::ToJson() const
{
    QVariantMap vmap;
    vmap.insert("name",this->_name);
    vmap.insert("x",this->_x);
    vmap.insert("y",this->_y);
    vmap.insert("z",this->_z);
    vmap.insert("baudrate",this->_baud_rate);
    QJsonDocument jdocument(QJsonObject::fromVariantMap(vmap));
    return jdocument.toJson();
}

void DeviceInfo::FromJSON(const QByteArray &json)
{
    QJsonDocument JDoc=QJsonDocument::fromJson(json);
    this->_name=JDoc.object()["name"].toString().toUtf8();
    this->_material=JDoc.object()["material"].toString().toUtf8();
    this->_x=JDoc.object()["x"].toString().toUInt();
    this->_y=JDoc.object()["y"].toString().toUInt();
    this->_z=JDoc.object()["z"].toString().toUInt();
    this->_id=JDoc.object()["id"].toString().toUInt();
    this->_baud_rate=JDoc.object()["baudrate"].toString().toUInt();

}

DeviceInfo::operator QByteArray()
{
    return this->_name;
}




