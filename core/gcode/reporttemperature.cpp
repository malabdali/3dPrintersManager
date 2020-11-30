#include "reporttemperature.h"
#include "../deviceport.h"
#include <QRegularExpression>
#include <regex>
#include <string>
GCode::ReportTemperature::ReportTemperature(Device *device):GCodeCommand(device,"M105")
{

}

double GCode::ReportTemperature::GetHotendTemperature() const
{
    return _hotend_temperature;
}

double GCode::ReportTemperature::GetBedTemperature() const
{
    return _bed_temperature;
}


void GCode::ReportTemperature::InsideStart()
{
    _device->GetDevicePort()->Write(QByteArray("M105 \n"));
}

void GCode::ReportTemperature::InsideStop()
{
}


void GCode::ReportTemperature::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().startsWith("ok t:")){
        std::match_results<QByteArray::const_iterator> matches;
        std::regex_search(ba.begin(),ba.end(),matches,std::regex(R"((T:\d+\.\d+))"));
        for(auto m:matches)
        {
            QList<QByteArray> list=QByteArray::fromStdString(m.str()).split(':');
            _hotend_temperature=list[1].toDouble();
        }
        std::regex_search(ba.begin(),ba.end(),matches,std::regex(R"((B:\d+\.\d+))"));
        for(auto m:matches)
        {
            QList<QByteArray> list=QByteArray::fromStdString(m.str()).split(':');
            _bed_temperature= list[1].toDouble();
        }
        Finish(true);
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


void GCode::ReportTemperature::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::ReportTemperature::Finish(bool b)
{
    GCodeCommand::Finish(b);
}
