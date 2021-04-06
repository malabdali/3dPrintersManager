#include "devicecomponent.h"
#include "device.h"

DeviceComponent::DeviceComponent(Device *device) : QObject(device),_device(device)
{
    connect(device,&Device::DeviceRemoved,this,&DeviceComponent::WhenDeviceRemoved);
}

Device *DeviceComponent::GetDevice()
{
    return _device;
}

void DeviceComponent::WhenDeviceRemoved()
{
    this->Disable();
}
