#include "devicewidget.h"
#include "ui_devicewidget.h"
#include "../core/remoteserver.h"
#include "../core/devices.h"
#include "../core/gcode/devicestats.h"
#include "../core/devicefilessystem.h"
#include <QApplication>
#include "../core/deviceinfo.h"
#include "serialwidget.h"
#include "../core/deviceproblemsolver.h"
#include "../core/gcode/printingstats.h"
#include "../core/devicemonitor.h"
#include "../core/deviceport.h"
#include "./printercontrol.h"
#include "../core/gcode/stopsdprint.h"
#include <QSpacerItem>
#include <QVBoxLayout>
#include "../core/system.h"
#include "../core/printcontroller.h"
#include "camerawidget.h"
#include "../core/deviceport.h"
#include "../core/gcode/settemperatures.h"
#include "../core/gcode/powersupply.h"
DeviceWidget::DeviceWidget(Device* device,QWidget *parent) :
    QWidget(parent),_device(device),ui(new Ui::DeviceWidget)
{
    ui->setupUi(this);
    Setup();
    _serial_widget=nullptr;
    _files_widget=nullptr;
    _printer_control_widget=nullptr;
    _camera_widget=nullptr;
    if(device){
        connect(device->GetDeviceMonitor(),&DeviceMonitor::updated,this,&DeviceWidget::WhenMonitorUpdated);
    }
}

void DeviceWidget::Update()
{
    if(_device)
    {
        ui->_uploading->setText("");
        ui->_uploading->setVisible(false);
        ui->_uploading_label->setVisible(false);
        if(_device->GetDeviceInfo()->GetDeviceType()==DeviceInfo::Types::FDM){
            if(_device->IsOpen() && _device->GetStatus()==Device::DeviceStatus::Ready && !_device->GetPrintController()->IsPrinting()){
                ui->_on_off_button->setVisible(true);
            }
            else{
                ui->_on_off_button->setVisible(false);
            }
        }
        if(_device->GetPrintController()->IsPrinting()  && _device->IsOpen() && _device->GetStatus()==Device::DeviceStatus::Ready &&
                !_device->GetDeviceMonitor()->IsBusy()  )
        {
            ui->_stop_print_button->setVisible(true);
            this->ui->_pause_print_button->setVisible(true);
        }
        else
        {
            ui->_stop_print_button->setVisible(false);
            this->ui->_pause_print_button->setVisible(false);
        }
        if(_device->GetFileSystem()->IsStillUploading() && _device->GetStatus()==Device::DeviceStatus::Ready)
        {
            ui->_uploading->setVisible(true);
            ui->_uploading_label->setVisible(true);
            ui->_status->setText("Uploading Files");
            ui->_uploading->setText(QString::number(_device->GetFileSystem()->GetUploadProgress(),'f',2)+"%");
        }
        else
        {
            auto status=_device->GetStatus();
            switch (status) {
            case Device::DeviceStatus::Ready:
                ui->_status->setText("Ready");
                break;
            case Device::DeviceStatus::Busy:
                ui->_status->setText("Busy");
                break;
            case Device::DeviceStatus::Connected:
                ui->_status->setText("open not ready");
                break;
            default:
                ui->_status->setText("closed");
                break;
            }
        }

        if(_device->GetProblemSolver()->IsThereProblem()){


            ui->_error->setText(_device->GetProblemSolver()->ErrorToText());
            ui->_error_label->setVisible(true);
            ui->_error->setVisible(true);
        }
        else{
            ui->_error_label->setVisible(false);
            ui->_error->setVisible(false);
        }
    }

}

DeviceWidget::~DeviceWidget()
{
    delete ui;
}

