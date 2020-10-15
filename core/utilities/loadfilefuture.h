#ifndef LOADFILEFUTURE_H
#define LOADFILEFUTURE_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent/QtConcurrentRun>
#include <functional>

class LoadFileFuture : public QObject
{
    Q_OBJECT
private:
    QFuture<QList<QByteArray>> _future;
    QFutureWatcher<QList<QByteArray>> _watcher;
    QString _file_name;
    std::function<void(QList<QByteArray>)> _callback;
    quint64 _offset;
public:
    explicit LoadFileFuture(QString file,std::function<void(QList<QByteArray>)> callback,quint64 offset=1,QObject *parent = nullptr);
private://methods
    QList<QByteArray> LoadFile();

signals:
private slots:
    void WhenFinished();


};

#endif // LOADFILEFUTURE_H
