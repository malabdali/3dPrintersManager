#include "chituuploadfile.h"
#include "../../device.h"
#include "QTimer"
#include "../../deviceport.h"
#include "../../../config.h"

GCode::Chitu::ChituUploadFile::ChituUploadFile(Device *device, QByteArray fileName, const QByteArrayList& data,  unsigned speed):
    ChituGcodeCommand(device,"M28"),_file_name(fileName),_file_size(0),_data(data),_speed(speed)
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

void GCode::Chitu::ChituUploadFile::InsideStart()
{
    _end_timer=new QTimer(this);
    _send_timer=new QTimer(this);
    _send_timer->setSingleShot(true);

    //_end_timer->moveToThread(this->thread());
    //_send_timer->moveToThread(this->thread());
    QObject::connect(_send_timer,&QTimer::timeout,this,&ChituUploadFile::Send);
    QObject::connect(_end_timer,&QTimer::timeout,this,&ChituUploadFile::Send29);
    for(QByteArray& ba :_data)
    {
        _file_size+=ba.mid(ba.indexOf(" "),ba.indexOf("*")-ba.indexOf(" ")+1).length();
    }
    _device->GetDeviceConnection()->Write(QByteArray("M28 ")+_file_name+"\n");
    _send_timer->stop();
    _send_timer->setSingleShot(true);
    _send_timer->start(3000);
}

void GCode::Chitu::ChituUploadFile::InsideStop()
{
    _send_timer->stop();
    _no_response_timer->stop();
    _end_timer->stop();
    Finish(false);
}

double GCode::Chitu::ChituUploadFile::GetProgress()
{
    QMutexLocker locker(&_mutex);
    return _progress;
}

QByteArray GCode::Chitu::ChituUploadFile::GetFileName()
{
    return _file_name;
}

QByteArrayList GCode::Chitu::ChituUploadFile::GetData()
{
    return _data;
}

quint32 GCode::Chitu::ChituUploadFile::GetSize()
{
    return _file_size;
}

bool GCode::Chitu::ChituUploadFile::IsSuccess()
{
    return _is_success;
}

void GCode::Chitu::ChituUploadFile::Resend()
{
    _wait_resend=false;
    _resend=false;
    this->_device->GetDeviceConnection()->Write(_data[++_counter]);
}

void GCode::Chitu::ChituUploadFile::Send()
{
    if((uint32_t)_data.length()<=_counter){
        this->_device->GetDeviceConnection()->Write(_data[_data.length()-1]);
        return;
    }
    _progress=((double)this->_counter/(double)this->_data.length())*100.0;
    this->_device->GetDeviceConnection()->Write(_data[_counter]);
}

void GCode::Chitu::ChituUploadFile::Send29()
{
    _upload_stage=false;
    _device->GetDeviceConnection()->Write("M29\n");
}

void GCode::Chitu::ChituUploadFile::OnAvailableData(const QByteArray &ba)
{
    if(_end_timer->isActive() || _send_timer->isActive())
    {
        _end_timer->stop();
        _send_timer->stop();
    }
    if(ba.contains("ok") && !_open_success && !_open_failed)
    {
        _open_success=true;
        _upload_stage=true;
        _send_timer->stop();
        _send_timer->start(1000);
    }
    else if(ba.toLower().contains("error!cann't")){
        _open_failed=true;
        SetError(CommandError::NoError);
        Finish(false);
    }
    else if(ba.contains("ok") && _upload_stage)
    {
        _resend_tries=RESEND_TRIES;
        _counter++;
        if((uint32_t)_data.length()<=_counter){
            _end_timer->stop();
            _end_timer->start(1000);
            return;
        }
        _send_timer->stop();
        _send_timer->setSingleShot(true);
        if(_speed>0)
            _send_timer->start((uint)(1000/_speed));
        else
            _send_timer->start(1);
    }
    else if(ba.toLower().contains("done saving file")){
        _file_saved=true;
    }
    else if(_open_success && !_upload_stage && _file_saved && ba.contains("ok")){
        Finish(true);
    }
    else if(_open_success && !_upload_stage && !_file_saved && ba.contains("ok")){
        return;
    }
    else if(ba.toLower().contains("Resend")){
        if(RESEND_TRIES>0){
            _resend_tries--;
            if(_resend_tries<0){
                Finish(false);
                return;
            }
        }
        Send();
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

void GCode::Chitu::ChituUploadFile::OnAllDataWritten(bool success)
{
    /*if(!success)
    {
        _counter--;
    }
    if((uint32_t)_data.length()>(_counter+1) && _upload_stage)
    {
        if(_resend && !_wait_resend)
        {
            _wait_resend=true;

            QTimer::singleShot(10,this,&ChituUploadFile::Resend);
        }
        else if(!_resend)
        {
            if(_speed>0)
                _send_timer->start(_data[_counter].length()/_speed);
            else
                Send();
        }

    }*/
}

void GCode::Chitu::ChituUploadFile::Finish(bool b)
{
    if(!_finished && !b)
        Send29();
    GCodeCommand::Finish(b);
}
