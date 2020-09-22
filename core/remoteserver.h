#ifndef REMOTESERVER_H
#define REMOTESERVER_H

#include <QObject>
#include <QApplication>
#include <QNetworkAccessManager>
#include "device.h"
#include <functional>
#include "../config.cpp"
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include<QJsonObject>

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
    QNetworkReply* SendSelectQuery(std::function<void(QNetworkReply*)> callback,QString table,QString fileds="*",QString where="1=1",QString orderColumn="id",QString orderBy="ASC");
    QNetworkReply *SendUpdateQuery(std::function<void(QNetworkReply*)> callback,QString table,QVariantMap data,int id);
    QNetworkReply* SendInsertQuery(std::function<void(QNetworkReply*)> callback,QString table,QVariantMap data);
    QNetworkReply *SendDeleteQuery(std::function<void(QNetworkReply*)> callback,QString table,int id);
    bool IsSuccess(QNetworkReply* reply);
    QJsonValue GetJSONValue(QNetworkReply* reply);

private://methods
    QNetworkReply* SendRequest(QUrlQuery,QUrl,std::function<void(QNetworkReply*)>);
private slots:
    void OnFinish(QNetworkReply *rep);

signals:
    void Finished(QNetworkReply *);

};

#endif // REMOTESERVER_H
