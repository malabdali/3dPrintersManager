#ifndef TASKSWIDGET_H
#define TASKSWIDGET_H

#include <QListWidgetItem>
#include "../core/tasks/task.h"

namespace Ui {
class TasksWidget;
}

class TasksWidget : public QWidget
{
    Q_OBJECT
private://fields
    int _timer_id;
    std::map<QListWidgetItem*,Task*> _tasks;
    bool _is_connect_to_tasks_manager;
    Task* _selected_task;

public:
    explicit TasksWidget(QWidget *parent = nullptr);
    void UpdateList();
    QListWidgetItem *GetItemByTask(Task* t);
    Task *GetTaskByItem(QListWidgetItem *);
    QListWidgetItem *AddItem(Task* task);
    void RemoveItem(QListWidgetItem *);
    void UpdateItem(QListWidgetItem *);
    QList<Task*> GatTasksByStatus();
    ~TasksWidget();

private slots:
    void on__cancel_action_triggered();

    void on__repeat_action_triggered();

    void on__finish_action_triggered();

private:
    Ui::TasksWidget *ui;
    void timerEvent(QTimerEvent* te) override;
    void WhenTaskSelected(QListWidgetItem * item);
    void WhenTaskDeleted(Task*);
    void WhenTaskAdded(Task*);
    void WhenTaskStatusChanged(Task*);
    void ConnectToTasksManager();
    void ShowContextMenu(const QPoint &pos);
    QString StatusToText(Task::TaskStatus status);
    QString StatusToIcon(Task::TaskStatus status);
};

#endif // TASKSWIDGET_H