void DeviceWidget::Setup()
{
    if(this->_device){
        if(_device->GetDeviceInfo()->GetDeviceType()==DeviceInfo::Types::SLA){
            ui->_icon->setPixmap(QPixmap(":/icon/images/SLAPrinter.png"));
        }
        ui->_name->setReadOnly(true);
        ui->_baud_rate->setText(QString::number(_device->GetDeviceInfo()->GetBaudRate()));
        ui->_name->setText(_device->GetDeviceInfo()->GetDeviceName());
        ui->_device_model->setCurrentText(_device->GetDeviceInfo()->GetDeviceModel().isEmpty()?QStringLiteral("UNKNOWN"):QString(_device->GetDeviceInfo()->GetDeviceModel()));
        ui->_port->setText(this->_device->GetPort());
        ui->_x->setText(QString::number(_device->GetDeviceInfo()->GetX()));
        ui->_y->setText(QString::number(_device->GetDeviceInfo()->GetY()));
        ui->_z->setText(QString::number(_device->GetDeviceInfo()->GetZ()));
        ui->_ip_port->setText(QString("%1:%2").arg(QString::fromUtf8(_device->GetDeviceInfo()->GetDeviceIP())).arg(_device->GetDeviceInfo()->GetNetworkPort()));
        ui->_nozzle->setText(QString::number(_device->GetDeviceInfo()->GetNozzleDiameter()));
        ui->_material->setCurrentText(_device->GetDeviceInfo()->GetFilamentMaterial());
        ui->_connection_type->setCurrentIndex(_device->GetDeviceInfo()->GetConnectionType()-1);
        ui->_device_type->setCurrentText(_device->GetDeviceInfo()->GetDeviceType());
        ui->_status->setVisible(true);
        ui->_status_label->setVisible(true);
        ui->_save_changes_button->setVisible(false);
        ui->_create_button->setVisible(false);
        ui->_delete_button->setVisible(true);
        ui->_detect_port_button->setVisible(true);
        if( _device->GetDeviceInfo()->GetConnectionType()==DeviceInfo::ConnectionType::Serial)
            ui->_open_port_button->setVisible(false);
        ui->_close_port_button->setVisible(false);
        ui->_error_label->setVisible(false);
        ui->_error->setVisible(false);
        ui->_stop_print_button->setVisible(false);
        this->ui->_pause_print_button->setVisible(false);
        ui->_continue_print_button->setVisible(false);
        ui->_connection_type->setEnabled(false);
        ui->_connection_type_label->setEnabled(false);
        ui->_device_type->setEnabled(false);
        ui->_device_model->setEnabled(false);
        if(!(_device->GetDeviceMonitor()->GetMonitorOptions()&DeviceMonitor::HotendAndBedTemperature)){
            ui->_hotend_temperature_->setVisible(false);
            ui->_hotend_temperature_label->setVisible(false);
            ui->_bed_temperature->setVisible(false);
            ui->_bed_temperature_label->setVisible(false);
        }

        if(_device->GetDeviceInfo()->GetConnectionType()!=DeviceInfo::ConnectionType::Serial)
            ui->_detect_port_button->setVisible(false);


        // device events
        QObject::connect(this->_device->GetDeviceInfo(),&DeviceInfo::InfoChanged,this,&DeviceWidget::OnDeviceInfoChanged,Qt::ConnectionType::QueuedConnection);
        QObject::connect(this->_device,&Device::CommandFinished,this,&DeviceWidget::OnCommandFinished);
        QObject::connect(this->_device,&Device::CommandStarted,this,&DeviceWidget::OnCommandStarted);
        QObject::connect(this->_device,&Device::DetectPortSucceed,this,&DeviceWidget::OnDetectPort);
        QObject::connect(this->_device,&Device::DetectPortFailed,this,&DeviceWidget::OnDetectPort);
        QObject::connect(this->_device,&Device::Opened,this,&DeviceWidget::OnConnected);
        QObject::connect(this->_device,&Device::Closed,this,&DeviceWidget::OnDisconnected);
        QObject::connect(this->_device->GetProblemSolver(),&DeviceProblemSolver::ProblemDetected,this,&DeviceWidget::OnErrorOccured);
        QObject::connect(this->_device->GetProblemSolver(),&DeviceProblemSolver::SolveFinished,this,&DeviceWidget::OnErrorSolved);
        QObject::connect(this->_device->GetDeviceInfo(),&DeviceInfo::Saved,this,&DeviceWidget::WhenDeviceInfoSaved);
        QObject::connect(Devices::GetInstance(),&Devices::DeviceDeleted,this,&DeviceWidget::WhenDeviceDeleted);
    }
    else {
        ui->_status->setVisible(false);
        ui->_port->setVisible(false);
        ui->_status_label->setVisible(false);
        ui->_port_label->setVisible(false);
        ui->_name->setReadOnly(false);
        ui->_create_button->setVisible(false);
        ui->_save_changes_button->setVisible(false);
        ui->_delete_button->setVisible(false);
        ui->_detect_port_button->setVisible(false);
        ui->_open_port_button->setVisible(false);
        ui->_close_port_button->setVisible(false);
        ui->_error_label->setVisible(false);
        ui->_error->setVisible(false);
        ui->_hotend_temperature_->setVisible(false);
        ui->_hotend_temperature_label->setVisible(false);
        ui->_reset_button->setVisible(false);
        ui->_bed_temperature->setVisible(false);
        ui->_bed_temperature_label->setVisible(false);
        ui->_stop_print_button->setVisible(false);
        this->ui->_pause_print_button->setVisible(false);
        ui->_continue_print_button->setVisible(false);
        QObject::connect(Devices::GetInstance(),&Devices::DeviceCreated,this,&DeviceWidget::WhenDeviceCreated);

    }

    ui->_uploading->setVisible(false);
    ui->_uploading_label->setVisible(false);
    ui->_printing->setVisible(false);
    ui->_printing_label->setVisible(false);
    ui->_on_off_button->setVisible(false);

    {
        //ui events
        QObject::connect(ui->_name,&QLineEdit::textEdited,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_baud_rate,&QLineEdit::textEdited,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_x,&QLineEdit::textEdited,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_y,&QLineEdit::textEdited,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_z,&QLineEdit::textEdited,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_ip_port,&QLineEdit::textEdited,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_nozzle,&QLineEdit::textEdited,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_material,&QComboBox::currentTextChanged,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_device_model,&QComboBox::currentTextChanged,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_device_type,&QComboBox::currentTextChanged,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_connection_type,&QComboBox::currentTextChanged,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_save_changes_button,&QPushButton::clicked,this,&DeviceWidget::SaveChanges);
        QObject::connect(ui->_create_button,&QPushButton::clicked,this,&DeviceWidget::CreateDevice);
        QObject::connect(ui->_delete_button,&QPushButton::clicked,this,&DeviceWidget::DeleteDevice);
        QObject::connect(ui->_detect_port_button,&QPushButton::clicked,this,&DeviceWidget::DetectPort);
        QObject::connect(ui->_open_port_button,&QPushButton::clicked,this,&DeviceWidget::OpenPort);
        QObject::connect(ui->_close_port_button,&QPushButton::clicked,this,&DeviceWidget::ClosePort);
        this->setContextMenuPolicy(Qt::CustomContextMenu);

        connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
                this, SLOT(ShowContextMenu(const QPoint &)));
    }

    RefreshFieldsDisplay();


}

