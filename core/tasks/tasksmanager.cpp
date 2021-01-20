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
        if(t->GetDevice()==dev && !t->IsFinished())
            tasks.append(t);
    return tasks;
}

QList<Task *> TasksManager::GetAllTasks()
{
    return _tasks;
}

void TasksManager::AddTask(Task *task)
{
    if(!_tasks.contains(task))
    {
        _tasks.append(task);
        if(task->GetStaus()>Task::Wait ){
            _started_tasks.append(task);
        }
        emit OnTaskAdded(task);
    }
    connect(task,&Task::OnFinished,this,&TasksManager::WhenTaskFinished);
    connect(task,&Task::OnStarted,this,&TasksManager::WhenTaskStatrted);
    connect(task,&Task::OnStatusUpdated,this,&TasksManager::WhenTaskStatusChanged);
}

void TasksManager::RemoveTask(Task *task)
{
    if(_tasks.contains(task))
    {
        _tasks.removeAll(task);
        _started_tasks.removeAll(task);
        _finished_tasks.removeAll(task);
        emit OnTaskRemoved(task);
        task->deleteLater();
    }
}

void TasksManager::UpdateTasks()
{
    QJsonObject obj,$in,$gt,$ne;
    QJsonArray ja;
    for(Device* dev:Devices::GetInstance()->GetAllDevices())
        ja.append(QString(dev->GetDeviceInfo()->GetDeviceName()));
    $in.insert("$in",ja);
    $gt.insert("$gt",-1);
    //$ne.insert("$ne",true);

    obj.insert("printer",$in);
    obj.insert("status",$gt);
    //obj.insert("cancel",$ne);
    QJsonDocument jd;
    jd.setObject(obj);

    RemoteServer::GetInstance()->SendSelectQuery([this](QNetworkReply* reply)->void{
        if(RemoteServer::GetInstance()->IsSuccess(reply)){
            WhenTasksUpdated(RemoteServer::GetInstance()->GetJSONValue(reply).toArray());
        }
        reply->deleteLater();
    },TASKS_TABLE,jd.toJson());
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

bool TasksManager::TaskIsExistsInLastTasks(Task *t)
{
    return _last_tasks.contains(t);
}


void TasksManager::timerEvent(QTimerEvent *ev)
{

    CallNextStepForTasks();
    UpdateTasks();
}

void TasksManager::WhenTasksUpdated(QJsonArray ja)
{
    _last_tasks.clear();
    for(QJsonValue jv:ja){
        Task* task=GetTaskByID(jv.toObject()["_id"].toString().toUtf8());
        if(!task){
            Task* ct=CreateTask(jv.toObject());
            _last_tasks.append(ct);
            AddTask(ct);
        }
        else{
            _last_tasks.append(task);
            task->SetData(jv.toObject());
        }
    }
    for(Task* t :_tasks)
        if(!_last_tasks.contains(t) && !t->IsFinished()){
            t->Cancel();

        }

}

void TasksManager::WhenDeviceRemoved(Device *dev)
{
    QList<Task*> tasks=GetTasksOfDevice(dev);
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
    if(tasks.length()==0)
        return nullptr;
    auto iterator=std::min_element(tasks.begin(),tasks.end(),[](const Task* f ,const Task* s)->bool{return f->GetCreateTime()<s->GetCreateTime();});
    if(iterator==tasks.end())
        return nullptr;
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

void TasksManager::RepeatTask(Task *t)
{
    _started_tasks.removeAll(t);
    _finished_tasks.removeAll(t);
    t->Repeat();
}

void TasksManager::WhenTaskFinished()
{
    Task* t=dynamic_cast<Task*>(sender());
    this->_started_tasks.removeAll(t);
    this->_finished_tasks.append(t);
}

void TasksManager::WhenTaskStatrted()
{
    Task* t=dynamic_cast<Task*>(sender());
    if(!_started_tasks.contains(t))
        this->_started_tasks.append(t);

}

void TasksManager::WhenTaskStatusChanged()
{
    Task* t=dynamic_cast<Task*>(this->sender());
    if(t)
        emit OnTaskStatusChanged(t);
}

TasksManager::~TasksManager()
{
    if(_timer_id>=0)
        this->killTimer(_timer_id);

}

