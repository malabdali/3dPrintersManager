#include "camera.h"
#include <algorithm>
#include <map>
#include <QJsonObject>
#include <QJsonDocument>
#include "devicefilessystem.h"
#include "remoteserver.h"
#include "deviceinfo.h"
#include "../config.h"
std::map<QByteArray,Camera::CapturedImage> Camera::_captured_images=std::map<QByteArray,Camera::CapturedImage>();
Camera::CapturedImage Camera::GetCapture(QByteArray deviceName)
{
    if(_captured_images.find(deviceName)!=_captured_images.end())
        return _captured_images[deviceName];
    else
        return CapturedImage();

}

void Camera::SetCapture(QByteArray deviceName, QImage image)
{
    if(_captured_images.find(deviceName)!=_captured_images.end())
        _captured_images.erase(deviceName);
    _captured_images.emplace(std::make_pair(deviceName,Camera::CapturedImage{deviceName,QDateTime::currentDateTime(),image}));
}

Camera::Camera(Device *device) : DeviceComponent(device)
{
    _camera=nullptr;
    _image_capture=nullptr;
    _want_capture=false;
    _saved=true;
    _upload_reply=nullptr;
    _capture_offset={0,0};
    _capture_width_height={0,0};

}

void Camera::CaptureImage()
{
    CapturedImage capture=GetCapture(this->_camera_info.deviceName().toUtf8());
    if(!capture.camera.isEmpty()){
        if(capture.time>this->_last_capture_date)
        {
            _want_capture=true;
            WhenImageCaptured(0,capture.image);
            return;
        }
    }
    if(_camera->availability()==QMultimedia::AvailabilityStatus::Available && _image_capture->availability()==QMultimedia::AvailabilityStatus::Available)
    {
        _camera->start();
        _camera->searchAndLock();
        _image_capture->capture();
        _want_capture=true;
    }
    else {
        EndCapture(false);
    }
}

void Camera::Close()
{
    if(_camera)
    {
        _camera->unlock();
        _camera->unload();
        _camera->stop();
    }

}

QCameraInfo Camera::GetCameraInfo() const{
    return _camera_info;
}

QPair<int, int> Camera::GetCaptureOffset() const{
    return _capture_offset;
}

QPair<int, int> Camera::GetCaptureDimensions() const{
    return _capture_width_height;
}

void Camera::UploadImage()
{
    _upload_reply=RemoteServer::GetInstance()->UploadImage(_device->GetFileSystem()->GetLocaleDirectory(CAPTURE_CAMERA_FILE_NAME),
                                                           QString(CAPTURE_CAMERA_UPLOAD_PATH).arg(QString(_device->GetDeviceInfo()->GetDeviceName())),
                                                           [this](QNetworkReply* reply)->void{
            if(RemoteServer::GetInstance()->IsSuccess(reply)){
            ImageUploaded(true);
}
            else
            ImageUploaded(false);
            reply->deleteLater();
            _upload_reply=nullptr;
});
}

void Camera::SetCameraSetting(QByteArray ba,int xoffset,int yoffset,int width,int height)
{
    if(ba.isEmpty())return;
    _capture_offset=QPair(xoffset,yoffset);
    _capture_width_height=QPair(width,height);
    if(_camera){
        Close();
        _camera->deleteLater();
    }
    QList<QCameraInfo> cameras=QCameraInfo::availableCameras();
    auto cam=std::find_if(cameras.begin(),cameras.end(),[this,&ba](QCameraInfo& cam)->bool{return cam.deviceName().toUtf8()==ba;});
    if(cam!=cameras.end()){
        _camera_info=*cam;
        _camera=new QCamera(_camera_info,this);
        _image_capture = new QCameraImageCapture(_camera,_camera);
        _camera->setCaptureMode(QCamera::CaptureStillImage);

        QObject::connect( _image_capture, (void(QCameraImageCapture::*)(int,QImage))&QCameraImageCapture::imageCaptured, this,&Camera::WhenImageCaptured);
        QObject::connect( _camera, (void(QCamera::*)(bool))&QCamera::lockFailed,this,&Camera::WhenLockFailed);
        QObject::connect( _camera,(void(QCamera::*)(bool))&QCamera::availabilityChanged,this,&Camera::WhenAvailablityChanged);
        QObject::connect( _camera, static_cast<void(QCamera::*)(QCamera::Error)>(&QCamera::error),this,&Camera::WhenErrorAccured);
    }

}

