#ifndef GCODECOMMAND_H
#define GCODECOMMAND_H

#include <QObject>
#include <QVariant>
#include <QDebug>
#include <QMutex>

class GCodeCommand : public QObject
{
    Q_OBJECT
public://types
    enum CommandError{
        NoError=0,
        PortClosed=1,
        PortError=2,
        NoChecksum=3
    };

protected://fields
    class Device *_device;
    QByteArray _gcode;
    bool _finished,_started,_is_success;
    QMutex _mutex;
    CommandError _command_error;
public:
    explicit GCodeCommand(Device *device, QByteArray command);
    virtual ~GCodeCommand();
    Q_INVOKABLE void Start();
    bool IsFinished();
    Q_INVOKABLE void Stop();
    bool IsStarted()const;
    QByteArray GetGCode()const;
    bool IsSuccess()const;
    CommandError GetError()const;

private slots:
    void WhenLineAvailable(QByteArrayList);
    void WhenWriteFinished(bool success);
    void WhenErrorOccured(int);
    void WhenPortClosed();

protected:
    virtual void OnAvailableData(const QByteArray& ba)=0;
    virtual void OnAllDataWritten(bool)=0;
    virtual void Finish(bool);
    virtual void InsideStart()=0;
    virtual void InsideStop()=0;
    void SetError(CommandError error);

signals:
    void Finished(bool);

};

#endif // GCODECOMMAND_H
