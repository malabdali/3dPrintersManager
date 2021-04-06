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
private slots:
    void WhenDeviceRemoved();

signals:

};

#endif // DEVICECOMPONENT_H
