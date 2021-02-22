#include "camerasmanager.h"
#include <QApplication>
#include "devices.h"
#include "../config.h"
CamerasManager* CamerasManager::_instance=nullptr;
CamerasManager::CamerasManager(QObject *parent) : QObject(parent)
{
    _start_capture=false;
    _current_camera=-1;
    _is_played=false;
    _cpture_timer_id=-1;
}

void CamerasManager::StartCaptureAllDevicesCameras()
{
    _cameras.clear();
    _current_camera=-1;
    for(Device* dev: Devices::GetInstance()->GetAllDevices()){
        Camera* cam=dev->GetCamera();
        if(!cam->GetCameraInfo().isNull()){
            _cameras.push_back(cam);
        }
    }
    this->_start_capture=true;
    StartNextCapture();

    qDebug()<<"CamerasManager::StartCaptureAllDevicesCameras"<<_cameras.length();
}

void CamerasManager::Play()
{
    if(_is_played)
        return;
    qDebug()<<"CamerasManager::Play";
    _is_played=true;
    this->_cpture_timer_id=this->startTimer(CAPTURE_ALL_CAMERAS_TIMER);
}

void CamerasManager::Pause()
{
    if(_is_played)
    {
        killTimer(_cpture_timer_id);
        _cpture_timer_id=-1;
    }
    _is_played=false;
}

bool CamerasManager::IsPlayed() const
{
    return _is_played;
}

void CamerasManager::StartNextCapture()
{

    _current_camera++;
    if((int)_cameras.size()-1<_current_camera||!_is_played){
        _start_capture=false;
        emit CapturesFinished();
    }
    else {
        connect(_cameras[_current_camera],&Camera::OnImageCaptured,this,&CamerasManager::WhenImageCaptured);
        connect(_cameras[_current_camera],&Camera::OnCaptureFailed,this,&CamerasManager::WhenCaptureFailed);
        connect(_cameras[_current_camera],&Camera::ImageUploaded,this,&CamerasManager::WhenImageUploaded);

        _cameras[_current_camera]->CaptureImage();

    }
}

void CamerasManager::timerEvent(QTimerEvent *event)
{
    if(event->timerId()==_cpture_timer_id){
        if(!this->_start_capture)
            this->StartCaptureAllDevicesCameras();
    }
}

void CamerasManager::WhenDeviceAdded(Device *device)
{

}

void CamerasManager::WhenDeviceRemoved(Device *device)
{
    if(_cameras.contains(device->GetCamera())){
        if(_cameras.indexOf(device->GetCamera())>_current_camera){
            _cameras.removeAll(device->GetCamera());
        }
        else if(_cameras.indexOf(device->GetCamera())==_current_camera){
            StartNextCapture();
        }
    }
}

void CamerasManager::WhenImageCaptured(QPixmap pixmap)
{
    _cameras[_current_camera]->UploadImage();
}

void CamerasManager::WhenImageUploaded(bool b)
{
    disconnect(_cameras[_current_camera],&Camera::OnImageCaptured,this,&CamerasManager::WhenImageCaptured);
    disconnect(_cameras[_current_camera],&Camera::OnCaptureFailed,this,&CamerasManager::WhenCaptureFailed);
    disconnect(_cameras[_current_camera],&Camera::ImageUploaded,this,&CamerasManager::WhenImageUploaded);
    StartNextCapture();
}

void CamerasManager::WhenCaptureFailed()
{

    disconnect(_cameras[_current_camera],&Camera::OnImageCaptured,this,&CamerasManager::WhenImageCaptured);
    disconnect(_cameras[_current_camera],&Camera::OnCaptureFailed,this,&CamerasManager::WhenCaptureFailed);
    disconnect(_cameras[_current_camera],&Camera::ImageUploaded,this,&CamerasManager::WhenImageUploaded);
    StartNextCapture();
}

CamerasManager *CamerasManager::GetInstance()
{
    if(_instance==nullptr)
    {
        _instance=new CamerasManager(qApp);
    }
    return _instance;


}
