#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QJsonObject>
#include <QDateTime>
#include <QNetworkReply>
class Task : public QObject
{
    Q_OBJECT

    friend class TasksManager;
public: //nested types
    enum TaskStatus:int{
        Canceled=-2,
        Done=-1,
        Wait=0,
        Started=1,
        Downloading=2,
        Downloaded=3,
        Uploading=4,
        Uploaded=5,
        Printing=6,
        Printed=7
    };

protected:
    QByteArray _id,_type;
    class Device* _device;
    QJsonObject _data;
    QDateTime _create_time;
    bool _status_updated , _is_started, _is_finished;
    QNetworkReply* _network_reply;
private://fields
    TaskStatus _status;


public:
    explicit Task(QJsonObject data,QObject *parent = nullptr);
    QByteArray GetID()const;
    bool operator ==(const Task& t)const;
    bool operator ==(const QByteArray& id)const;
    operator QByteArray();
    QByteArray GetType();
    TaskStatus GetStaus();
    QDateTime GetCreateTime()const;
    class Device* GetDevice()const;
    void UpdateStatus();
    virtual ~Task();
    void FinishTask();
    bool IsStarted();
    bool IsFinished();
    virtual bool NextStepIsFinished()=0;
    virtual void Start();
    virtual void Finish();
    virtual void Cancel();
    virtual void Continue();
    virtual void NextStep();
    virtual void SetData(QJsonObject data);

protected://methods
    void SetStatus(TaskStatus status);
    virtual void Repeat();
private slots:
signals:
    void OnStatusUpdated();
    void OnStarted();
    void OnFinished();

};

#endif // TASK_H