void DeviceWidget::RefreshFieldsDisplay()
{

    if(ui->_connection_type->currentIndex()==DeviceInfo::ConnectionType::Serial-1){
        ui->_port->setVisible(true);
        ui->_port_label->setVisible(true);
        ui->_baud_rate->setVisible(true);
        ui->_baud_rate_label->setVisible(true);
        ui->_ip_port_label->setVisible(false);
        ui->_ip_port->setVisible(false);
    }
    else{
        ui->_port->setVisible(false);
        ui->_port_label->setVisible(false);
        ui->_ip_port_label->setVisible(true);
        ui->_ip_port->setVisible(true);
        ui->_baud_rate->setVisible(false);
        ui->_baud_rate_label->setVisible(false);
    }

    if(ui->_device_type->currentText()==DeviceInfo::Types::FDM){
        ui->_nozzle->setVisible(true);
        ui->_nozzle_label->setVisible(true);
    }
    else{
        ui->_nozzle->setVisible(false);
        ui->_nozzle_label->setVisible(false);
    }

}

void DeviceWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QWidget::contextMenuEvent(event);
}

void DeviceWidget::OnDeviceInfoChanged()
{
    ui->_baud_rate->setText(QString::number(_device->GetDeviceInfo()->GetBaudRate()));
    ui->_name->setText(_device->GetDeviceInfo()->GetDeviceName());
    ui->_port->setText(this->_device->GetPort());
    ui->_x->setText(QString::number(_device->GetDeviceInfo()->GetX()));
    ui->_y->setText(QString::number(_device->GetDeviceInfo()->GetY()));
    ui->_z->setText(QString::number(_device->GetDeviceInfo()->GetZ()));
    ui->_ip_port->setText(QString("%1:%2").arg(QString::fromUtf8(_device->GetDeviceInfo()->GetDeviceIP())).arg(_device->GetDeviceInfo()->GetNetworkPort()));
}

