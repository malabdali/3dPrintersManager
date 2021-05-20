#ifndef LOADFILEFUTURE_H
#define LOADFILEFUTURE_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <functional>

class LoadPrintableFuture : public QObject
{
    Q_OBJECT
private:
    QFuture<QList<QByteArray>> _future;
    QFutureWatcher<QList<QByteArray>> _watcher;
    QByteArray _data;
    QString _file_name;
    std::function<void(QList<QByteArray>,QByteArray)> _callback;
    quint64 _offset;
    int _chunkSize;
public:
    explicit LoadPrintableFuture(QString file,std::function<void(QList<QByteArray>,QByteArray)> callback,quint64 offset=1,bool isGcode=false,int chunkSize=0,QObject *parent = nullptr);
    void Stop();
private://methods
    QList<QByteArray> LoadGCodeFile();
    QList<QByteArray> LoadBinaryFile();

signals:
private slots:
    void WhenFinished();


};

#endif // LOADFILEFUTURE_H
