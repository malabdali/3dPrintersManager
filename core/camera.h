#ifndef CAMERA_H
#define CAMERA_H

#include <QObject>
#include <QCamera>
#include <QCameraInfo>
#include <QCameraImageCapture>
#include "device.h"
#include <QPixmap>
#include "devicecomponent.h"
#include <QDateTime>
#include <QNetworkReply>
#include <map>

class Camera : public DeviceComponent
{
    Q_OBJECT
private:// fields
    QCamera* _camera;
    QCameraImageCapture* _image_capture;
    QPixmap _pixmap;
    QPair<int,int> _capture_offset,_capture_width_height;
    QCameraInfo _camera_info;
    bool _want_capture,_saved;
    QDateTime _last_capture_date;
    QNetworkReply* _upload_reply;
public://types
    struct CapturedImage{
        QByteArray camera;
        QDateTime time;
        QImage image;
    };
private://static
    static std::map<QByteArray,Camera::CapturedImage> _captured_images;
    static CapturedImage GetCapture(QByteArray deviceName);
    static void SetCapture(QByteArray deviceName,QImage image);
public:
    explicit Camera(Device* device);
    void CaptureImage();
    void Close();
    void SetCameraSetting(QByteArray ba,int xoffset,int yoffset,int width,int height);
    QCameraInfo GetCameraInfo()const;
    QPair<int,int> GetCaptureOffset()const;
    QPair<int,int> GetCaptureDimensions()const;
    void UploadImage();

    QJsonDocument ToJson() const;
    void FromJson(QJsonDocument json);
    void Save();
    void Load();
    ~Camera();


signals:
    void OnImageCaptured(QPixmap);
    void OnCaptureFailed();
    void ImageUploaded(bool);
    // DeviceComponent interface
public:
    void Setup() override;

private slots:
    void WhenImageCaptured(int,QImage);
    void WhenErrorAccured(QCamera::Error);
    void WhenAvailablityChanged(bool);
    void WhenLockFailed();
    void WhenDeviceRemoved();

private:
    void EndCapture(bool);




};

#endif // CAMERA_H