void DeviceWidget::OnCommandFinished(const GCodeCommand *function, bool b)
{

}

void DeviceWidget::OnCommandStarted(const GCodeCommand *function)
{

}

void DeviceWidget::WhenDeviceCreated(Device *dev)
{
    if(dev){
        ui->_create_button->setVisible(false);
    }
    else
        ui->_create_button->setVisible(true);
}

void DeviceWidget::WhenDeviceDeleted(Device *dev)
{
    ui->_delete_button->setVisible(true);
}

void DeviceWidget::OnErrorOccured()
{
    ui->_error->setText(_device->GetProblemSolver()->ErrorToText());
    ui->_error_label->setVisible(true);
    ui->_error->setVisible(true);
}

void DeviceWidget::OnErrorSolved()
{
    ui->_error_label->setVisible(false);
    ui->_error->setVisible(false);
}

void DeviceWidget::OnConnected()
{
    ui->_icon->setStyleSheet("background-color: rgb(20, 255, 20);");
    ui->_open_port_button->setVisible(false);
    ui->_close_port_button->setVisible(true);
    ui->_error_label->setVisible(false);
    ui->_error->setVisible(false);
    ui->_detect_port_button->setVisible(false);
}

void DeviceWidget::OnDisconnected()
{
    ui->_icon->setStyleSheet("background-color: rgb(255, 20, 20);");
    ui->_open_port_button->setVisible(true);
    ui->_close_port_button->setVisible(false);
    if(_device->GetDeviceInfo()->GetConnectionType()==DeviceInfo::ConnectionType::Serial)
        ui->_detect_port_button->setVisible(true);
    if(_serial_widget)
        _serial_widget->close();
    if(_files_widget)
        _files_widget->close();
}

void DeviceWidget::OnDetectPort()
{
    ui->_port->setText(this->_device->GetPort());
    ui->_detect_port_button->setVisible(true);
    if(!this->_device->GetPort().isEmpty() || _device->GetDeviceInfo()->GetConnectionType()!=DeviceInfo::ConnectionType::Serial)
    {
        ui->_open_port_button->setVisible(true);
    }
    else
    {
        //
        ui->_open_port_button->setVisible(false);
    }

}

void DeviceWidget::WhenEditValues()
{
    if(_device)
    {
        ui->_save_changes_button->setVisible(true);
        ui->_save_changes_button->setEnabled(true);
    }
    else{
        if(ui->_name->text().length()>2){
            ui->_create_button->setVisible(true);
            ui->_create_button->setEnabled(true);
        }
        else{
            ui->_create_button->setVisible(false);
            ui->_create_button->setEnabled(false);
        }
    }

    RefreshFieldsDisplay();
}

void DeviceWidget::SaveChanges()
{
    ui->_save_changes_button->setEnabled(false);
    _device->GetDeviceInfo()->SetBaudRate(ui->_baud_rate->text().toUInt());
    _device->GetDeviceInfo()->SetDimensions(ui->_x->text().toUInt(),ui->_y->text().toUInt(),ui->_z->text().toUInt());
    _device->GetDeviceInfo()->SetNozzleDiameter(ui->_nozzle->text().toFloat());
    _device->GetDeviceInfo()->SetFilamentMaterial(ui->_material->currentText().toUtf8());
    _device->GetDeviceInfo()->SetDeviceModel(ui->_device_model->currentText().toUtf8());
    if(ui->_ip_port->text().contains(":")){
        _device->GetDeviceInfo()->SetDeviceIP(ui->_ip_port->text().split(':')[0].toUtf8());
        _device->GetDeviceInfo()->SetNetworkPort(ui->_ip_port->text().split(':')[1].toUInt());
    }
    _device->GetDeviceInfo()->SetConnctionType((DeviceInfo::ConnectionType)(ui->_connection_type->currentIndex()+1));
    _device->GetDeviceInfo()->SetDeviceType(ui->_device_type->currentText().toUtf8());

    _device->Close();
    _device->GetDeviceInfo()->SaveChanges();
}

