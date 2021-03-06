#ifndef REMOTESERVER_H
#define REMOTESERVER_H

#include <QObject>
#include <QApplication>
#include <QNetworkAccessManager>
#include "device.h"
#include <functional>
#include "../config.h"
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include<QJsonObject>
#include<QJsonDocument>
#include<chrono>

class RemoteServer : public QObject
{
    Q_OBJECT
private://static
    static RemoteServer *_singleton;
public://static
    static RemoteServer *GetInstance();
private://non static fiels
    QNetworkAccessManager* _network;
    std::map<QNetworkReply*,std::function<void(QNetworkReply*)>> _callbacks;
    std::map<QNetworkReply*,std::pair<const std::chrono::time_point<std::chrono::steady_clock>,uint16_t>> _timers;
public:
    explicit RemoteServer(QObject *parent = nullptr);
    QNetworkReply* SendRequest(QUrlQuery,QString,std::function<void(QNetworkReply*)>,uint16_t secondsTimeout=DEFAULT_HTTP_REQUEST_TIMEOUT);
    QNetworkReply* SendSelectQuery(std::function<void (QNetworkReply *)> callback, QString table, QString where="{}",uint16_t secondsTimeout=DEFAULT_HTTP_REQUEST_TIMEOUT);
    QNetworkReply *SendUpdateQuery(std::function<void(QNetworkReply*)> callback,QString table,QVariantMap data,QByteArray id,uint16_t secondsTimeout=DEFAULT_HTTP_REQUEST_TIMEOUT);
    QNetworkReply* SendInsertQuery(std::function<void(QNetworkReply*)> callback,QString table,QVariantMap data,uint16_t secondsTimeout=DEFAULT_HTTP_REQUEST_TIMEOUT);
    QNetworkReply *SendDeleteQuery(std::function<void(QNetworkReply*)> callback,QString table,QByteArray id,uint16_t secondsTimeout=DEFAULT_HTTP_REQUEST_TIMEOUT);
    QNetworkReply *Download(std::function<void(QNetworkReply*)> callback,QString path);
    QNetworkReply *UploadImage(QString file,QString uploadPath,std::function<void(QNetworkReply*)> callback);
    bool IsSuccess(QNetworkReply* reply);
    QJsonValue GetJSONValue(QNetworkReply* reply);
    void RemoveRequest(QNetworkReply* reply);
    bool DownloadIsSuccess(QNetworkReply *reply);
private://methods
    void timerEvent(QTimerEvent* te)override;
private slots:
    void OnFinish(QNetworkReply *rep);

signals:
    void Finished(QNetworkReply *);

};

#endif // REMOTESERVER_H
