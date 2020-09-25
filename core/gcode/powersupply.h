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
    void Start() override;
    void Stop() override;

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnDataWritten() override;
    void OnAllDataWritten(bool) override;
    void Finish(bool) override;
};

#endif // POWERSUPPLY_H
