#ifndef TASKSMANAGER_H
#define TASKSMANAGER_H

#include <QObject>
#include "./task.h"

class TasksManager : public QObject
{
    Q_OBJECT
private: //statics

    static TasksManager* _Instance;
public: //statics
    static TasksManager *GetInstance();

private://fields
    QList<class Task*> _tasks;
    QList<class Task*> _finished_tasks;
    QList<class Task*> _started_tasks;
    int _timer_id;
private: //methods
    void timerEvent(QTimerEvent* ev)override;
    void WhenTasksUpdated(QJsonArray ja);
    void WhenDeviceRemoved(class Device* dev);
public:
    explicit TasksManager(QObject *parent = nullptr);
    class Task* CreateTask(QJsonObject data);
    QList<Task *> GetTasksOfDevice(class Device* dev,Task::TaskStatus ts);
    QList<Task *> GetTasksOfDevice(class Device* dev);
    QList<Task *> GetStartedTasksOfDevice(class Device* dev);
    void AddTask(Task* task);
    void RemoveTask(Task* task);
    void UpdateTasks();
    void CallNextStepForTasks();
    bool CheckTaskDeviceIsExist(Task *task);
    class Task* GetTaskByID(QByteArray id);
    class Task* GetTheOldestTaskOfDevice(class Device* dev,Task::TaskStatus ts);
    void Play();
    void Pause();
signals:

private slots:
    void WhenTaskFinished();
    void WhenTaskStatrted();
public:
    ~TasksManager();

};

#endif // TASKSMANAGER_H
