#ifndef PRINTTASK_H
#define PRINTTASK_H

#include "task.h"
#include <QObject>
#include"../gcode/startprinting.h"
#include<QJsonObject>

class PrintTask : public Task
{
    Q_OBJECT
    QByteArray _file;
    QJsonObject _file_info;
    bool _wait;
    GCode::StartPrinting* _printing_command;
public:
    PrintTask(QJsonObject data, QObject *parent=nullptr);
    void DownloadFile();
    void UploadFile();
    void Print();
    QByteArray GetFile();;
private:
    void WhenDownloadFinished(const QByteArray& data);
    void WhenUploadFinished();
signals:
    void DownloadFileSuccess();
    void DownloadFileFailed();
    void FileNotExist();

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
    void Continue() override;
    bool NextStepIsFinished() override;
    void Cancel() override;
    void Repeat() override;
};

#endif // PRINTTASK_H
