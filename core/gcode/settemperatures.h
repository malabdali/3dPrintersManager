#ifndef SETTEMPERATURES_H
#define SETTEMPERATURES_H

#include <QObject>
#include "../gcodecommand.h"
namespace GCode {
    class SetTemperatures;
}

class GCode::SetTemperatures : public GCodeCommand
{
    Q_OBJECT
private:
    int _bed,_hotend;
    bool _hotend_sent,_bed_sent;
public:
    explicit SetTemperatures(Device* device,int bed=-1,int hotend=-1);


    // GCodeCommand interface
public:

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};

#endif // SETTEMPERATURES_H
