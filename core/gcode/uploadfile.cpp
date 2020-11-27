#include "uploadfile.h"
#include "../device.h"
#include "QTimer"
#include "../deviceport.h"
#include "../device.h"
GCode::UploadFile::UploadFile(Device *device, std::function<void (bool)> callback,QByteArray fileName,const QByteArrayList& data,quint64 firstLine):
    GCodeCommand(device,"M28"),_callback(callback),_file_name(fileName),_file_size(0),_data(data),_first_line(firstLine)
{
    _resend_tries=RESEND_TRIES;
    _resend=false;
    _upload_stage=false;
    _counter=0;
    _open_failed=false;
    _open_success=false;
    _wait_resend=false;
}

void GCode::UploadFile::InsideStart()
{
    _end_timer=new QTimer();
    _end_timer->moveToThread(this->thread());
    QObject::connect(_end_timer,&QTimer::timeout,this,&UploadFile::Send29);
    for(QByteArray& ba :_data)
    {
        _file_size+=ba.mid(ba.indexOf(" "),ba.indexOf("*")-ba.indexOf(" ")+1).length();
    }
    _device->GetDevicePort()->Write(QByteArray("M28 ")+_file_name+"\n");
}

void GCode::UploadFile::InsideStop()
{
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
    this->_device->GetDevicePort()->Write(_data[++_counter]);
}

void GCode::UploadFile::Send()
{
    _progress=((double)this->_counter/(double)this->_data.length())*100.0;
    this->_device->GetDevicePort()->Write(_data[++_counter]);
}

void GCode::UploadFile::Send29()
{
    _upload_stage=false;
    _device->GetDevicePort()->Write("M29\n");
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
        this->_device->Write(_data[0]);
    }
    else if((uint32_t)_data.length()<=(_counter+1) && ba.contains("ok") && _upload_stage)
    {
        _end_timer->start(500);
    }
    else if(ba.contains("Done saving file")|| (_open_success && !_upload_stage && ba.contains("ok"))){
        //qDebug()<<(std::chrono::system_clock::now()-from).count();
        Finish(true);
    }
    else if(ba.contains("Resend: ")){
        if(RESEND_TRIES>0){
            _resend_tries--;
            if(_resend_tries<0){
                Finish(false);
                return;
            }
        }
        _device->GetDevicePort()->Clear();
        _resend=true;
        uint32_t ln=ba.mid(8).simplified().trimmed().toUInt()-_first_line;
        if(ln>=_counter)
        {
            _end_timer->start(500);
        }
        else
            _counter=ln-1;
        this->_device->Write(_data[_counter]);
    }
    else if(ba.contains("ok")&& _open_failed){
        Finish(false);
    }
}

void GCode::UploadFile::OnAllDataWritten(bool success)
{
    if(!success)
    {
        _counter--;
        qDebug()<<"UploadFile::OnAllDataWritten : write failed";
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
            Send();
            //QTimer::singleShot(_data[_counter].length()/20,this,&UploadFile::Send);
        }

    }
}

void GCode::UploadFile::Finish(bool b)
{
    qDebug()<<"UploadFile::Finish";
    if(!_finished && !b)
        Send29();
    GCodeCommand::Finish(b);
}
