#ifndef UPLOADFILE_H
#define UPLOADFILE_H

#include <QObject>
#include "../gcodecommand.h"
#include <functional>
#include <QMutex>
#include <QMutexLocker>

namespace GCode {
    class UploadFile;
}
class GCode::UploadFile : public GCodeCommand
{
    Q_OBJECT
public:
    explicit UploadFile(Device *device, std::function<void (bool)> callback,QByteArray fileName,const QByteArrayList& data);

signals:

private://fields
    uint32_t _counter;
    bool _resend,_upload_stage,_start_upload,_open_failed,_is_success;
    std::function<void(bool)> _callback;
    QByteArray _file_name;
    QByteArrayList _data;
    double _progress;
    QMutex _mutex;
    // GCodeCommand interface
public:
    void Start();
    void Stop();
    double GetProgress();
    QByteArray GetFileName();
    bool IsSuccess();

protected:
    void OnAvailableData(const QByteArray &ba);
    void OnDataWritten();
    void OnAllDataWritten(bool);
    void Finish(bool);

};

#endif // UPLOADFILE_H
