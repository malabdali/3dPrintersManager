#ifndef PRINTTASK_H
#define PRINTTASK_H

#include "task.h"
#include <QObject>
#include"../gcode/startprinting.h"
#include"../gcode/stopsdprint.h"
#include<QJsonObject>

class PrintTask : public Task
{
    Q_OBJECT
    QByteArray _file;
    QJsonObject _file_info;
    bool _wait,_want_to_cancel,_want_to_cancel_finished;
    GCode::StopSDPrint* _stop_printing_command;
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
    void TryToCancel();
    void StopPrinting();
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
    void WantToCancel();
    void SetData(QJsonObject data) override;
};

#endif // PRINTTASK_H
