#ifndef UPLOADFILE_H
#define UPLOADFILE_H

#include <QObject>
#include "../gcodecommand.h"
#include <functional>
#include <QMutex>
#include <QMutexLocker>
#include "../../config.h"
#include <QTimer>

namespace GCode {
    class UploadFile;
}
class GCode::UploadFile : public GCodeCommand
{
    Q_OBJECT
public:
    explicit UploadFile(Device *device,QByteArray fileName,const QByteArrayList& data,quint64 firstLine,unsigned speed);

signals:

private://fields
    //todo
    uint32_t _counter,_first_line;
    bool _resend,_upload_stage,_open_failed,_open_success,_wait_resend,_file_saved;
    QByteArray _file_name;
    size_t _file_size;
    QByteArrayList _data;
    double _progress;
    unsigned _speed;
    QTimer* _end_timer,* _send_timer;
    int _resend_tries;
    // GCodeCommand interface
public:
    double GetProgress();
    QByteArray GetFileName();
    QByteArrayList GetData();
    quint32 GetSize();
    bool IsSuccess();
private slots:
    void Resend();
    void Send();
    void Send29();

protected:
    void InsideStart() override;
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStop() override;
};

#endif // UPLOADFILE_H
