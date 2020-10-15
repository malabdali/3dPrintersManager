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
DeviceWidget::DeviceWidget(Device* device,QWidget *parent) :
    QWidget(parent),_device(device),ui(new Ui::DeviceWidget)
{
    ui->setupUi(this);
    Setup();
    _serial_widget=nullptr;
    _files_widget=nullptr;
}

void DeviceWidget::Update()
{
    if(_device)
    {
        if(_device->GetFileSystem()->IsStillUploading() && _device->IsReady())
        {
            ui->_status->setText("Uploading Files");
            qDebug()<<_device->GetFileSystem()->GetUploadProgress();
        }
        else if(_device->IsReady())
            ui->_status->setText("Ready");
        else if(!_device->IsReady() && _device->IsOpen() && !_device->GetProblemSolver()->IsThereProblem())
        {
            ui->_status->setText("port is open not ready");
            _device->UpdateDeviceStats();
        }
        else
            ui->_status->setText("port is closed");
    }
}

DeviceWidget::~DeviceWidget()
{
    delete ui;
}

void DeviceWidget::Setup()
{
    if(this->_device){
        ui->_name->setReadOnly(true);
        ui->_baud_rate->setText(QString::number(_device->GetDeviceInfo()->GetBaudRate()));
        ui->_name->setText(_device->GetDeviceInfo()->GetDeviceName());
        ui->_port->setText(this->_device->GetPort());
        ui->_x->setText(QString::number(_device->GetDeviceInfo()->GetX()));
        ui->_y->setText(QString::number(_device->GetDeviceInfo()->GetY()));
        ui->_z->setText(QString::number(_device->GetDeviceInfo()->GetZ()));
        ui->_nozzle->setText(QString::number(_device->GetDeviceInfo()->GetNozzleDiameter()));
        ui->_material->setCurrentText(_device->GetDeviceInfo()->GetFilamentMaterial());
        ui->_status->setVisible(true);
        ui->_status_label->setVisible(true);
        ui->_save_changes_button->setVisible(false);
        ui->_create_button->setVisible(false);
        ui->_delete_button->setVisible(true);
        ui->_detect_port_button->setVisible(true);
        ui->_open_port_button->setVisible(false);
        ui->_close_port_button->setVisible(false);
        ui->_error_label->setVisible(false);
        ui->_error->setVisible(false);

        // device events
        QObject::connect(this->_device->GetDeviceInfo(),&DeviceInfo::InfoChanged,this,&DeviceWidget::OnDeviceInfoChanged);
        QObject::connect(this->_device,&Device::CommandFinished,this,&DeviceWidget::OnCommandFinished);
        QObject::connect(this->_device,&Device::CommandStarted,this,&DeviceWidget::OnCommandStarted);
        QObject::connect(this->_device,&Device::DetectPortSucceed,this,&DeviceWidget::OnDetectPort);
        QObject::connect(this->_device,&Device::DetectPortFailed,this,&DeviceWidget::OnDetectPort);
        QObject::connect(this->_device,&Device::PortOpened,this,&DeviceWidget::OnPortConnected);
        QObject::connect(this->_device,&Device::PortClosed,this,&DeviceWidget::OnPortDisconnected);
        QObject::connect(this->_device,&Device::ErrorOccurred,this,&DeviceWidget::OnErrorOccured);
        QObject::connect(this->_device->GetProblemSolver(),&DeviceProblemSolver::ProblemDetected,this,&DeviceWidget::WhenProblemDetected);
        QObject::connect(this->_device->GetProblemSolver(),&DeviceProblemSolver::SolveFinished,this,&DeviceWidget::WhenSolveProblemFinished);

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

    }

    {
        //ui events
        QObject::connect(ui->_name,&QLineEdit::textEdited,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_baud_rate,&QLineEdit::textEdited,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_x,&QLineEdit::textEdited,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_y,&QLineEdit::textEdited,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_z,&QLineEdit::textEdited,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_nozzle,&QLineEdit::textEdited,this,&DeviceWidget::WhenEditValues);
        QObject::connect(ui->_material,&QComboBox::currentTextChanged,this,&DeviceWidget::WhenEditValues);
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
}

void DeviceWidget::OnCommandFinished(const GCodeCommand *function, bool b)
{

}

void DeviceWidget::OnCommandStarted(const GCodeCommand *function)
{

}

void DeviceWidget::OnErrorOccured(int error)
{
    _device->ClosePort();
    ui->_error_label->setVisible(true);
    ui->_error->setVisible(true);
    ui->_error->setText(QString::number(error));
}

void DeviceWidget::OnPortConnected()
{
    ui->_icon->setStyleSheet("background-color: rgb(20, 255, 20);");
    ui->_open_port_button->setVisible(false);
    ui->_close_port_button->setVisible(true);
    ui->_error_label->setVisible(false);
    ui->_error->setVisible(false);
    ui->_detect_port_button->setVisible(false);
}

void DeviceWidget::OnPortDisconnected()
{
    ui->_icon->setStyleSheet("background-color: rgb(255, 20, 20);");
    ui->_open_port_button->setVisible(true);
    ui->_close_port_button->setVisible(false);
    ui->_detect_port_button->setVisible(true);
}

void DeviceWidget::OnDetectPort()
{
    ui->_port->setText(this->_device->GetPort());
    ui->_detect_port_button->setVisible(true);
    if(!this->_device->GetPort().isEmpty())
    {
        //ui->_detect_port_button->setVisible(false);
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
}

void DeviceWidget::SaveChanges()
{
    qDebug()<<_device->thread()<<_device->GetDeviceInfo()->thread()<<this->thread()<<qApp->thread();
    ui->_save_changes_button->setEnabled(false);
    _device->GetDeviceInfo()->SetDimensions(ui->_x->text().toUInt(),ui->_y->text().toUInt(),ui->_z->text().toUInt());
    _device->GetDeviceInfo()->SetBaudRate(ui->_baud_rate->text().toUInt());
    _device->GetDeviceInfo()->SetNozzleDiameter(ui->_nozzle->text().toFloat());
    _device->GetDeviceInfo()->SetFilamentMaterial(ui->_material->currentText().toUtf8());

    _device->ClosePort();
    RemoteServer::GetInstance()->SendUpdateQuery([this](QNetworkReply* rep)->void{
        qDebug()<<rep->peek(rep->size());
        if(RemoteServer::GetInstance()->IsSuccess(rep))
        {
            ui->_save_changes_button->setVisible(false);
            _device->OpenPort();
        }
        else
            ui->_save_changes_button->setVisible(true);

        rep->deleteLater();
    },"Printers",*_device->GetDeviceInfo(),_device->GetDeviceInfo()->GetID());
}

void DeviceWidget::WhenProblemDetected()
{
    qDebug()<<"DeviceWidget::WhenProblemDetected";
    _device->GetProblemSolver()->SolveProblem();
}

void DeviceWidget::WhenSolveProblemFinished()
{
    qDebug()<<"DeviceWidget::WhenSolveProblemFinished";
}

void DeviceWidget::CreateDevice()
{
    DeviceInfo di(ui->_name->text().toUtf8());
    ui->_create_button->setEnabled(false);
    di.SetDimensions(ui->_x->text().toUInt(),ui->_y->text().toUInt(),ui->_z->text().toUInt());
    di.SetBaudRate(ui->_baud_rate->text().toUInt());
    di.SetNozzleDiameter(ui->_nozzle->text().toFloat());
    di.SetFilamentMaterial(ui->_material->currentText().toUtf8());
    RemoteServer::GetInstance()->SendInsertQuery([this](QNetworkReply* rep)->void{
        qDebug()<<rep->peek(rep->size());
        if(RemoteServer::GetInstance()->IsSuccess(rep))
        {
            DeviceInfo* device=new DeviceInfo(ui->_name->text().toUtf8());
            device->FromJSON(RemoteServer::GetInstance()->GetJSONValue(rep).toArray()[0].toObject());
            qDebug()<<device->ToJson();
            Devices::GetInstance()->AddDevice(device);

            ui->_create_button->setVisible(false);
        }
        else
            ui->_create_button->setVisible(true);

        rep->deleteLater();
    },"Printers",di);
}

void DeviceWidget::DeleteDevice()
{
    ui->_delete_button->setEnabled(false);
    RemoteServer::GetInstance()->SendDeleteQuery([this](QNetworkReply* rep)->void{
        qDebug()<<rep->peek(rep->size());
        if(RemoteServer::GetInstance()->IsSuccess(rep))
        {
            Devices::GetInstance()->RemoveDevice(_device->GetDeviceInfo());
        }
        else
            ui->_delete_button->setVisible(true);

        rep->deleteLater();
    },"Printers",_device->GetDeviceInfo()->GetID());
}

void DeviceWidget::DetectPort()
{
    this->_device->DetectDevicePort();
    ui->_detect_port_button->setVisible(false);
}

void DeviceWidget::OpenPort()
{
    this->_device->OpenPort();
        ui->_open_port_button->setVisible(false);

}

void DeviceWidget::ClosePort()
{
    this->_device->ClosePort();
}

void DeviceWidget::ShowContextMenu(const QPoint &pos)
{
    QMenu contextMenu(tr("Context menu"), this);

    if(_device->IsOpen())
    {
        contextMenu.addAction(ui->_files_action);
    }
    contextMenu.addAction(ui->_test_action);

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

void DeviceWidget::on__files_action_triggered(bool checked)
{
    //_device->AddFunction(DeviceFunctions::Function::FileList);

    if(_files_widget){
        _serial_widget->show();
        _serial_widget->raise();
        return;
    }
    _files_widget=new FilesSystemWidget(this->_device);
    _files_widget->setParent(this->window(),Qt::WindowType::Window);
    _files_widget->setWindowTitle(_device->GetDeviceInfo()->GetDeviceName()+ " : files");
    _files_widget->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    _files_widget->setWindowModality(Qt::WindowModality::WindowModal);
    _files_widget->show();
    QObject::connect(_files_widget,&QWidget::destroyed,this,&DeviceWidget::FilesWidgetClosed);
}

void DeviceWidget::on__test_action_triggered()
{
    //_device->AddGCodeCommand(new GCode::PrintingStatus(_device,[](bool b)->void{}));

    if(_serial_widget){
        _serial_widget->show();
        _serial_widget->raise();
        return;
    }
    _serial_widget=new SerialWidget(this);
    _serial_widget->setParent(this->window(),Qt::WindowType::Window);
    _serial_widget->setWindowTitle(_device->GetDeviceInfo()->GetDeviceName()+ " : serial");
    _serial_widget->setAttribute(Qt::WidgetAttribute::WA_DeleteOnClose);
    _serial_widget->setWindowModality(Qt::WindowModality::WindowModal);
    _serial_widget->show();
    QObject::connect(_serial_widget,&QWidget::destroyed,this,&DeviceWidget::FilesWidgetClosed);
}
