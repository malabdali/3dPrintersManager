#include "system.h"
System* System::_Instance=nullptr;

System *System::GetInstance()
{
    return _Instance;
}

System::System(QObject *parent) : QObject(parent)
{
    System::_Instance=this;
}
