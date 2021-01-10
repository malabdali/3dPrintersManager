#ifndef STOPSDPRINT_H
#define STOPSDPRINT_H

#include <QObject>
#include "../gcodecommand.h"
#include <functional>

namespace GCode {
    class StopSDPrint;
}

class GCode::StopSDPrint : public GCodeCommand
{
    Q_OBJECT
public:
    explicit StopSDPrint(Device* device);


    // GCodeCommand interface
public:

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};

#endif // STOPSDPRINT_H
