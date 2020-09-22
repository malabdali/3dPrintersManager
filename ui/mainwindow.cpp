#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include "QFile"
#include "../core/remoteserver.h"
#include <QList>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),_devices_widget(new DevicesWidget())
{
    ui->setupUi(this);
    this->setCentralWidget(_devices_widget);
}

MainWindow::~MainWindow()
{
    delete ui;
}
