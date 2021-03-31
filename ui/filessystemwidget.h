#ifndef FILESSYSTEMWIDGET_H
#define FILESSYSTEMWIDGET_H

#include <QWidget>
#include "../core/device.h"
#include "../core/gcodecommand.h"
namespace Ui {
class FilesSystemWidget;
}

class FilesSystemWidget : public QWidget
{
    Q_OBJECT
private://fields
    Device* _device;
    QByteArray _wanted_order;
public:
    explicit FilesSystemWidget(Device* device,QWidget *parent = nullptr);
    ~FilesSystemWidget();
    void timerEvent(QTimerEvent* event)override;
    void CheckSdSupport();



private slots:
    void on__update_files_button_clicked();
    void OnFileListUpdated();

    void on__files_list_itemSelectionChanged();

    void on__delete_file_button_clicked();

    void on__upload_file_button_clicked();

    void on__stop_upload_button_clicked();

    void on__print_button_clicked();

    void WhenSdSupportChanged(bool b);

    void WhenUpdateDeviceStatsFailed(GCodeCommand*);

private:
    Ui::FilesSystemWidget *ui;

};

#endif // FILESSYSTEMWIDGET_H
