#include "preprint.h"
#include <QByteArray>
#include "../device.h"
#include "../deviceport.h"
GCode::PrePrint::PrePrint(Device* device,int fanSpeed,QByteArray acceleration,QByteArray jerkLimits,QByteArray maxAcceleration,QByteArray maxFeedrate,double ePosition,bool goHome,bool EAbsolute):GCodeCommand(device,"M20X")
{
    _fan_speed=fanSpeed;
    this->_acceleration_m204=acceleration;
    _go_home=goHome;
    this->_jerk_limits_m205=jerkLimits;
    _e_absolute=EAbsolute;
    _e_position=ePosition;
    _maximum_feedrate_m203=maxFeedrate;
    _acceleration_m204=acceleration;
    _max_acceleration_m201=maxAcceleration;
    _jerk_limits_m205=jerkLimits;
    _e_position_finished=false;
    _e_absolute_finished=false;
    _go_home_finished=false;
    _acceleration_finished=false;
    _fan_speed_finished=false;
    _jerk_finished=false;

    if(_fan_speed<0)
        _fan_speed_finished=true;
    if(_acceleration_m204.isEmpty())
        _acceleration_finished=true;
    if(_jerk_limits_m205.isEmpty())
        _jerk_finished=true;
    if(_max_acceleration_m201.isEmpty())
        _max_acceleration_finished=true;
    if(_maximum_feedrate_m203.isEmpty())
        _max_feedrate_finished=true;
    if(_e_position<0)
        _e_position=true;
    if(!_go_home)
        _go_home_finished=true;
    if(!_e_absolute)
        _e_absolute_finished=true;
}


void GCode::PrePrint::InsideStart()
{
    if(!_fan_speed_finished)
        _device->GetDevicePort()->Write(QByteArray("M106 S"+QByteArray::number(_fan_speed)+"\n"));
    else if(!_e_absolute_finished)
        _device->GetDevicePort()->Write(QByteArray("M82 \n"));
    else if(!_e_position_finished)
        _device->GetDevicePort()->Write(QByteArray("G92 E"+QByteArray::number(_e_position)+"\n"));
    else if(!_max_acceleration_finished)
        _device->GetDevicePort()->Write(QByteArray("M201 "+this->_max_acceleration_m201+"\n"));
    else if(!_max_feedrate_finished)
        _device->GetDevicePort()->Write(QByteArray("M203 "+this->_maximum_feedrate_m203+"\n"));
    else if(!_acceleration_finished)
        _device->GetDevicePort()->Write(QByteArray("M204 "+this->_acceleration_m204+"\n"));
    else if(!_jerk_finished)
        _device->GetDevicePort()->Write(QByteArray("M205 "+_jerk_limits_m205+"\n"));
    else if(!_go_home_finished)
        _device->GetDevicePort()->Write(QByteArray("G28 \n"));



}

void GCode::PrePrint::InsideStop()
{
}

void GCode::PrePrint::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().startsWith("x:"))
        return;
    if(ba.toLower().startsWith("ok")){
        if(!_fan_speed_finished){
            _fan_speed_finished=true;
            _device->GetDevicePort()->Write(QByteArray("M82 \n"));
        }
        else if(!_e_absolute_finished){
            _e_absolute_finished=true;
            _device->GetDevicePort()->Write(QByteArray("G92 E"+QByteArray::number(_e_position)+"\n"));
        }
        else if(!_e_position_finished){
            _e_position_finished=true;
            _device->GetDevicePort()->Write(QByteArray("M201 "+this->_max_acceleration_m201+"\n"));
        }
        else if(!_max_acceleration_finished){
            _max_acceleration_finished=true;
            _device->GetDevicePort()->Write(QByteArray("M203 "+this->_maximum_feedrate_m203+"\n"));
        }
        else if(!_max_feedrate_finished){
            _max_feedrate_finished=true;
            _device->GetDevicePort()->Write(QByteArray("M204 "+this->_acceleration_m204+"\n"));
        }
        else if(!_acceleration_finished){
            _acceleration_finished=true;
            _device->GetDevicePort()->Write(QByteArray("M205 "+_jerk_limits_m205+"\n"));
        }
        else if(!_jerk_finished){
            _device->GetDevicePort()->Write(QByteArray("G28 \n"));
            _jerk_finished=true;
            _go_home_finished=true;
            Finish(true);
        }
    }
    else{
        if(ba.contains("Error:No Checksum") || ba.contains("Resend:"))
        {
            SetError(CommandError::NoChecksum);
        }
        else if(ba.toLower().contains("busy")||ba.toLower().startsWith("t:"))
        {
            SetError(CommandError::Busy);
            Finish(false);
        }
        else{
            SetError(CommandError::UnknownError);
            Finish(false);
        }
    }
}

void GCode::PrePrint::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::PrePrint::Finish(bool b)
{
    GCodeCommand::Finish(b);
}
