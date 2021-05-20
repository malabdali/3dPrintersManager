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
public:
    explicit DeleteFile(Device *device,QByteArray fileName);

signals:


// GCodeCommand interface
public:
QByteArray GetFileName();

protected:
void OnAvailableData(const QByteArray &ba) override;
void OnAllDataWritten(bool success) override;
void Finish(bool) override;
void InsideStart() override;
void InsideStop() override;
};

#endif // DELETEFILE_H
