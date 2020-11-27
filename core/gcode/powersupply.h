#ifndef POWERSUPPLY_H
#define POWERSUPPLY_H
#include <QObject>
#include "../gcodecommand.h"
#include <functional>

namespace GCode {
    class PowerSupply;
}

class GCode::PowerSupply : public GCodeCommand
{
    Q_OBJECT
public:
    explicit PowerSupply(Device* device,bool on,std::function<void(bool)> callback);


    // GCodeCommand interface
public:

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};

#endif // POWERSUPPLY_H
