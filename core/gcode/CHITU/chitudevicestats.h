#ifndef ChituDeviceStats_H
#define ChituDeviceStats_H

#include <QObject>
#include "../../chitugcodecommand.h"
#include "../../device.h"

namespace GCode {
namespace Chitu {
    class ChituDeviceStats;
}
}
class GCode::Chitu::ChituDeviceStats : public ChituGcodeCommand
{
    Q_OBJECT
private://fields

    QMap<QByteArray,QByteArray> _stats;
public:
    explicit ChituDeviceStats(Device* _device);
    QMap<QByteArray,QByteArray> GetStats();

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};

#endif // PRINTINGSTATUS_H
