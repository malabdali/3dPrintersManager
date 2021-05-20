#include "loadprintablefuture.h"
#include "QFile"

LoadPrintableFuture::LoadPrintableFuture(QString file, std::function<void(QList<QByteArray>, QByteArray)> callback, quint64 offset, bool isGcode, int chunkSize,QObject *parent):
    _file_name(file),_callback(callback),_offset(offset),_chunkSize(chunkSize)
{
    QObject::connect(&_watcher,&QFutureWatcher<QList<QByteArray>>::finished,this,&LoadPrintableFuture::WhenFinished);
    if(isGcode)
        _future=QtConcurrent::run(this,&LoadPrintableFuture::LoadGCodeFile);
    else
        _future=QtConcurrent::run(this,&LoadPrintableFuture::LoadBinaryFile);

    _watcher.setFuture(_future);
}

void LoadPrintableFuture::Stop()
{
    QObject::disconnect(&_watcher,&QFutureWatcher<QList<QByteArray>>::finished,this,&LoadPrintableFuture::WhenFinished);
    _future.cancel();
    _callback(QList<QByteArray>(),"");
    this->deleteLater();
}

QList<QByteArray> LoadPrintableFuture::LoadBinaryFile()
{
    QFile f(_file_name);
    f.open(QIODevice::ReadOnly);
    QByteArrayList lines;
    QList<uint64_t> positions;
    while (f.size()>f.pos()) {
        positions.append(f.pos());
        QByteArray bytes=f.read(_chunkSize);
        lines.append(bytes);
    }
    QList<QByteArray> newLines;
    for(int i=0;i<lines.length();i++){
        uint8_t num8 = 0;
        uint64_t position=positions[i];
        QByteArray line=lines[i];
        line.append((uint8_t)(position & 0xFF));
        line.append((uint8_t)((position >> 8) & 0xFF));
        line.append((uint8_t)((position >> 16) & 0xFF));
        line.append((uint8_t)((position >> 24) & 0xFF));
        for (int ii = 0; ii < line.size(); ii++)
        {
            num8 = (uint8_t)(num8 ^ line.at(ii));
        }
        line.append( num8);
        line.append((uint8_t)0x83);
        newLines.append(line);
    }
    return newLines;
}

QList<QByteArray> LoadPrintableFuture::LoadGCodeFile()
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
        if(line.contains("M109")|| line.contains("M190"))
        {
            line.replace("M109","M104");
            line.replace("M190","M140");
        }
        int index=line.indexOf(';');
        line.remove(index,line.length()-index);
        _data.append(line+" \n");
        line.prepend(QByteArray("N")+QByteArray::number(i)+" ");
        uint8_t checksum=std::accumulate(line.begin(),line.end(),0,[](uint8_t v,char c){return c^v;});
        line.append(QByteArray("*")+QByteArray::number(checksum)+"\n");
        i++;
        newLines.append(line);
    }
    return newLines;
}

void LoadPrintableFuture::WhenFinished()
{
    _callback(_future.result(),_data);
    this->deleteLater();
}

