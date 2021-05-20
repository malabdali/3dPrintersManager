#ifndef CHITUDELETEFILE_H
#define CHITUDELETEFILE_H
#include <QObject>
#include "../../chitugcodecommand.h"
#include "../../device.h"

namespace GCode {
namespace Chitu {
    class ChituDeleteFile;
}
}

class GCode::Chitu::ChituDeleteFile : public ChituGcodeCommand
{
    Q_OBJECT

private://fields
QByteArray _file;
bool _is_success;
public:
    explicit ChituDeleteFile(Device *device,QByteArray fileName);

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

#endif // CHITUDELETEFILE_H
