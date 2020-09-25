#ifndef GCODECOMMAND_H
#define GCODECOMMAND_H

#include <QObject>
#include <QVariant>
#include "device.h"

class GCodeCommand : public QObject
{
    Q_OBJECT
protected://fields
    QByteArray _gcode;
    class Device *_device;
    bool _finished,_started;
public:
    explicit GCodeCommand(Device *device, QByteArray command);
    virtual ~GCodeCommand();
    virtual void Start();
    bool IsFinished();
    virtual void Stop()=0;
    bool IsStarted()const;
    QByteArray GetGCode()const;


private slots:
    void WhenLineAvailable(QByteArrayList);
    void WhenWriteFinished(bool b);
    void WhenWriteLine();
    void WhenErrorOcurre(int);

protected:
    virtual void OnAvailableData(const QByteArray& ba)=0;
    virtual void OnDataWritten()=0;
    virtual void OnAllDataWritten(bool)=0;
    virtual void Finish(bool);

signals:
    void Finished(bool);

};

#endif // GCODECOMMAND_H