void DeviceWidget::CreateDevice()
{
    DeviceInfo di(ui->_name->text().toUtf8());
    di.SetNetworkID(System::GetInstance()->GetNetworkID());
    ui->_create_button->setEnabled(false);
    di.SetDimensions(ui->_x->text().toUInt(),ui->_y->text().toUInt(),ui->_z->text().toUInt());
    di.SetBaudRate(ui->_baud_rate->text().toUInt());
    di.SetNozzleDiameter(ui->_nozzle->text().toFloat());
    di.SetFilamentMaterial(ui->_material->currentText().toUtf8());
    di.SetDeviceModel(ui->_device_model->currentText().toUtf8());
    if(ui->_ip_port->text().contains(":")){
        di.SetDeviceIP(ui->_ip_port->text().split(':')[0].toUtf8());
        di.SetNetworkPort(ui->_ip_port->text().split(':')[1].toUInt());
    }
    di.SetConnctionType((DeviceInfo::ConnectionType)(ui->_connection_type->currentIndex()+1));
    di.SetDeviceType(ui->_device_type->currentText().toUtf8());
    Devices::GetInstance()->CreateDevice(di);
}

void DeviceWidget::DeleteDevice()
{
    Devices::GetInstance()->DeleteDevice(*_device->GetDeviceInfo());
    ui->_delete_button->setEnabled(false);
}

void DeviceWidget::DetectPort()
{
    ui->_detect_port_button->setVisible(false);
    this->_device->DetectDevicePort();
}

void DeviceWidget::OpenPort()
{
    if(!_device->IsOpen())
        this->_device->Open();
    ui->_open_port_button->setVisible(false);

}

void DeviceWidget::ClosePort()
{
    this->_device->Close();
}

void DeviceWidget::WhenDeviceInfoSaved(bool res)
{
    if(res)
    {
        ui->_save_changes_button->setVisible(false);
        _device->Open();
    }
    else
        ui->_save_changes_button->setVisible(true);
}

void DeviceWidget::ShowContextMenu(const QPoint &pos)
{
    QMenu contextMenu(tr("Context menu"), this);

    if(_device->IsOpen())
    {
        contextMenu.addAction(ui->_files_action);
        contextMenu.addAction(ui->_gcode_action);
        contextMenu.addAction(ui->_control_action);
    }
    contextMenu.addAction(ui->_camera_settings_action);
    contextMenu.exec(mapToGlobal(pos));
}

void DeviceWidget::FilesWidgetClosed()
{
    _files_widget=nullptr;
}

void DeviceWidget::SerialWidgetClosed()
{
    _serial_widget=nullptr;
}

void DeviceWidget::WhenMonitorUpdated()
{
    if(_device->GetDeviceMonitor()->IsPrinting()){
        ui->_printing->show();
        ui->_printing_label->show();
        ui->_printing->setText(QString::number(_device->GetDeviceMonitor()->GetPrintProgress(),'f',2)+"%");
    }
    else{
        ui->_printing->hide();
        ui->_printing_label->hide();
    }
    if(_device->GetDeviceMonitor()->IsPaused() || (!_device->GetDeviceMonitor()->IsPrinting() && _device->GetDeviceMonitor()->IsWasPrinting())){
        ui->_reset_button->show();
    }
    else{
        ui->_reset_button->hide();
    }
    if(_device->GetPrintController()->CanContinuePrinting())
    {
        ui->_continue_print_button->show();
    }
    else{
        ui->_continue_print_button->hide();
    }
    ui->_bed_temperature->setText(QString::number(_device->GetDeviceMonitor()->GetBedTemperature()));
    ui->_hotend_temperature_->setText(QString::number(_device->GetDeviceMonitor()->GetHotendTemperature()));
}

