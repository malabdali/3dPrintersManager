#include "ui/mainwindow.h"

#include <QApplication>
#include <core/devices.h>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DeviceInfo di1("3D Printer"),di2("3D Printer 2"),di3("3D Printer 2"),di4("3D Printer 2"),
            di5("3D Printer 2"),di6("3D Printer 2"),di7("3D Printer 2"),di8("3D Printer 2"),di9("3D Printer 2"),di10("3D Printer 2")
            ,di11("3D Printer 2"),di13("3D Printer 2"),di14("3D Printer 2"),di15("3D Printer 2"),di16("3D Printer 2"),di17("3D Printer 2")
            ,di18("3D Printer 2"),di19("3D Printer 2"),di20("3D Printer 2"),di21("3D Printer 2"),di22("3D Printer 2"),di23("3D Printer 2")
            ,di24("3D Printer 2"),di25("3D Printer 2"),di26("3D Printer 2"),di27("3D Printer 2"),di28("3D Printer 2"),di29("3D Printer 2");
    di1.SetBaudRate(250000);
    di2.SetBaudRate(250000);
    di3.SetBaudRate(250000);
    di4.SetBaudRate(250000);
    di5.SetBaudRate(250000);
    di6.SetBaudRate(250000);
    di7.SetBaudRate(250000);
    di8.SetBaudRate(250000);
    di9.SetBaudRate(250000);
    di10.SetBaudRate(250000);
    di11.SetBaudRate(250000);
    Devices::GetInstance()->AddDevice(di1);
    Devices::GetInstance()->AddDevice(di2);
    Devices::GetInstance()->AddDevice(di3);
    Devices::GetInstance()->AddDevice(di4);
    Devices::GetInstance()->AddDevice(di5);
    Devices::GetInstance()->AddDevice(di6);
    Devices::GetInstance()->AddDevice(di7);
    Devices::GetInstance()->AddDevice(di8);
    Devices::GetInstance()->AddDevice(di9);
    Devices::GetInstance()->AddDevice(di10);
    Devices::GetInstance()->AddDevice(di11);
    Devices::GetInstance()->DetectPortAndConnectForAllDevices();
    MainWindow mw;
    mw.show();
    return a.exec();
}
