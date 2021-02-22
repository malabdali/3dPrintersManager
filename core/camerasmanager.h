#ifndef CAMERASMANAGER_H
#define CAMERASMANAGER_H

#include <QObject>
#include "device.h"
#include "devices.h"
#include "camera.h"
#include <QTimerEvent>

class CamerasManager : public QObject
{
    Q_OBJECT
    QList<Camera*> _cameras;
    bool _start_capture,_is_played;
    int _current_camera;
    int _cpture_timer_id;
public:
    explicit CamerasManager(QObject *parent = nullptr);
    void StartCaptureAllDevicesCameras();
    void Play();
    void Pause();
    bool IsPlayed()const;

protected:

private://methods
    void StartNextCapture();
    void timerEvent(QTimerEvent* event) override;

signals:
    void CapturesFinished();

private slots:
    void WhenDeviceAdded(Device* device);
    void WhenDeviceRemoved(Device* device);
    void WhenImageCaptured(QPixmap pixmap);
    void WhenImageUploaded(bool);
    void WhenCaptureFailed();

public://static
    static CamerasManager* GetInstance();
private://static
    static CamerasManager *_instance;

};

#endif // CAMERASMANAGER_H