QJsonDocument Camera::ToJson() const
{
    QVariantHash vh;
    vh.insert("CAMERA",_camera_info.deviceName().toUtf8());
    vh.insert("XOFFSET",_capture_offset.first);
    vh.insert("YOFFSET",_capture_offset.first);
    vh.insert("WIDTH",_capture_width_height.first);
    vh.insert("HEIGHT",_capture_width_height.first);
    vh.insert("LAST_CAPTURE_DATE",this->_last_capture_date.toString(Qt::DateFormat::ISODateWithMs));
    return QJsonDocument(QJsonObject::fromVariantHash(vh));
}

void Camera::FromJson(QJsonDocument json)
{
    int xoffset=0;
    int yoffset=0;
    int width=1;
    int height=1;
    QByteArray name="";
    if(json.object().contains("CAMERA")){
        name=json.object()["CAMERA"].toString().toUtf8();
    }
    if(json.object().contains("XOFFSET")){
        xoffset=json.object()["XOFFSET"].toInt();
        yoffset=json.object()["YOFFSET"].toInt();
    }
    if(json.object().contains("WIDTH")){
        width=json.object()["WIDTH"].toInt();
        height=json.object()["HEIGHT"].toInt();
    }
    if(json.object().contains("LAST_CAPTURE_DATE")){
        this->_last_capture_date=QDateTime::fromString(json.object()["CAMERA"].toString(),Qt::DateFormat::ISODateWithMs);
    }
    if(!name.isEmpty())this->SetCameraSetting(name,xoffset,yoffset,width,height);
}

void Camera::Save()
{
    _device->AddData("CAMERA",ToJson().object());
}

void Camera::Load()
{
    QJsonDocument documetn=QJsonDocument(_device->GetData("CAMERA"));
    FromJson(documetn);
}

Camera::~Camera()
{
    Close();

}


void Camera::Setup()
{
    _last_capture_date=QDateTime(QDate(2000,1,1),QTime(1,1));
    connect(_device,&Device::BeforeSaveDeviceData,this,&Camera::Save);
    connect(_device,&Device::DeviceDataLoaded,this,&Camera::Load);
    connect(_device,&Device::DeviceRemoved,this,&Camera::WhenDeviceRemoved);
}

void Camera::WhenImageCaptured(int id, QImage image)
{
    if(id>0){
        SetCapture(this->_camera_info.deviceName().toUtf8(),image);
    }
    int width=_capture_width_height.first<1?image.width():_capture_width_height.first;
    int height=_capture_width_height.second<1?image.height():_capture_width_height.second;
    this->_pixmap=QPixmap::fromImage(image.copy(_capture_offset.first,_capture_offset.second,_capture_width_height.first,_capture_width_height.second));
    this->_last_capture_date=QDateTime::currentDateTime();
    this->_saved=false;
    this->_pixmap.save(_device->GetFileSystem()->GetLocaleDirectory(CAPTURE_CAMERA_FILE_NAME),"jpg");
    EndCapture(true);
}

void Camera::WhenErrorAccured(QCamera::Error)
{

}

void Camera::WhenAvailablityChanged(bool b)
{

}

void Camera::WhenLockFailed()
{
    if(_want_capture){
        EndCapture(false);
    }
}

void Camera::WhenDeviceRemoved()
{
    this->Close();
    if(_upload_reply)
    {
        RemoteServer::GetInstance()->RemoveRequest(_upload_reply);
    }

}

void Camera::EndCapture(bool s)
{
    this->Close();
    if(s){
        emit OnImageCaptured(_pixmap);
    }
    else{
        emit OnCaptureFailed();
    }
    _want_capture=false;
}
