#include "uploadfile.h"
#include "../device.h"
GCode::UploadFile::UploadFile(Device *device, std::function<void (bool)> callback,QByteArray fileName,const QByteArrayList& data):
    GCodeCommand(device,"M28"),_callback(callback),_file_name(fileName),_data(data)
{
    qDebug()<<data;
    _resend=false;
    _upload_stage=false;
    _start_upload=false;
    _counter=0;
    _open_failed=false;
}

void GCode::UploadFile::Start()
{
    GCodeCommand::Start();
    _device->Write(QByteArray("M28 ")+_file_name+"\n");
}

void GCode::UploadFile::Stop()
{
    Finish(false);
    _device->StopWrite();
}

double GCode::UploadFile::GetProgress()
{
    QMutexLocker locker (&_mutex);
    return _progress;
}

void GCode::UploadFile::OnAvailableData(const QByteArray &ba)
{
    if(ba.contains("Writing to file:") && !_start_upload)
    {
        _start_upload=true;
    }
    else if(ba.contains("open failed")){
        _start_upload=false;
        _open_failed=true;
    }
    else if(ba.contains("ok") && _start_upload && !_upload_stage){
        _upload_stage=true;
        this->_device->Write(_data[0]);
    }
    else if(ba.contains("ok") && (uint32_t)_data.length()>(_counter+1) && _upload_stage)
    {
        if(_resend)
        {
            _resend=false;
            _counter-=1;
        }
        _mutex.lock();
        _progress=((double)this->_counter/(double)this->_data.length())*100.0;
        _mutex.unlock();
        this->_device->Write(_data[++_counter]);
    }
    else if((uint32_t)_data.length()==(_counter+1) && ba.contains("ok") && _upload_stage)
    {
        _upload_stage=false;
        _device->Write("M29\n");
    }
    else if(ba.contains("Done saving file")){
        //qDebug()<<(std::chrono::system_clock::now()-from).count();
        Finish(true);
    }
    else if(ba.contains("Resend: ")){

        //_device->StopWrite();
        //_device->ClearLines();
        _resend=true;
        //uint32_t ln=ba.mid(8,ba.length()).simplified().trimmed().toUInt();
        //_counter=ln-1;
        //this->_device->Write(_outputs[ln-1]);
    }
    else if(ba.contains("ok")&& _open_failed){
        Finish(false);
    }
}

void GCode::UploadFile::OnDataWritten()
{

}

void GCode::UploadFile::OnAllDataWritten(bool)
{

}

void GCode::UploadFile::Finish(bool b)
{
    _callback(b);
    GCodeCommand::Finish(b);
}

