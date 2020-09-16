#include "devicefunctions.h"
#include <QDebug>
#include <algorithm>
#include "device.h"
DeviceFunctions::DeviceFunctions(Device *device, DeviceFunctions::Function fun,QByteArray data1,QByteArray data2):QObject(nullptr),
    _device(device),_function(fun),_data1(data1),_data2(data2),_result(""),_started(false)
{
    qDebug()<<"create function";
}

void DeviceFunctions::Start()
{

    this->setParent(_device);
    QObject::connect(_device,&Device::NewLinesAvailable,this,&DeviceFunctions::WhenLineAvailable);
    QObject::connect(_device,&Device::EndWrite,this,&DeviceFunctions::WhenWriteFinished);
    QObject::connect(_device,&Device::BytesWritten,this,&DeviceFunctions::WhenWriteLine);
    _counter=0;
    _resend=false;
    _upload_stage=false;
    if(_function==Function::FileList)
    {
        _device->Write("M20\n");
    }
    else if(_function==Function::UploadFile)
    {
        _device->Write(QByteArray("M28 ")+_data1+"\n");
    }
}

//start files list
void DeviceFunctions::FileListFunctionDataAvailable(QByteArray ba)
{
    if(ba.contains("Begin file list"))
        _result+="/";
    else if(ba.contains("End file list"))
    {
        _result=_result.mid(1,_result.length()-2);
    }
    else if(_result.contains('/') && ba.contains(".GCO"))
    {
        int index=ba.indexOf(".GCO");
        QByteArray file=ba.left(index+4);
        _result+=file+',';
    }
    else if(ba=="ok"){
        Finish(true);
    }

}

void DeviceFunctions::FileListFunctionWriteLine()
{

}

void DeviceFunctions::FileListFunctionFinishWrite(bool b){

    if(b==false)
    {
        Finish(false);
    }
}

void DeviceFunctions::FileListFunctionStop()
{
    Finish(false);
}

//end file lists

//upload file
void DeviceFunctions::UploadFileFunctionDataAvailable(QByteArray ba)
{
    if(ba.contains("Writing to file:") && _outputs.isEmpty())
    {
        _outputs.append("start");
    }
    else if(ba.contains("ok") && _outputs.length()==1&& _outputs[0]=="start"){
        qDebug()<<"start now ";
        QByteArrayList lines=_data2.split('\n');
        qDebug()<<lines.length();
        lines.erase(std::remove_if(lines.begin(),lines.end(),[](QByteArray& line)->bool{
            return !line.startsWith("M")&& !line.startsWith("G");
        }),lines.end());
        int i=1;
        for(QByteArray& line:lines){
            int index=line.indexOf(';');
            line.remove(index,line.length()-index);
            line.replace('\r',"");
            line=line.trimmed();
            line.prepend(QByteArray("N")+QByteArray::number(i)+" ");
            uint8_t checksum=std::accumulate(line.begin(),line.end(),0,[](uint8_t v,char c){return c^v;});
            line.append(QByteArray("*")+QByteArray::number(checksum)+"\n");
            _outputs.clear();
            _outputs=lines;
            i++;
        }
        qDebug()<<_outputs.length();
        _upload_stage=true;
        this->_device->Write(_outputs[0]);
    }
    else if(ba.contains("ok") && (uint32_t)_outputs.length()>(_counter+1) && _upload_stage)
    {
        if(_resend)
        {
            _resend=false;
            _counter-=1;
        }
        this->_device->Write(_outputs[++_counter]);
    }
    else if((uint32_t)_outputs.length()==(_counter+1) && ba.contains("ok") && _upload_stage)
    {
        _upload_stage=false;
        _device->Write("M29\n");
    }
    else if(ba.contains("Done saving file")){
        //qDebug()<<(std::chrono::system_clock::now()-from).count();
        qDebug()<<"file uploaded"<<_counter;
        Finish(true);
    }
    else if(ba.contains("Resend: ")){
        //_device->StopWrite();
        //_device->ClearLines();
        qDebug()<<"resend";
        _resend=true;
        //uint32_t ln=ba.mid(8,ba.length()).simplified().trimmed().toUInt();
        //_counter=ln-1;
        //this->_device->Write(_outputs[ln-1]);
        qDebug()<<ba;
    }
    else if(_outputs.length()==0){
    }

}

void DeviceFunctions::UploadFileFunctionWriteLine()
{
    //qDebug()<<"write line";
}

void DeviceFunctions::UploadFileFunctionFinishWrite(bool b)
{
    //qDebug()<<"finish write"<<b;
    if(b==false)
    {
        Finish(false);
    }
}

void DeviceFunctions::UploadFileFunctionStop()
{
    Finish(false);
}

//end upload file

bool DeviceFunctions::IsFinished(){
    return _finished;
}

void DeviceFunctions::Stop()
{

    if(_function==Function::FileList)
        FileListFunctionStop();
    if(_function==Function::UploadFile)
        UploadFileFunctionStop();
}

bool DeviceFunctions::IsStarted() const
{
    return _started;
}

DeviceFunctions::~DeviceFunctions()
{
}


void DeviceFunctions::Finish(bool b)
{
    qDebug()<<"function is finished";
    _finished=true;
    QObject::disconnect(_device,&Device::NewLinesAvailable,this,&DeviceFunctions::WhenLineAvailable);
    QObject::disconnect(_device,&Device::EndWrite,this,&DeviceFunctions::WhenWriteFinished);
    QObject::disconnect(_device,&Device::BytesWritten,this,&DeviceFunctions::WhenWriteLine);

    //_device->StopWrite();
    emit Finished(b);
}

void DeviceFunctions::WhenLineAvailable(QByteArrayList list)
{
    while (_device->IsThereAvailableLines()) {
        if(_function==Function::FileList)
            FileListFunctionDataAvailable(_device->ReadLine());
        if(_function==Function::UploadFile)
            UploadFileFunctionDataAvailable(_device->ReadLine());
    }

}

void DeviceFunctions::WhenWriteFinished(bool b)
{
    if(_function==Function::FileList)
        FileListFunctionFinishWrite(b);
    if(_function==Function::UploadFile)
        UploadFileFunctionFinishWrite(b);
}

void DeviceFunctions::WhenWriteLine()
{
    if(_function==Function::FileList)
        FileListFunctionWriteLine();
    if(_function==Function::UploadFile)
        UploadFileFunctionWriteLine();
}

