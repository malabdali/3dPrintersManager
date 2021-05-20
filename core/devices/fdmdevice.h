#ifndef FDMDEVICE_H
#define FDMDEVICE_H

#include "../device.h"
#include "fdmdevicemonitor.h"
#include "fdmprintcontroller.h"
#include "fdmfilessystem.h"

class FDMDevice:public Device
{
public:
    FDMDevice(DeviceInfo* device_info,QObject *parent = nullptr);

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
    FDMFilesSystem *GetFileSystem() const override;
    FDMDeviceMonitor *GetDeviceMonitor() override;
};

#endif // FDMDEVICE_H
