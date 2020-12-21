#include "ui/mainwindow.h"

#include <QApplication>
#include <core/devices.h>
#include "core/tasks/tasksmanager.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QCoreApplication::setOrganizationName("malabdali");
    QCoreApplication::setOrganizationDomain("malabdali.com");
    QCoreApplication::setApplicationName("3D Printers Manager");
    MainWindow mw;
    mw.show();
    return a.exec();
}
