#ifndef PRINTINGSTATS_H
#define PRINTINGSTATS_H

#include <QObject>
#include "../gcodecommand.h"
namespace GCode {
    class PrintingStats;
}
class GCode::PrintingStats : public GCodeCommand
{
    Q_OBJECT
private://fields
    bool _is_printing;
    double _percent;
public:
    explicit PrintingStats(Device* _device);
    double GetPercent();
    bool IsPrinting();

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};

#endif // PRINTINGSTATS_H
