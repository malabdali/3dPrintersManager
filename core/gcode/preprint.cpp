#include "preprint.h"
#include <QByteArray>
#include "../device.h"
#include "../deviceport.h"
GCode::PrePrint::PrePrint(Device* device,int fanSpeed,int acceleration,int jerk,double ePosition,bool goHome,bool EAbsolute):GCodeCommand(device,"M106")
{
    _fan_speed=fanSpeed;
    _acceleration=acceleration;
    _go_home=goHome;
    _jerk=jerk;
    _e_absolute=EAbsolute;
    _e_position=ePosition;
    _e_position_finished=false;
    _e_absolute_finished=false;
    _go_home_finished=false;
    _acceleration_finished=false;
    _fan_speed_finished=false;
    _jerk_finished=false;

    if(_fan_speed<0)
        _fan_speed_finished=true;
    if(_acceleration<0)
        _acceleration_finished=true;
    if(_jerk<0)
        _jerk_finished=true;
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
    else if(!_acceleration_finished)
        _device->GetDevicePort()->Write(QByteArray("M204 S"+QByteArray::number(_acceleration)+"\n"));
    else if(!_jerk_finished)
        _device->GetDevicePort()->Write(QByteArray("M205 X"+QByteArray::number(_jerk)+" Y"+QByteArray::number(_jerk)+"\n"));
    else if(!_go_home_finished)
        _device->GetDevicePort()->Write(QByteArray("G28 \n"));



}

void GCode::PrePrint::InsideStop()
{
}

void GCode::PrePrint::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().startsWith("ok") || ba.toLower().startsWith("x:")){
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
            _device->GetDevicePort()->Write(QByteArray("M204 S"+QByteArray::number(_acceleration)+"\n"));
        }
        else if(!_acceleration_finished){
            _acceleration_finished=true;
            _device->GetDevicePort()->Write(QByteArray("M205 X"+QByteArray::number(_jerk)+" Y"+QByteArray::number(_jerk)+"\n"));
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
