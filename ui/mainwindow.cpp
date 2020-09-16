#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "QFile"
#include "../core/remoteserver.h"
#include <QList>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    _dfunction=nullptr;
    QObject::connect(ui->TestButton,&QPushButton::clicked,this,&MainWindow::WhenClickTestButton);
    QObject::connect(ui->stopButton,&QPushButton::clicked,this,&MainWindow::StopFunction);
    QObject::connect(Devices::GetInstance()->GetDevice(DeviceInfo("3D Printer")),&Device::NewLinesAvailable,
                     this,&MainWindow::WhenLineAvailabel);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::WhenClickTestButton()
{
    RemoteServer::GetInstance()->LoadAllDevicesInfo([](QNetworkReply* reply)->void{qDebug()<<"request result : "<<RemoteServer::GetInstance()->IsSuccess(reply);});
    /*if(_dfunction)
        delete  _dfunction;
    QFile file("C:/Users/malabdali/Desktop/circle.gcode");
    file.open(QIODevice::ReadOnly);
    QByteArray data=file.readAll();
    file.close();
    Devices::GetInstance()->GetDevice(DeviceInfo("3D Printer"))->ClearLines();
    Devices::GetInstance()->GetDevice(DeviceInfo("3D Printer"))->AddFunction(DeviceFunctions::Function::UploadFile,"TESTFILE.GCO",data);
    //_dfunction=new DeviceFunctions(Devices::GetInstance()->GetDevice(DeviceInfo("3D Printer")),DeviceFunctions::Function::UploadFile,"TESTFILE.GCO",data);
    //qDebug()<<data;
    //_dfunction->Start();
    //qDebug()<<Devices::GetInstance()->GetDevice(DeviceInfo("3D Printer"))->thread()<<this->thread();
    /*qDebug()<<"please Send data";
    Devices::GetInstance()->GetDevice(DeviceInfo("3D Printer"))->Write("M20\n");
    qDebug()<<"wait please";*/

}

void MainWindow::StopFunction()
{
    Devices::GetInstance()->GetDevice(DeviceInfo("3D Printer"))->ClearFunctions();
}

void MainWindow::WhenLineAvailabel(QByteArrayList ba)
{
    //qDebug()<<Devices::GetInstance()->GetDevice(DeviceInfo("3D Printer"))->thread()<<this->thread();
    /*while(Devices::GetInstance()->GetDevice(DeviceInfo("3D Printer"))->IsThereAvailableLines()){
        //qDebug()<<Devices::GetInstance()->GetDevice(DeviceInfo("3D Printer"))->ReadLine();
    }*/
}

