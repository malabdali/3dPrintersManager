#ifndef REPORTTEMPERATURE_H
#define REPORTTEMPERATURE_H

#include <QObject>
#include "../gcodecommand.h"
#include "../device.h"

namespace GCode {
    class ReportTemperature;
}
class GCode::ReportTemperature : public GCodeCommand
{
    Q_OBJECT
private://fields

    double _bed_temperature,_hotend_temperature;
public:
    explicit ReportTemperature(Device* _device);
    double GetHotendTemperature()const;
    double GetBedTemperature()const;

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};

#endif // PRINTINGSTATUS_H
