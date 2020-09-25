#ifndef DEVICEWIDGET_H
#define DEVICEWIDGET_H

#include <QWidget>
#include "../core/device.h"
#include <QMenu>
#include "./filessystemwidget.h"
namespace Ui {
class DeviceWidget;
}

class DeviceWidget : public QWidget
{
    Q_OBJECT
private://fields
    Device* _device;
    Ui::DeviceWidget *ui;
    FilesSystemWidget *_files_widget;

public:
    explicit DeviceWidget(Device* device,QWidget *parent = nullptr);
    void Update();
    ~DeviceWidget();

private://methods
    void Setup();
    void contextMenuEvent(QContextMenuEvent *event) override;
private slots:
    void OnDeviceInfoChanged();
    void OnCommandFinished(const GCodeCommand* function,bool b);
    void OnCommandStarted(const GCodeCommand* function);
    void OnErrorOccured(int);
    void OnPortConnected();
    void OnPortDisconnected();
    void OnDetectPort();
    void WhenEditValues();
    void SaveChanges();
    void CreateDevice();
    void DeleteDevice();
    void DetectPort();
    void OpenPort();
    void ClosePort();
    void ShowContextMenu(const QPoint &pos);
    void FilesWidgetClosed();
    void on__files_action_triggered(bool checked);
    void on__test_action_triggered();
};

#endif // DEVICEWIDGET_H
