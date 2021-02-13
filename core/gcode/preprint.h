#ifndef PREPRINT_H
#define PREPRINT_H
#include <QObject>
#include "../gcodecommand.h"
#include <functional>

namespace GCode {
    class PrePrint;
}

class GCode::PrePrint : public GCodeCommand
{
    Q_OBJECT
private://fields
    int _fan_speed;
    bool _go_home,_e_absolute;
    double _e_position;
    QByteArray _maximum_feedrate_m203,_acceleration_m204,_max_acceleration_m201,_jerk_limits_m205;
    bool _fan_speed_finished,_acceleration_finished,_e_absolute_finished,_go_home_finished,_e_position_finished,_jerk_finished,_max_feedrate_finished,_max_acceleration_finished;
public:
    explicit PrePrint(Device* device,int fanSpeed,QByteArray acceleration,QByteArray jerkLimits,QByteArray maxAcceleration,QByteArray maxFeedrate,double ePosition,bool goHome,bool EAbsolute);


    // GCodeCommand interface
public:

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};
#endif // PREPRINT_H
