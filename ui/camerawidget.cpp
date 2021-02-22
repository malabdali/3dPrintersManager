#include "camerawidget.h"
#include "ui_camerawidget.h"
#include "../core/camera.h"
#include "../core/remoteserver.h"

CameraWidget::CameraWidget(Device *device) :
    _device(device),
    ui(new Ui::CameraWidget)
{
    ui->setupUi(this);
    connect(_device->GetCamera(),&Camera::OnImageCaptured,this,&CameraWidget::WhenImageCaptured);
    ui->_camera_selector->addItem("");
    for(QCameraInfo& cam:QCameraInfo::availableCameras()){
        ui->_camera_selector->addItem(cam.deviceName());
    }
    SetCurrentData();
}

CameraWidget::~CameraWidget()
{
    delete ui;
}

void CameraWidget::on__save_button_clicked()
{
    _device->GetCamera()->SetCameraSetting(ui->_camera_selector->currentText().toUtf8(),ui->_x_offset->text().toInt(),ui->_y_offset->text().toInt(),ui->_x_to->text().toInt(),ui->_y_to->text().toInt());

    _device->GetCamera()->CaptureImage();
    _device->Save();
}

void CameraWidget::WhenImageCaptured(QPixmap pixmap)
{
    qDebug()<<"image captured";
    ui->_capture_window->setPixmap(pixmap);
    _device->GetCamera()->UploadImage();

}

void CameraWidget::SetCurrentData()
{
    if(!_device->GetCamera()->GetCameraInfo().isNull()){

        ui->_camera_selector->setCurrentText(_device->GetCamera()->GetCameraInfo().deviceName());
    }
    ui->_x_offset->setText(QString::number(_device->GetCamera()->GetCaptureOffset().first));
    ui->_y_offset->setText(QString::number(_device->GetCamera()->GetCaptureOffset().second));
    ui->_x_to->setText(QString::number(_device->GetCamera()->GetCaptureDimensions().first));
    ui->_y_to->setText(QString::number(_device->GetCamera()->GetCaptureDimensions().second));
}
