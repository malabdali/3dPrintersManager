#ifndef FILESSYSTEMWIDGET_H
#define FILESSYSTEMWIDGET_H

#include <QWidget>
#include "../core/device.h"
namespace Ui {
class FilesSystemWidget;
}

class FilesSystemWidget : public QWidget
{
    Q_OBJECT
private://fields
    Device* _device;
public:
    explicit FilesSystemWidget(Device* device,QWidget *parent = nullptr);
    ~FilesSystemWidget();

private slots:
    void on__update_files_button_clicked();
    void OnFileListUpdated();

    void on__files_list_itemSelectionChanged();

    void on__delete_file_button_clicked();

    void on__upload_file_button_clicked();

    void on__stop_upload_button_clicked();

private:
    Ui::FilesSystemWidget *ui;

};

#endif // FILESSYSTEMWIDGET_H
