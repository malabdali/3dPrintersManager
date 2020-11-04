#include "printingstats.h"
#include "../device.h"
#include "../deviceport.h"
#include <regex>

GCode::PrintingStats::PrintingStats(Device *device):GCodeCommand(device,"M27")
{
    _is_printing=false;
    _percent=0;
}

double GCode::PrintingStats::GetPercent()
{
    return _percent;

}

bool GCode::PrintingStats::IsPrinting()
{
    return _is_printing;

}

void GCode::PrintingStats::InsideStart()
{
    _device->GetDevicePort()->Write(QByteArray("M27 \n"));
}

void GCode::PrintingStats::InsideStop()
{
}


void GCode::PrintingStats::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().startsWith("ok")){
        Finish(true);
    }
    else if(ba.toLower().contains(QByteArray("Not SD printing").toLower()))
    {
        _is_printing=false;
    }
    else if(ba.toLower().contains(QByteArray("SD printing byte").toLower())){
        _is_printing=true;
        std::regex rgx(R"(\d+/\d+)");
        std::match_results<QByteArray::ConstIterator> matches;
        std::regex_search(ba.begin(),ba.end(), matches, rgx);
        _percent=(double) QString(matches[0].first).split('/')[0].toUInt()/(double) QString(matches[0].first).split('/')[1].toUInt();
    }
    else{
        if(ba.contains("Error:No Checksum"))
        {
            SetError(CommandError::NoChecksum);
        }
    }
}


void GCode::PrintingStats::OnAllDataWritten()
{

}

void GCode::PrintingStats::Finish(bool b)
{
    GCodeCommand::Finish(b);
}

