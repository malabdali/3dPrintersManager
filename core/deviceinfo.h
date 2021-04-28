#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>
#include <QNetworkReply>

class DeviceInfo:public QObject
{
    Q_OBJECT

private://fields
    QByteArray _name,_filament_material,_device_type,_device_model,_device_ip;
    uint32_t _baud_rate,_x,_y,_z,_status,_device_network_port;
    QByteArray _id,_network_id;
    QNetworkReply* _save_changes_reply;
    float _nozzle;


public:
    DeviceInfo(QByteArray name,QObject* parent=nullptr);
    uint32_t GetBaudRate()const;
    QByteArray GetDeviceName()const;
    uint32_t GetX()const;
    uint32_t GetY()const;
    uint32_t GetZ()const;
    QByteArray GetID()const;
    QByteArray GetDeviceType()const;
    QByteArray GetDeviceModel()const;
    QByteArray GetDeviceIP()const;
    int GetNetworkPort()const;
    float GetNozzleDiameter();
    QByteArray GetFilamentMaterial();
    QByteArray GetNetworkID();

    void SaveChanges();


    void SetBaudRate(uint32_t br);
    void SetDimensions(uint32_t x,uint32_t y,uint32_t z);
    void SetNozzleDiameter(float diameter);
    void SetFilamentMaterial(QByteArray material);
    void SetNetworkID(QByteArray);
    void SetDeviceType(QByteArray);
    void SetDeviceModel(QByteArray);
    void SetDeviceIP(QByteArray);
    void SetNetworkPort(int);

    bool operator==(const DeviceInfo&)const;
    bool operator!=(const DeviceInfo&)const;
    operator QByteArray()const;
    QByteArray ToJson()const;
    operator QVariantMap()const;
    void FromJSON(const QJsonObject& json);

    ~DeviceInfo();

    struct Types{
        const static QByteArray FDM,SLA;
    };


signals:
    void InfoChanged();
    void Saved(bool);

};

#endif // DEVICEINFO_H
