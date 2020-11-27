#include "fileslist.h"
#include "../device.h"
#include "../deviceport.h"



GCode::FilesList::FilesList(Device *device):GCodeCommand(device,"M20")
{
    _is_begin=false;
    _files_list_end=false;
}

void GCode::FilesList::InsideStart()
{
    _device->GetDevicePort()->Write("M20\n");
}

void GCode::FilesList::InsideStop()
{
}

QMap<QByteArray, size_t> GCode::FilesList::GetFilesList()
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

void GCode::FilesList::OnAvailableData(const QByteArray &ba)
{
    if(ba.contains("Begin file list"))
        _is_begin=true;
    else if(ba.contains("End file list"))
    {
        _files_list_end=true;
    }
    else if(_is_begin && ba!="ok")
    {
        int index=ba.indexOf(" ");
        QByteArray file=ba.left(index);
        _result_files.append(file);
        _sizes.append(ba.mid(index+1,ba.length()-index-1).trimmed().toUInt());
    }
    else if(ba=="ok" && _files_list_end){
        Finish(true);
    }
    else{
        Finish(false);
    }
}

void GCode::FilesList::OnAllDataWritten(bool success)
{

}

void GCode::FilesList::Finish(bool b)
{
    GCodeCommand::Finish(b);
}