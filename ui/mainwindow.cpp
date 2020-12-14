#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "QFile"
#include "../core/remoteserver.h"
#include <QList>
#include <QInputDialog>
#include "../core/tasks/tasksmanager.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),_devices_widget(new DevicesWidget())
{
    ui->setupUi(this);
    this->setCentralWidget(_devices_widget);
    connect(this->ui->_network_action,&QAction::triggered,this,&MainWindow::WhenNetworkInfoActionClicked);
    connect(this->ui->_refresh_devices_action,&QAction::triggered,_devices_widget,&DevicesWidget::LoadDevices);
    connect(this->ui->_task_manager_action,&QAction::changed,this,&MainWindow::WhenTaskManagerChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::WhenNetworkInfoActionClicked()
{
    bool ok;
    QString net=QInputDialog::getText(this,"network id","set network id",QLineEdit::Normal,Devices::GetInstance()->GetNetworkID(),&ok);
    if(ok){
        Devices::GetInstance()->SetNetworkID(net.toUtf8());
    }


}

void MainWindow::WhenTaskManagerChanged()
{
    qDebug()<<ui->_task_manager_action->isChecked();
    if(!TasksManager::GetInstance()){
        new TasksManager(this);
    }
    if(ui->_task_manager_action->isChecked())
        TasksManager::GetInstance()->Play();
    else
        TasksManager::GetInstance()->Pause();

}

