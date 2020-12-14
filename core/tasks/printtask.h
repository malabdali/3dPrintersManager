#ifndef PRINTTASK_H
#define PRINTTASK_H

#include "task.h"
#include <QObject>
#include"../gcode/startprinting.h"

class PrintTask : public Task
{
    Q_OBJECT
    QByteArray _file;
    bool _wait;
    GCode::StartPrinting* _printing_command;
public:
    PrintTask(QJsonObject data, QObject *parent=nullptr);
    void DownloadFile();
    void UploadFile();
    void Print();
private:
    void WhenDownloadFinished(const QByteArray& data);
    void WhenUploadFinished();
signals:
    void DownloadFileSuccess();
    void DownloadFileFailed();

    // Task interface
public:
    void NextStep() override;
private slots:
    void WhenFileUploadSuccess(QByteArray ba);
    void WhenFileUploadFailed(QByteArray ba);
    void WhenCommandFinished(GCodeCommand* command,bool success);

public:
    ~PrintTask();

    // Task interface
public:
    void Start() override;
    void Finish() override;

    // Task interface
public:
    void Continue() override;
};

#endif // PRINTTASK_H
