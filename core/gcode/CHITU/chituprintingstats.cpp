#include "chituprintingstats.h"
#include "../../device.h"
#include "../../deviceport.h"
#include <regex>

GCode::Chitu::ChituPrintingStats::ChituPrintingStats(Device *device):ChituGcodeCommand(device,"M27")
{
    _is_printing=false;
    _percent=0;
    _printin_stats_updated=false;
}

double GCode::Chitu::ChituPrintingStats::GetPercent()
{
    return _percent;
}

uint32_t GCode::Chitu::ChituPrintingStats::GetPrintedBytes()
{
    return _printed_bytes;
}

uint32_t GCode::Chitu::ChituPrintingStats::GetTotalBytes()
{
    return _total_bytes;
}

bool GCode::Chitu::ChituPrintingStats::IsPrinting()
{
    return _is_printing;

}

void GCode::Chitu::ChituPrintingStats::InsideStart()
{
    _device->GetDeviceConnection()->Write(QByteArray("M27 \n"));
}

void GCode::Chitu::ChituPrintingStats::InsideStop()
{
}


void GCode::Chitu::ChituPrintingStats::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().startsWith("ok") && _printin_stats_updated){
        Finish(true);
    }
    else if(ba.toLower().contains(QByteArray("It's not printing now").toLower()))
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


void GCode::Chitu::ChituPrintingStats::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::Chitu::ChituPrintingStats::Finish(bool b)
{
    GCodeCommand::Finish(b);
}

