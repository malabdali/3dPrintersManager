#ifndef M600_H
#define M600_H
#include <QObject>
#include "../gcodecommand.h"
#include <functional>

namespace GCode {
    class M600;
}

class GCode::M600 : public GCodeCommand
{
    Q_OBJECT
public:
    explicit M600(Device* device);


    // GCodeCommand interface
public:

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};
#endif // M600_H
