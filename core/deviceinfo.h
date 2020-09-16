#ifndef DEVICEINFO_H
#define DEVICEINFO_H

#include <QObject>
#include <QJsonDocument>
#include <QJsonObject>

class DeviceInfo
{
private://fields
    QByteArray _name,_material;
    uint32_t _baud_rate,_x,_y,_z,_id,_status;


public:
    DeviceInfo(QByteArray name);
    uint32_t GetBaudRate()const;
    const QByteArray& GetDeviceName()const;
    void SetBaudRate(uint32_t br);
    bool operator==(const DeviceInfo&);
    bool operator!=(const DeviceInfo&);
    operator QByteArray();
    QByteArray ToJson()const;
    void FromJSON(const QByteArray& json);


signals:

};

#endif // DEVICEINFO_H
