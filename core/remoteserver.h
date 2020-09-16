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
    QNetworkReply* LoadAllDevicesInfo(std::function<void(QNetworkReply*)>);
    QNetworkReply* SelectQuery(std::function<void(QNetworkReply*)>);
    bool IsSuccess(QNetworkReply* reply);
    QJsonObject GetJSONObject(QNetworkReply* reply);

private://methods
    QNetworkReply* SendRequest(QUrlQuery,QUrl,std::function<void(QNetworkReply*)>);
private slots:
    void OnFinish(QNetworkReply *rep);

signals:

};

#endif // REMOTESERVER_H
