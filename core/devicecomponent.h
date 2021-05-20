#ifndef DEVICECOMPONENT_H
#define DEVICECOMPONENT_H

#include <QObject>

class DeviceComponent : public QObject
{
    Q_OBJECT
protected:
    class Device* _device;
public:
    explicit DeviceComponent(class Device* device);
    virtual void Setup()=0;
    virtual void Disable()=0;
    Device* GetDevice();
    virtual QJsonDocument ToJson()=0;
    virtual void FromJson(QJsonDocument json)=0;
    virtual void Save()=0;
    virtual void Load()=0;
private slots:
    void WhenDeviceRemoved();

signals:

};

#endif // DEVICECOMPONENT_H
