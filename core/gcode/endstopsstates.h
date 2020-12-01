#ifndef ENDSTOPSSTATES_H
#define ENDSTOPSSTATES_H

#include <QObject>
#include "../gcodecommand.h"
namespace GCode {
    class EndstopsStates;
}
class GCode::EndstopsStates : public GCodeCommand
{
    Q_OBJECT
private://fields
    bool _filament_is_ok;
public:
    explicit EndstopsStates(Device* _device);
    bool FilamentExist()const;

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};

#endif // ENDSTOPSSTATES_H
