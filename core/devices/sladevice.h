#ifndef SLADEVICE_H
#define SLADEVICE_H

#include "../device.h"
#include "sladevicemonitor.h"
#include "slafilessystem.h"
class SLADevice:public Device
{
public:
    SLADevice(DeviceInfo* device_info,QObject *parent = nullptr);

    // Device interface
protected:
    void Setup() override;

    // Device interface
public:
    void UpdateDeviceStats() override;

protected slots:
    void WhenStatsUpdated() override;

    // Device interface
public:
    SLAFilesSystem *GetFileSystem() const override;
    SLADeviceMonitor *GetDeviceMonitor() override;
};

#endif // SLADEVICE_H
