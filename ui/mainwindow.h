#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "../core/devices.h"
#include "../core/device.h"
#include "../core/devicefunctions.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
    DeviceFunctions* _dfunction;
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void WhenClickTestButton();
    void StopFunction();
    void WhenLineAvailabel(QByteArrayList ba);
};
#endif // MAINWINDOW_H
