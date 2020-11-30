#include "loadfilefuture.h"
#include "QFile"

LoadFileFuture::LoadFileFuture(QString file, std::function<void (QList<QByteArray>)> callback,quint64 offset, QObject *parent):
    _file_name(file),_callback(callback),_offset(offset)
{
    QObject::connect(&_watcher,&QFutureWatcher<QList<QByteArray>>::finished,this,&LoadFileFuture::WhenFinished);
    _future=QtConcurrent::run(this,&LoadFileFuture::LoadFile);
    _watcher.setFuture(_future);
}

QList<QByteArray> LoadFileFuture::LoadFile()
{
    QFile f(_file_name);
    f.open(QIODevice::ReadOnly);
    QByteArray data=f.readAll();
    QByteArrayList lines=data.split('\n');
    /*lines.erase(std::remove_if(lines.begin(),lines.end(),[](QByteArray& line)->bool{
        return !line.startsWith("M")&& !line.startsWith("G");
    }),lines.end());*/
    QList<QByteArray> newLines;
    int i=_offset;
    for(QByteArray& line:lines){
        line.replace('\r',"");
        line=line.simplified();
        line=line.trimmed();
        if(!line.startsWith("M")&& !line.startsWith("G")){
            continue;
        }
        int index=line.indexOf(';');
        line.remove(index,line.length()-index);
        line.prepend(QByteArray("N")+QByteArray::number(i)+" ");
        uint8_t checksum=std::accumulate(line.begin(),line.end(),0,[](uint8_t v,char c){return c^v;});
        line.append(QByteArray("*")+QByteArray::number(checksum)+"\n");
        i++;
        newLines.append(line);
    }
    return newLines;
}

void LoadFileFuture::WhenFinished()
{
    _callback(_future.result());
    this->deleteLater();
}