void DeviceWidget::on__files_action_triggered(bool checked)
{
    //_device->AddFunction(DeviceFunctions::Function::FileList);

    if(_files_widget){
        _files_widget->show();
        _files_widget->raise();
        return;
    }
    _files_widget=new FilesSystemWidget(this->_device);
    _files_widget->setParent(this->window(),Qt::WindowType::Window);
    _files_widget->setWindowTitle(_device->GetDeviceInfo()->GetDeviceName()+ " : files");
    _files_widget->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    //_files_widget->setWindowModality(Qt::WindowModality::WindowModal);
    _files_widget->show();
    QObject::connect(_files_widget,&QWidget::destroyed,this,&DeviceWidget::FilesWidgetClosed);
}

void DeviceWidget::on__reset_button_clicked()
{
    ui->_reset_button->hide();
    _device->GetDeviceMonitor()->Reset();
}

void DeviceWidget::on__gcode_action_triggered()
{

    if(_serial_widget){
        _serial_widget->show();
        _serial_widget->raise();
        return;
    }
    _serial_widget=new SerialWidget(_device,this);
    _serial_widget->setParent(this->window(),Qt::WindowType::Window);
    _serial_widget->setWindowTitle(_device->GetDeviceInfo()->GetDeviceName()+ " : serial");
    _serial_widget->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    //_serial_widget->setWindowModality(Qt::WindowModality::WindowModal);
    _serial_widget->show();
    QObject::connect(_serial_widget,&QWidget::destroyed,this,&DeviceWidget::SerialWidgetClosed);
}

void DeviceWidget::on__control_action_triggered()
{

    if(_printer_control_widget){
        _printer_control_widget->show();
        _printer_control_widget->raise();
        return;
    }
    _printer_control_widget=new PrinterControl(this->_device);
    _printer_control_widget->setParent(this->window(),Qt::WindowType::Window);
    _printer_control_widget->setWindowTitle(_device->GetDeviceInfo()->GetDeviceName()+ " controller");
    _printer_control_widget->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    //_printer_control_widget->setWindowModality(Qt::WindowModality::WindowModal);
    _printer_control_widget->show();
    QObject::connect(_printer_control_widget,&QWidget::destroyed,this,[this]{_printer_control_widget=nullptr;});
}

void DeviceWidget::on__stop_print_button_clicked()
{
    _device->GetPrintController()->StopPrint();
    this->ui->_stop_print_button->setVisible(false);
    this->ui->_pause_print_button->setVisible(false);
}

void DeviceWidget::on__continue_print_button_clicked()
{
    _device->GetPrintController()->ContinuePrint();
}

void DeviceWidget::on__camera_settings_action_triggered()
{
    if(_camera_widget){
        _camera_widget->show();
        _camera_widget->raise();
        return;
    }
    _camera_widget=new CameraWidget(this->_device);
    _camera_widget->setParent(this->window(),Qt::WindowType::Window);
    _camera_widget->setWindowTitle(_device->GetDeviceInfo()->GetDeviceName()+ " camera settings");
    _camera_widget->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    //_camera_widget->setWindowModality(Qt::WindowModality::WindowModal);
    _camera_widget->show();
    QObject::connect(_camera_widget,&QWidget::destroyed,this,[this]{_camera_widget=nullptr;});
}

void DeviceWidget::on__pause_print_button_clicked()
{
    _device->GetPrintController()->PausePrint();
    this->ui->_stop_print_button->setVisible(false);
    this->ui->_pause_print_button->setVisible(false);
}


void DeviceWidget::on__on_off_button_toggled(bool checked)
{
    if(checked){
        GCode::SetTemperatures* temp=new GCode::SetTemperatures(this->_device,-1,180);
        this->_device->AddGCodeCommand(temp);
    }
    else{
        GCode::SetTemperatures* temp=new GCode::SetTemperatures(this->_device,0,0);
        this->_device->AddGCodeCommand(temp);
    }
}
