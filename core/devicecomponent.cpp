#include "devicecomponent.h"
#include "device.h"

DeviceComponent::DeviceComponent(Device *device) : QObject(device),_device(device)
{

}

Device *DeviceComponent::GetDevice()
{
    return _device;
}
