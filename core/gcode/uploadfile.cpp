#include "uploadfile.h"
#include "../device.h"
#include "QTimer"
#include "../deviceport.h"
#include "../device.h"
#include "../../config.h"
GCode::UploadFile::UploadFile(Device *device, QByteArray fileName, const QByteArrayList& data, quint64 firstLine, unsigned speed):
    GCodeCommand(device,"M28"),_first_line(firstLine),_file_name(fileName),_file_size(0),_data(data),_speed(speed)
{
    _resend_tries=RESEND_TRIES;
    _resend=false;
    _upload_stage=false;
    _counter=0;
    _open_failed=false;
    _open_success=false;
    _wait_resend=false;
    _file_saved=false;
}

void GCode::UploadFile::InsideStart()
{
    _end_timer=new QTimer(this);
    _send_timer=new QTimer(this);
    _send_timer->setSingleShot(true);

    //_end_timer->moveToThread(this->thread());
    //_send_timer->moveToThread(this->thread());
    QObject::connect(_send_timer,&QTimer::timeout,this,&UploadFile::Send);
    QObject::connect(_end_timer,&QTimer::timeout,this,&UploadFile::Send29);
    for(QByteArray& ba :_data)
    {
        _file_size+=ba.mid(ba.indexOf(" "),ba.indexOf("*")-ba.indexOf(" ")+1).length();
    }
    _device->GetDeviceConnection()->Write(QByteArray("M28 ")+_file_name+"\n");
}

void GCode::UploadFile::InsideStop()
{
    _send_timer->stop();
    _no_response_timer->stop();
    _end_timer->stop();
    Finish(false);
}

double GCode::UploadFile::GetProgress()
{
    QMutexLocker locker(&_mutex);
    return _progress;
}

QByteArray GCode::UploadFile::GetFileName()
{
    return _file_name;
}

QByteArrayList GCode::UploadFile::GetData()
{
    return _data;
}

quint32 GCode::UploadFile::GetSize()
{
    return _file_size;
}

bool GCode::UploadFile::IsSuccess()
{
    return _is_success;
}

void GCode::UploadFile::Resend()
{
    _wait_resend=false;
    _resend=false;
    this->_device->GetDeviceConnection()->Write(_data[++_counter]);
}

void GCode::UploadFile::Send()
{
    if((uint32_t)_data.length()<=(_counter+1)){
        this->_device->GetDeviceConnection()->Write(_data[_data.length()-1]);
        return;
    }
    _progress=((double)this->_counter/(double)this->_data.length())*100.0;
    this->_device->GetDeviceConnection()->Write(_data[++_counter]);
}

void GCode::UploadFile::Send29()
{
    _upload_stage=false;
    _device->GetDeviceConnection()->Write("M29\n");
}

void GCode::UploadFile::OnAvailableData(const QByteArray &ba)
{
    if(_end_timer->isActive())
    {
        _end_timer->stop();
    }
    if(_wait_resend)
        return;
    if(ba.contains("Writing to file:"))
    {
        _open_success=true;
    }
    else if(ba.toLower().contains("open failed")){
        _open_failed=true;
    }
    else if(ba.contains("ok") && _open_success && !_upload_stage && _counter==0){
        _upload_stage=true;
        this->_device->GetDeviceConnection()->Write(_data[0]);
    }
    else if((uint32_t)_data.length()<=(_counter+1) && ba.contains("ok") && _upload_stage)
    {
        Send();
    }
    else if(ba.contains("Done saving file")){
        _file_saved=true;
    }
    else if(_open_success && !_upload_stage && _file_saved && ba.contains("ok")){
        Finish(true);
    }
    else if(_open_success && !_upload_stage && !_file_saved && ba.contains("ok")){
        return;
    }
    else if(ba.contains("Resend: ")){
        if(RESEND_TRIES>0){
            _resend_tries--;
            if(_resend_tries<0){
                Finish(false);
                return;
            }
        }
        _device->GetDeviceConnection()->Clear();
        if(_send_timer->isActive())_send_timer->stop();
        _resend=true;
        uint32_t ln=ba.mid(8).simplified().trimmed().toUInt()-_first_line;
        if(ln>=_counter)
        {
            _end_timer->stop();
            _end_timer->start(1000);
        }
        else{
            if(!_upload_stage){
                _upload_stage=true;
            }
            _counter=ln-1;
        }
        OnAllDataWritten(true);
        //this->_device->GetDevicePort()->Write(_data[_counter]);
    }
    else if(ba.contains("ok")&& _open_failed){
        Finish(false);
    }
    else if(ba.toLower().contains("busy")||ba.toLower().startsWith("t:"))
    {
        SetError(CommandError::Busy);
        Finish(false);
    }
}

void GCode::UploadFile::OnAllDataWritten(bool success)
{
    if(!success)
    {
        _counter--;
    }
    if((uint32_t)_data.length()>(_counter+1) && _upload_stage)
    {
        if(_resend && !_wait_resend)
        {
            _wait_resend=true;

            QTimer::singleShot(10,this,&UploadFile::Resend);
        }
        else if(!_resend)
        {
            if(_speed>0)
                _send_timer->start(_data[_counter].length()/_speed);
            else
                Send();
        }

    }
}

void GCode::UploadFile::Finish(bool b)
{
    if(!_finished && !b)
        Send29();
    GCodeCommand::Finish(b);
}
