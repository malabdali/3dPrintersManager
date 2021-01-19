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

class RemoteServer : public QObject
{
    Q_OBJECT
private://static
    static RemoteServer *_singleton;
public://static
    static RemoteServer *GetInstance();
private://non static fiels
    QNetworkAccessManager* _network;
    QMap<QNetworkReply*,std::function<void(QNetworkReply*)>> _callbacks;
public:
    explicit RemoteServer(QObject *parent = nullptr);
    QNetworkReply* SendRequest(QUrlQuery,QString,std::function<void(QNetworkReply*)>);
    QNetworkReply* SendSelectQuery(std::function<void (QNetworkReply *)> callback, QString table, QString where="{}");
    QNetworkReply *SendUpdateQuery(std::function<void(QNetworkReply*)> callback,QString table,QVariantMap data,QByteArray id);
    QNetworkReply* SendInsertQuery(std::function<void(QNetworkReply*)> callback,QString table,QVariantMap data);
    QNetworkReply *SendDeleteQuery(std::function<void(QNetworkReply*)> callback,QString table,QByteArray id);
    QNetworkReply *Download(std::function<void(QNetworkReply*)> callback,QString path);
    bool IsSuccess(QNetworkReply* reply);
    QJsonValue GetJSONValue(QNetworkReply* reply);
    void RemoveRequest(QNetworkReply* reply);
    bool DownloadIsSuccess(QNetworkReply *reply);
private://methods
private slots:
    void OnFinish(QNetworkReply *rep);

signals:
    void Finished(QNetworkReply *);

};

#endif // REMOTESERVER_H
