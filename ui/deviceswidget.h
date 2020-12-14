#ifndef DEVICESWIDGET_H
#define DEVICESWIDGET_H

#include <QWidget>
#include "devicewidget.h"
#include "QMap"
#include "../config.h"
#include "../core/devices.h"
namespace Ui {
class DevicesWidget;
}

class DevicesWidget : public QWidget
{
    Q_OBJECT
private://fields
    QMap<class Device*,DeviceWidget*> _devices_widgets;
    Ui::DevicesWidget *ui;
    QTimerEvent* _update_timer;

public:
    explicit DevicesWidget(QWidget *parent = nullptr); 
    ~DevicesWidget();
    DeviceWidget *AddDeviceWidget(Device* device);
    DeviceWidget *GetDeviceWidget(Device* device);
    void RemoveDeviceWidget(Device* device);
    void RemoveAllDevices();
    void Update();
    void LoadDevices();
protected:
private://methods
    void timerEvent(QTimerEvent *event)override;
private slots:
    void OnDeviccesLoaded(bool b);
    void OnAddDeviceButtonClicked();
    void OnDeviceAdded(Device* dev);
    void OnDeviceRemoved(Device* dev);
};

#endif // DEVICESWIDGET_H
