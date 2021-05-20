#include "chitufileslist.h"

#include "../../device.h"
#include "../../deviceport.h"



GCode::Chitu::ChituFilesList::ChituFilesList(Device *device):ChituGcodeCommand(device,"M20")
{
    _is_begin=false;
    _files_list_end=false;
}

void GCode::Chitu::ChituFilesList::InsideStart()
{
    _device->GetDeviceConnection()->Write("M20\n");
}

void GCode::Chitu::ChituFilesList::InsideStop()
{
}

QMap<QByteArray, size_t> GCode::Chitu::ChituFilesList::GetFilesList()
{
    QMap<QByteArray, size_t> map;
    if(_finished)
    {
        for(int i=0;i<_result_files.length();i++)
        {
            map.insert(_result_files[i],_sizes[i]);
        }
    }
    return map;
}

void GCode::Chitu::ChituFilesList::OnAvailableData(const QByteArray &ba)
{
    if(ba.toLower().contains("begin file list"))
        _is_begin=true;
    else if(ba.toLower().contains("end file list"))
    {
        _files_list_end=true;
    }
    else if(_is_begin && !_files_list_end)
    {
        int index=ba.lastIndexOf(" ");
        QByteArray file=ba.left(index);
        _result_files.append(file);
        _sizes.append(ba.mid(index+1,ba.length()-index-1).trimmed().toUInt());
    }
    else if(ba.toLower().contains("ok ") && _files_list_end){
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

void GCode::Chitu::ChituFilesList::OnAllDataWritten(bool success)
{
    if(!success)
    {
        SetError(CommandError::WriteError);
        Finish(false);
    }
}

void GCode::Chitu::ChituFilesList::Finish(bool b)
{
    GCodeCommand::Finish(b);
}
