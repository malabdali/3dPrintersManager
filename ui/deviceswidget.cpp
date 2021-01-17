#include "deviceswidget.h"
#include "ui_deviceswidget.h"
DevicesWidget::DevicesWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DevicesWidget)
{
    ui->setupUi(this);
    this->startTimer(UPDATE_DEVICES_GUI_TIMER,Qt::CoarseTimer);
    QObject::connect(Devices::GetInstance(),&Devices::DevicesLoaded,this,&DevicesWidget::OnDeviccesLoaded);
    QObject::connect(Devices::GetInstance(),&Devices::DeviceAdded,this,&DevicesWidget::OnDeviceAdded);
    QObject::connect(Devices::GetInstance(),&Devices::DeviceRemoved,this,&DevicesWidget::OnDeviceRemoved);
    QObject::connect(ui->_add_printer_button,&QPushButton::clicked,this,&DevicesWidget::OnAddDeviceButtonClicked);
}

DevicesWidget::~DevicesWidget()
{
    delete ui;
}

DeviceWidget *DevicesWidget::GetDeviceWidget(Device *device)
{
    return _devices_widgets[device];
}

void DevicesWidget::RemoveDeviceWidget(Device *device)
{
    if(!_devices_widgets.contains(device))
        return ;
    _devices_widgets[device]->close();
    _devices_widgets.remove(device);
}

void DevicesWidget::RemoveAllDevices()
{
    for(Device* dw: _devices_widgets.keys())
        RemoveDeviceWidget(dw);
}

void DevicesWidget::Update()
{
    for(DeviceWidget* dw:_devices_widgets.values()){
        dw->Update();
    }
}

void DevicesWidget::LoadDevices()
{

    this->RemoveAllDevices();
    Devices::GetInstance()->Clear();
    Devices::GetInstance()->LoadDevicesFromRemoteServer();

}

void DevicesWidget::timerEvent(QTimerEvent *event){
    Update();
}

void DevicesWidget::OnDeviccesLoaded(bool b)
{
    if(b)
    {
        for(Device* &dev:Devices::GetInstance()->GetAllDevices()){
            if(!_devices_widgets.contains(dev))
                this->AddDeviceWidget(dev);
        }

        Devices::GetInstance()->DetectPortAndConnectForAllDevices();
    }

}

void DevicesWidget::OnAddDeviceButtonClicked()
{
    this->AddDeviceWidget(nullptr);
}

void DevicesWidget::OnDeviceAdded(Device *dev)
{
    if(!_devices_widgets.contains(dev))
        this->AddDeviceWidget(dev);
    RemoveDeviceWidget(nullptr);
}

void DevicesWidget::OnDeviceRemoved(Device *dev)
{
    RemoveDeviceWidget(dev);
}

DeviceWidget* DevicesWidget::AddDeviceWidget(Device *device)
{
    if(_devices_widgets.contains(device))
        return GetDeviceWidget(device);

    QVBoxLayout* layout=(QVBoxLayout*)this->ui->_devices_list->layout();
    DeviceWidget* widget=new DeviceWidget(device,this);
    layout->insertWidget(0,widget);
    this->ui->_devices_list->layout()->setAlignment(widget,Qt::AlignTop);
    this->_devices_widgets.insert(device,widget);
    /*QSpacerItem* oldSpacer=layout->spacerItem();
    if(oldSpacer!=nullptr)
    {
        layout->removeItem(oldSpacer);
        delete oldSpacer;
    }
    QSpacerItem *spacer=new QSpacerItem(0,200,QSizePolicy::Minimum,QSizePolicy::Expanding);
    layout->addSpacerItem(spacer);*/
    return widget;
}

