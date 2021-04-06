#ifndef DEVICEPROBLEMSOLVER_H
#define DEVICEPROBLEMSOLVER_H

#include <QObject>
#include "devicecomponent.h"
#include <QJsonDocument>
#include "gcodecommand.h"

class DeviceProblemSolver : public DeviceComponent
{
    Q_OBJECT
private://fields
    int _last_command_error,_last_device_error;
    bool _commands_was_played;
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
    QJsonDocument ToJson();
    void FromJson(QJsonDocument* json);
    QByteArray ErrorToText();

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
    void Save();
    void Load();


    // DeviceComponent interface
public:
    void Setup() override;

    // DeviceComponent interface
public:
    void Disable() override;
};

#endif // DEVICEPROBLEMSOLVER_H
