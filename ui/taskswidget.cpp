#include "taskswidget.h"
#include "ui_taskswidget.h"
#include <QDebug>
#include "../core/tasks/task.h"
#include "../core/tasks/printtask.h"
#include "../core/tasks/tasksmanager.h"
#include "../core/device.h"
#include "../core/deviceinfo.h"
#include <QMenu>
TasksWidget::TasksWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TasksWidget)
{
    ui->setupUi(this);
    _timer_id=this->startTimer(1000);
    connect(this->ui->_tasks_list,&QListWidget::itemChanged,this,&TasksWidget::WhenTaskSelected);
    _is_connect_to_tasks_manager=false;
    ConnectToTasksManager();
    QObject::connect(this->ui->_tasks_list, &QWidget::customContextMenuRequested,this, &TasksWidget::ShowContextMenu);
    this->ui->_tasks_list->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
    _selected_task=nullptr;
}

void TasksWidget::UpdateList()
{
    if(!TasksManager::GetInstance())
        return;

    QList<Task *> tasks=GatTasksByStatus();
    for(Task *t:tasks){
        auto item=GetItemByTask(t);
        if(!item){
            this->AddItem(t);
        }
        else{
            this->UpdateItem(item);
        }
    }
    for(auto&[k,v]:_tasks)
        if(!tasks.contains(v))
            RemoveItem(k);

}

QListWidgetItem *TasksWidget::GetItemByTask(Task *t)
{
    for(auto [k,v]:_tasks){
        if(t==v)
            return k;
    }
    return nullptr;
}

Task *TasksWidget::GetTaskByItem(QListWidgetItem * item)
{
    if(_tasks.find(item)!=_tasks.end()){
        return _tasks[item];
    }
    return nullptr;
}

QListWidgetItem *TasksWidget::AddItem(Task *task)
{
    QListWidgetItem* item=new QListWidgetItem(this->ui->_tasks_list);
    _tasks[item]=task;
    this->ui->_tasks_list->addItem(item);
    UpdateItem(item);
    return item;
}

void TasksWidget::RemoveItem(QListWidgetItem *item)
{
    this->ui->_tasks_list->removeItemWidget(item);
    _tasks.erase(item);
    delete item;
}

void TasksWidget::UpdateItem(QListWidgetItem * item)
{
    Task* task=GetTaskByItem(item);
    if(task)
    {
        if(PrintTask* pt=dynamic_cast<PrintTask*>(task))
        {
            item->setText(task->GetDevice()->GetDeviceInfo()->GetDeviceName()+" "+task->GetType()+" ( "+pt->GetFile()+" ) "+StatusToText(task->GetStaus()));
        }
        item->setIcon(QIcon(StatusToIcon(task->GetStaus())));
    }
}

QList<Task *> TasksWidget::GatTasksByStatus()
{
    QList<Task *> tasks;
    if(this->ui->_tasks_type->currentIndex()==0){
        tasks= TasksManager::GetInstance()->GetAllTasks();
    }
    else if(this->ui->_tasks_type->currentIndex()==1){
        for(Task* t:TasksManager::GetInstance()->GetAllTasks())
            if(!t->IsFinished())
                tasks.append(t);
    }
    else if(this->ui->_tasks_type->currentIndex()==2){
        for(Task* t:TasksManager::GetInstance()->GetAllTasks())
            if(!t->IsFinished() && t->IsStarted())
                tasks.append(t);
    }
    else if(this->ui->_tasks_type->currentIndex()==3){
        for(Task* t:TasksManager::GetInstance()->GetAllTasks())
            if(t->IsFinished())
                tasks.append(t);
    }
    return tasks;
}

TasksWidget::~TasksWidget()
{
    this->destroy(_timer_id);
    delete ui;
}

void TasksWidget::timerEvent(QTimerEvent *te)
{
    if(this->isVisible()){
        ConnectToTasksManager();
        UpdateList();
    }
}



void TasksWidget::WhenTaskSelected(QListWidgetItem *item)
{

}

