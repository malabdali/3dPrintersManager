#ifndef DEVICEPROBLEMSOLVER_H
#define DEVICEPROBLEMSOLVER_H

#include <QObject>
#include "devicecomponent.h"
#include <QJsonDocument>
#include "gcodecommand.h"
#include "QDateTime"

class DeviceProblemSolver : public DeviceComponent
{
    Q_OBJECT
private://fields
    int _last_command_error,_last_device_error;
    bool _commands_was_played;
    QDateTime _last_error_time;
public: //nested types
    enum SolvingType:int{
        NON=0,
        OpenPort=1,
        GCode=2
    };
public:
    explicit DeviceProblemSolver(class Device *device = nullptr);
    SolvingType GetSolvingType()const;
    bool IsThereProblem();
    QByteArray ErrorToText()const ;

signals:
    void ProblemDetected();
    void SolveFinished();

private slots:
    void WhenLinesAvailable(QList<QByteArray> lines);
    void WhenErrorOccured(int error);
    void WhenPortClosed();
    void WhenPortOpened();
    void WhenCommandStarted(class GCodeCommand* command);
    void WhenCommandFinished(class GCodeCommand* command,bool success);
    void WhenStatsUpdateFailed(class GCodeCommand* command);
    void CheckCommandError(GCodeCommand* command);
public slots:
    void SolveProblem();

private://methods
    void SolveNoChecksumProblem();
    void WhenProblemSolved();
    QJsonDocument ToJson() override;
    void FromJson(QJsonDocument json) override;
    void Save()override;
    void Load()override;


    // DeviceComponent interface
public:
    void Setup() override;

    // DeviceComponent interface
public:
    void Disable() override;
};

#endif // DEVICEPROBLEMSOLVER_H
