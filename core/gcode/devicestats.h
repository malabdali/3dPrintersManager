#ifndef PRINTINGSTATUS_H
#define PRINTINGSTATUS_H

#include <QObject>
#include "../gcodecommand.h"
#include <functional>
#include "../device.h"

namespace GCode {
    class DeviceStats;
}
class GCode::DeviceStats : public GCodeCommand
{
    Q_OBJECT
private://fields

    QMap<QByteArray,QByteArray> _stats;
public:
    explicit DeviceStats(Device* _device);
    QMap<QByteArray,QByteArray> GetStats();

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};

#endif // PRINTINGSTATUS_H
