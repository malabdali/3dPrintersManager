#ifndef PRINTINGSTATUS_H
#define PRINTINGSTATUS_H

#include <QObject>
#include "../gcodecommand.h"
#include <functional>
#include "../device.h"

namespace GCode {
    class PrintingStatus;
}
class GCode::PrintingStatus : public GCodeCommand
{
    Q_OBJECT
private://fields
    std::function<void(bool)> _callback;
public:
    explicit PrintingStatus(Device* _device,std::function<void(bool)> callback);

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

#endif // PRINTINGSTATUS_H
