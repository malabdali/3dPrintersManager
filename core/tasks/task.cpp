#include "task.h"
#include "../devices.h"
#include "../remoteserver.h"
Task::Task(QJsonObject data, QObject *parent) : QObject(parent)
{
    _data=data;
    _type=data["type"].toString().toUtf8();
    _id=data["_id"].toString().toUtf8();
    _status=(TaskStatus)data["status"].toInt();
    _device=Devices::GetInstance()->GetDevice(data["printer"].toString().toUtf8());
    _status_updated=true;
    _is_started=false;
    _is_finished=false;
    _create_time=QDateTime::fromString(data["date"].toString().toUtf8(),Qt::DateFormat::ISODateWithMs);
}

QByteArray Task::GetID() const{
    return _id;
}

bool Task::operator ==(const QByteArray &id) const{
    return this->_id==id;
}

QByteArray Task::GetType()
{
    return _type;
}

Task::TaskStatus Task::GetStaus()
{
    return _status;
}

QDateTime Task::GetCreateTime()const
{
    return _create_time;
}

Device *Task::GetDevice() const
{
    return _device;
}

void Task::UpdateStatus()
{
    QVariantMap vmap;
    vmap.insert("status",_status);
    RemoteServer::GetInstance()->SendUpdateQuery([this](QNetworkReply* reply)->void{
        if(RemoteServer::GetInstance()->IsSuccess(reply))
            _status_updated=true;
    },TASKS_TABLE,vmap,_id);
}

Task::~Task()
{
}

bool Task::IsStarted()
{
    return _is_started;
}

bool Task::IsFinished(){
    return _is_finished;
}

void Task::Start(){
    _is_started=true;
    SetStatus(Started);
    qDebug()<<"start task";
    emit OnStarted();
}

void Task::Finish(){
    qDebug()<<"task finished";
    SetStatus(Done);
    UpdateStatus();
}

void Task::Continue()
{
    _is_started=true;
}

void Task::NextStep()
{
    if(_status_updated && _status==TaskStatus::Done){
        _is_finished=true;
        emit OnFinished();
    }
    else if(!_status_updated)
        UpdateStatus();
}

void Task::SetStatus(Task::TaskStatus status){
    if(_status == status)
        return;
    _status=status;
    _status_updated=false;
}

Task::operator QByteArray(){
    return _id;
}

bool Task::operator ==(const Task &t) const{
    return this->_id==t.GetID();
}
