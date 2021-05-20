#include "printingstats.h"
#include "../device.h"
#include "../deviceport.h"
#include <regex>

GCode::PrintingStats::PrintingStats(Device *device):GCodeCommand(device,"M27")
{
    _is_printing=false;
    _percent=0;
    _printin_stats_updated=false;
}

double GCode::PrintingStats::GetPercent()
{
    return _percent;
}

uint32_t GCode::PrintingStats::GetPrintedBytes()
{
    return _printed_bytes;
}

uint32_t GCode::PrintingStats::GetTotalBytes()
{
    return _total_bytes;
}

bool GCode::PrintingStats::IsPrinting()
{
    return _is_printing;

}

void GCode::PrintingStats::InsideStart()
{
    _device->GetDeviceConnection()->Write(QByteArray("M27 \n"));
}

void GCode::PrintingStats::InsideStop()
{
}


void GCode::PrintingStats::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().startsWith("ok") && _printin_stats_updated){
        Finish(true);
    }
    else if(ba.toLower().contains(QByteArray("Not SD printing").toLower()))
    {
        _printin_stats_updated=true;
        _is_printing=false;
    }
    else if(ba.toLower().contains(QByteArray("SD printing byte").toLower())){
        _printin_stats_updated=true;
        _is_printing=true;
        std::regex rgx(R"(\d+/\d+)");
        std::match_results<QByteArray::ConstIterator> matches;
        std::regex_search(ba.begin(),ba.end(), matches, rgx);
        _printed_bytes=QString(matches[0].first).split('/')[0].toUInt();
        _total_bytes=QString(matches[0].first).split('/')[1].toUInt();
        _percent=(double) _printed_bytes/(double) _total_bytes;
        _percent*=100.0;
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


void GCode::PrintingStats::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::PrintingStats::Finish(bool b)
{
    GCodeCommand::Finish(b);
}

