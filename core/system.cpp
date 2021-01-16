#include "system.h"
#include <QSettings>
#include <QVariant>
#include <QDebug>
#include <QApplication>
System* System::_Instance=nullptr;

System *System::GetInstance()
{
    if(!_Instance)
        new System();
    return _Instance;
}

System::System(QObject *parent) : QObject(parent)
{
    System::_Instance=this;
    QSettings settings;
    _network_id=settings.value("network").value<QByteArray>();
    connect(qApp,&QApplication::aboutToQuit,this,&System::onApplicationQuit);
}

void System::SetNetworkID(const QByteArray& id)
{
    _network_id=id;
    QSettings settings;
    settings.setValue("network",id);
}

QByteArray System::GetNetworkID() const
{
    return _network_id;
}

System::~System(){
}

void System::onApplicationQuit()
{
    deleteLater();
}
