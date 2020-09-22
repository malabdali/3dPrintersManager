#include "fileslist.h"
#include "../device.h"



GCode::FilesList::FilesList(Device *device, std::function<void (bool, QByteArrayList,QList<quint32>)> callback):GCodeCommand(device,"M20"),_callback(callback)
{
    _is_begin=false;
    _files_list_end=false;
}

void GCode::FilesList::Start()
{
    GCodeCommand::Start();
    _device->Write("M20\n");
}

void GCode::FilesList::Stop()
{
    Finish(false);
}

void GCode::FilesList::OnAvailableData(const QByteArray &ba)
{
    qDebug()<<ba;
    if(ba.contains("Begin file list"))
        _is_begin=true;
    else if(ba.contains("End file list"))
    {
        _files_list_end=true;
    }
    else if(_is_begin && (ba.contains(".GCO")||ba.contains(".gcode")))
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

void GCode::FilesList::OnDataWritten()
{

}

void GCode::FilesList::OnAllDataWritten(bool)
{

}

void GCode::FilesList::Finish(bool b)
{
    qDebug()<<" finish file list";
    _callback(b,_result_files,_sizes);
    GCodeCommand::Finish(b);
}
