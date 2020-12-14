#include "tasksmanager.h"
#include "printtask.h"
#include "../remoteserver.h"
#include "../../config.h"
#include "QJsonDocument"
#include "QVariantHash"
#include "../device.h"
#include "../devices.h"
#include "../deviceinfo.h"
#include <QJsonValue>
#include <QTimerEvent>
#include "task.h"

TasksManager* TasksManager::_Instance=nullptr;
TasksManager *TasksManager::GetInstance()
{
    return _Instance;
}




TasksManager::TasksManager(QObject *parent) : QObject(parent)
{
    _Instance=this;
    connect(Devices::GetInstance(),&Devices::DeviceRemoved,this,&TasksManager::WhenDeviceRemoved);
    _timer_id=-1;
}

Task *TasksManager::CreateTask(QJsonObject data)
{
    QByteArray ba=data["type"].toString().toUtf8();
    Task* t=nullptr;
    if(ba=="PRINT"){
        t=new PrintTask(data,this);
    }
    return t;
}

QList<Task *> TasksManager::GetTasksOfDevice(Device *dev, Task::TaskStatus ts)
{
    QList<Task*> tasks;
    for(Task* t:_tasks)
        if(t->GetDevice()==dev && t->GetStaus()==ts)
            tasks.append(t);
    return tasks;
}

QList<Task *> TasksManager::GetTasksOfDevice(Device *dev)
{
    QList<Task*> tasks;
    for(Task* t:_tasks)
        if(t->GetDevice()==dev)
            tasks.append(t);
    return tasks;
}

QList<Task *> TasksManager::GetStartedTasksOfDevice(Device *dev)
{
    QList<Task*> tasks;
    for(Task* t:_started_tasks)
        if(t->GetDevice()==dev && t->GetStaus()!=Task::Done)
            tasks.append(t);
    return tasks;
}

void TasksManager::AddTask(Task *task)
{
    qDebug()<<"TasksManager::AddTask";
    if(!_tasks.contains(task))
    {
        _tasks.append(task);
        if(task->GetStaus()!=Task::Wait){
            _started_tasks.append(task);
        }
    }
    connect(task,&Task::OnFinished,this,&TasksManager::WhenTaskFinished);
    connect(task,&Task::OnStarted,this,&TasksManager::WhenTaskStatrted);
}

void TasksManager::RemoveTask(Task *task)
{
    if(_tasks.contains(task))
    {
        qDebug()<<"TasksManager::RemoveTask "<<task;
        _tasks.removeAll(task);
        delete task;
    }
}

void TasksManager::UpdateTasks()
{
    QJsonObject obj,$in,$ne;
    QJsonArray ja;
    for(Device* dev:Devices::GetInstance()->GetAllDevices())
        ja.append(QString(dev->GetDeviceInfo()->GetDeviceName()));
    $in.insert("$in",ja);
    $ne.insert("$ne",-1);
    obj.insert("printer",$in);
    obj.insert("status",$ne);
    QJsonDocument jd;
    jd.setObject(obj);

    RemoteServer::GetInstance()->SendSelectQuery([this](QNetworkReply* reply)->void{WhenTasksUpdated(RemoteServer::GetInstance()->GetJSONValue(reply).toArray());},TASKS_TABLE,jd.toJson());
}

void TasksManager::CallNextStepForTasks()
{
    for(Device* dev:Devices::GetInstance()->GetAllDevices())
    {
        QList<Task*> st=GetStartedTasksOfDevice(dev);
        Task* wt=GetTheOldestTaskOfDevice(dev,Task::TaskStatus::Wait);
        if(st.length()>0){
            if(!st[0]->IsStarted())
                st[0]->Continue();
            st[0]->NextStep();
        }
        else if(wt){
            wt->Start();
        }
    }
}

bool TasksManager::CheckTaskDeviceIsExist(Task* task)
{
    return Devices::GetInstance()->GetAllDevices().contains(task->GetDevice());
}


void TasksManager::timerEvent(QTimerEvent *ev)
{

    CallNextStepForTasks();
    UpdateTasks();
}

void TasksManager::WhenTasksUpdated(QJsonArray ja)
{
    for(QJsonValue jv:ja){
        Task* task=GetTaskByID(jv.toObject()["_id"].toString().toUtf8());
        if(!task){
            AddTask(CreateTask(jv.toObject()));
        }
    }

}

void TasksManager::WhenDeviceRemoved(Device *dev)
{
    QList<Task*> tasks=GetTasksOfDevice(dev);
    qDebug()<<" call TasksManager::WhenDeviceRemoved "<<tasks;
    for(Task* &t:tasks.toVector())
        RemoveTask(t);
}

Task *TasksManager::GetTaskByID(QByteArray id)
{
    auto iterator=std::find_if(_tasks.begin(),_tasks.end(),[&id](Task* t)->bool{return id==*t;});
    if(iterator!=_tasks.end()){
        return *iterator;
    }
    else return nullptr;
}

Task *TasksManager::GetTheOldestTaskOfDevice(Device *dev, Task::TaskStatus ts)
{
    QList<Task*> tasks=GetTasksOfDevice(dev,ts);
    qDebug()<<tasks;
    if(tasks.length()==0)
        return nullptr;
    auto iterator=std::min_element(tasks.begin(),tasks.end(),[](const Task* f ,const Task* s)->bool{return f->GetCreateTime()<s->GetCreateTime();});
    if(iterator==tasks.end())
        return nullptr;
    qDebug()<<"found it and sent";
    return *iterator;
}

void TasksManager::Play()
{
    if(_timer_id!=-1)
        Pause();
    _timer_id=this->startTimer(TASK_MANAGER_TIMER);
}

void TasksManager::Pause()
{
    killTimer(_timer_id);
    _timer_id=-1;
}

void TasksManager::WhenTaskFinished()
{
    Task* t=dynamic_cast<Task*>(sender());
    this->_started_tasks.removeAll(t);
    this->_finished_tasks.append(t);
}

void TasksManager::WhenTaskStatrted()
{
    qDebug()<<"when task started";
    Task* t=dynamic_cast<Task*>(sender());
    if(!_started_tasks.contains(t))
        this->_started_tasks.append(t);

}

TasksManager::~TasksManager()
{
    if(_timer_id>=0)
        this->killTimer(_timer_id);

}
