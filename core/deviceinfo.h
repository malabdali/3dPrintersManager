#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutexLocker>

class DeviceInfo:public QObject
{
    Q_OBJECT

private://fields
    QByteArray _name,_filament_material;
    uint32_t _baud_rate,_x,_y,_z,_id,_status;
    float _nozzle;
    QMutex* _mutex;


public:
    DeviceInfo(QByteArray name,QObject* parent=nullptr);
    uint32_t GetBaudRate()const;
    const QByteArray& GetDeviceName()const;
    uint32_t GetX()const;
    uint32_t GetY()const;
    uint32_t GetZ()const;
    uint32_t GetID()const;
    float GetNozzleDiameter();
    QByteArray GetFilamentMaterial();


    void SetBaudRate(uint32_t br);
    void SetDimensions(uint32_t x,uint32_t y,uint32_t z);
    void SetNozzleDiameter(float diameter);
    void SetFilamentMaterial(QByteArray material);
    bool operator==(const DeviceInfo&)const;
    bool operator!=(const DeviceInfo&)const;
    operator QByteArray()const;
    QByteArray ToJson()const;
    operator QVariantMap()const;
    void FromJSON(const QJsonObject& json);

    ~DeviceInfo();


signals:
    void InfoChanged();

};

#endif // DEVICEINFO_H
