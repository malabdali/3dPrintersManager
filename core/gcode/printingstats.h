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
    bool _is_printing,_printin_stats_updated;
    double _percent;
    uint32_t _printed_bytes, _total_bytes;

public:
    explicit PrintingStats(Device* _device);
    double GetPercent();
    uint32_t GetPrintedBytes();
    uint32_t GetTotalBytes();
    bool IsPrinting();

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};

#endif // PRINTINGSTATS_H
