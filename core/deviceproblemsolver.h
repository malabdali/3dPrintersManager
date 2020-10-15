#ifndef DEVICEPROBLEMSOLVER_H
#define DEVICEPROBLEMSOLVER_H

#include <QObject>

class DeviceProblemSolver : public QObject
{
    Q_OBJECT
private://fields
    class Device* _device;
    int _last_command_error,_last_device_error;
    bool _commands_was_played;
public:
    explicit DeviceProblemSolver(class Device *device = nullptr);

    bool IsThereProblem();


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
public slots:
    void SolveProblem();
    void CheckCommandError(GCodeCommand* command);

private://methods
    void SolveNoChecksumProblem();
    void WhenProblemSolved();

};

#endif // DEVICEPROBLEMSOLVER_H