void TasksWidget::WhenTaskDeleted(Task *t)
{
    auto item=this->GetItemByTask(t);
    if(item)this->RemoveItem(item);
}

void TasksWidget::WhenTaskAdded(Task *t)
{
    this->UpdateList();
}

void TasksWidget::WhenTaskStatusChanged(Task *t)
{
    this->UpdateList();
}

void TasksWidget::ConnectToTasksManager()
{
    if(TasksManager::GetInstance() && !_is_connect_to_tasks_manager)
    {
        connect(TasksManager::GetInstance(),&TasksManager::OnTaskRemoved,this,&TasksWidget::WhenTaskDeleted);
        connect(TasksManager::GetInstance(),&TasksManager::OnTaskAdded,this,&TasksWidget::WhenTaskAdded);
        connect(TasksManager::GetInstance(),&TasksManager::OnTaskStatusChanged,this,&TasksWidget::WhenTaskStatusChanged);
        _is_connect_to_tasks_manager=true;
    }

}

void TasksWidget::ShowContextMenu(const QPoint &pos)
{
    QList<QListWidgetItem*> items=this->ui->_tasks_list->selectedItems();
    if(items.length()>0)
    {
        QPoint p=pos;
        p.setX(p.x()+20);
        QListWidgetItem* i=items[0];
        Task* t=GetTaskByItem(i);
        if(t){
            _selected_task=t;
            QMenu menu;
            if(t->NextStepIsFinished()){
                menu.addAction(this->ui->_finish_action);
                menu.addAction(this->ui->_repeat_action);
            }
            else if(t->IsFinished()){
                menu.addAction(this->ui->_repeat_action);
            }
            else{
                menu.addAction(this->ui->_cancel_action);
            }
            menu.exec(this->mapToGlobal(p));
        }
    }
}



QString TasksWidget::StatusToText(Task::TaskStatus status)
{
    switch (status) {
    case Task::TaskStatus::Wait:
        return "Wait";
        break;

    case Task::TaskStatus::Downloading:
        return "Downloading";
        break;

    case Task::TaskStatus::Downloaded:
        return "Downloaded";
        break;

    case Task::TaskStatus::Uploading:
        return "Uploading";
        break;

    case Task::TaskStatus::Uploaded:
        return "Uploaded";
        break;

    case Task::TaskStatus::Started:
        return "Started";
        break;

    case Task::TaskStatus::Printing:
        return "Printing";
        break;

    case Task::TaskStatus::Printed:
        return "Printed";
        break;

    case Task::TaskStatus::Done:
        return "Done";
        break;

    case Task::TaskStatus::Canceled:
        return "Canceled";
        break;


    }
}

QString TasksWidget::StatusToIcon(Task::TaskStatus status)
{switch (status) {
    case Task::TaskStatus::Wait:
        return ":/icon/images/wait.png";
        break;

    case Task::TaskStatus::Downloading:
        return ":/icon/images/played.png";
        break;

    case Task::TaskStatus::Downloaded:
        return ":/icon/images/played.png";
        break;

    case Task::TaskStatus::Uploading:
        return ":/icon/images/played.png";
        break;

    case Task::TaskStatus::Uploaded:
        return ":/icon/images/played.png";
        break;

    case Task::TaskStatus::Started:
        return ":/icon/images/played.png";
        break;

    case Task::TaskStatus::Printing:
        return ":/icon/images/played.png";
        break;

    case Task::TaskStatus::Printed:
        return ":/icon/images/played.png";
        break;

    case Task::TaskStatus::Done:
        return ":/icon/images/done.png";
        break;

    case Task::TaskStatus::Canceled:
        return ":/icon/images/fail.png";
        break;


    }
}

void TasksWidget::on__cancel_action_triggered()
{
    _selected_task->Cancel();
}

void TasksWidget::on__repeat_action_triggered()
{
    TasksManager::GetInstance()->RepeatTask(_selected_task);
}

void TasksWidget::on__finish_action_triggered()
{
    _selected_task->Finish();
}
