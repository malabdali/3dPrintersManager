#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "QFile"
#include "../core/remoteserver.h"
#include <QList>
#include <QInputDialog>
#include "../core/tasks/tasksmanager.h"
#include <QMainWindow>
#include <QDockWidget>
#include <QHBoxLayout>
#include "../core/system.h"
#include "../core/camerasmanager.h"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),_devices_widget(new DevicesWidget()),_tasks_widget(nullptr)
{
    ui->setupUi(this);

    this->setCentralWidget(_devices_widget);
    connect(this->ui->_network_action,&QAction::triggered,this,&MainWindow::WhenNetworkInfoActionClicked);
    connect(this->ui->_refresh_devices_action,&QAction::triggered,_devices_widget,&DevicesWidget::LoadDevices);
    connect(this->ui->_task_manager_action,&QAction::changed,this,&MainWindow::WhenTaskManagerChanged);
    connect(this->ui->_show_tasks_window_action,&QAction::triggered,this,&MainWindow::WhensShowTasksTrigerred);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::WhenNetworkInfoActionClicked()
{
    bool ok;
    QString net=QInputDialog::getText(this,"network id","set network id",QLineEdit::Normal,System::GetInstance()->GetNetworkID(),&ok);
    if(ok){
        System::GetInstance()->SetNetworkID(net.toUtf8());
    }


}

void MainWindow::WhenTaskManagerChanged()
{
    if(!TasksManager::GetInstance()){
        new TasksManager(this);
    }
    if(ui->_task_manager_action->isChecked())
    {
        ui->_task_manager_action->setText("disable task manager");
        TasksManager::GetInstance()->Play();
    }
    else
    {
        ui->_task_manager_action->setText("enable task manager");
        TasksManager::GetInstance()->Pause();
    }

}

void MainWindow::WhensShowTasksTrigerred()
{
    if(!_tasks_widget)
    {
        _tasks_widget=new TasksWidget();
        QDockWidget* dw=new QDockWidget("tasks",this);
        dw->setWidget(_tasks_widget);
        this->addDockWidget(Qt::DockWidgetArea::LeftDockWidgetArea,dw);
    }
    else{
        _tasks_widget->parentWidget()->show();
    }
}



void MainWindow::on_actioncapture_toggled(bool arg1)
{
    if(arg1)CamerasManager::GetInstance()->Play();
    else CamerasManager::GetInstance()->Pause();
}
