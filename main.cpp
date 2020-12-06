#include "ui/mainwindow.h"

#include <QApplication>
#include <core/devices.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow mw;
    qDebug()<<021;
    mw.show();
    return a.exec();
}
