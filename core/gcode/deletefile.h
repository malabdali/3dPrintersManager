#ifndef DELETEFILE_H
#define DELETEFILE_H

#include <QObject>
#include "../gcodecommand.h"
#include <functional>

namespace GCode {
    class DeleteFile;
}
class GCode::DeleteFile : public GCodeCommand
{
    Q_OBJECT

private://fields
QByteArray _file;
bool _is_success;
std::function<void(bool)> _callback;
public:
    explicit DeleteFile(Device *device, std::function<void (bool)> callback,QByteArray fileName);

signals:


// GCodeCommand interface
public:
void Start();
void Stop();

protected:
void OnAvailableData(const QByteArray &ba);
void OnDataWritten();
void OnAllDataWritten(bool);
void Finish(bool);
};

#endif // DELETEFILE_H
