#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include "../core/device.h"
namespace Ui {
class CameraWidget;
}

class CameraWidget : public QWidget
{
    Q_OBJECT
    Device* _device;
public:
    explicit CameraWidget(Device* device);
    ~CameraWidget();

private slots:
    void on__save_button_clicked();
    void WhenImageCaptured(QPixmap pixmap);
    void SetCurrentData();

private:
    Ui::CameraWidget *ui;

};

#endif // CAMERAWIDGET_H
