#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../core/devices.h"
#include "../core/device.h"
#include "deviceswidget.h"
#include "./taskswidget.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
private:// fields
    DevicesWidget* _devices_widget;
    TasksWidget* _tasks_widget;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void WhenNetworkInfoActionClicked();
    void WhenTaskManagerChanged();
    void WhensShowTasksTrigerred();

    void on_actioncapture_toggled(bool arg1);

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
