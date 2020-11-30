#ifndef GCODECOMMAND_H
#define GCODECOMMAND_H

#include <QObject>
#include <QVariant>
#include <QDebug>
#include <QMutex>
#include "config.h"
class GCodeCommand : public QObject
{
    Q_OBJECT
public://types
    enum CommandError{
        NoError=0,
        PortClosed=1,
        PortError=2,
        NoChecksum=3,
        WriteError=4,
        UnknownError=5,
        Busy=6,
        TimeOut,
    };

protected://fields
    class Device *_device;
    QByteArray _gcode;
    bool _finished,_started,_is_success;
    QMutex _mutex;
    CommandError _command_error;
    uint32_t _no_response_time_out;
    QTimer *_no_response_timer;
public:
    explicit GCodeCommand(Device *device, QByteArray command,uint32_t noResponseTimeout=DEFAULT_Command_No_RESPONSE_TIMEOUT);
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
    virtual void WhenTimeOut();
    void SetError(CommandError error);

signals:
    void Finished(bool);

};

#endif // GCODECOMMAND_H
