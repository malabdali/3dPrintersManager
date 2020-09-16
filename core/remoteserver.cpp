#include "remoteserver.h"
RemoteServer* RemoteServer::_singleton=nullptr;
RemoteServer::RemoteServer(QObject *parent) : QObject(parent)
{

    _singleton=this;
    _network=new QNetworkAccessManager(this);
    connect(_network,SIGNAL(finished(QNetworkReply*)),this,SLOT(OnFinish(QNetworkReply*)));
}

QNetworkReply *RemoteServer::SendRequest(QUrlQuery query, QUrl url,std::function<void(QNetworkReply*)> callback)
{
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QNetworkReply* reply= _network->post(request, query.toString().toUtf8());
    _callbacks.insert(reply,callback);
    return reply;
}

void RemoteServer::OnFinish(QNetworkReply *rep)
{
    _callbacks[rep](rep);
    _callbacks.remove(rep);
}

RemoteServer *RemoteServer::GetInstance()
{
    if(_singleton==nullptr)
        _singleton=new RemoteServer(qApp);
    return _singleton;
}



QNetworkReply* RemoteServer::LoadAllDevicesInfo(std::function<void(QNetworkReply*)> callback)
{
    QUrlQuery query;
    query.addQueryItem("table","Printers");
    query.addQueryItem("where","1=1");
    query.addQueryItem("fields","*");
    query.addQueryItem("orderColumn","id");
    query.addQueryItem("orderBy","ASC");
    query.addQueryItem("admin_pass","mM41634163Mm");
    query.addQueryItem("admin_name","malabdali");
    return SendRequest(query,QStringLiteral(REMOTE_SERVER_URL)+"Query/Select",callback);

}

bool RemoteServer::IsSuccess(QNetworkReply *reply)
{
    if(reply->error()!=QNetworkReply::NoError)
        return false;
    QString data=reply->readAll();
    data=data.simplified();
    data.replace("\\\"","\"");
    QString res=QJsonDocument::fromJson(data.toUtf8()).object()["result"].toString();
    if(res=="ok")
        return true;
    //qDebug()<<data;
    return false;
}

QJsonObject RemoteServer::GetJSONObject(QNetworkReply *reply)
{
    QJsonDocument document;
    if(IsSuccess(reply)){

        QString data=reply->readAll();
        data=data.simplified();
        data.replace("\\\"","\"");
        return QJsonDocument::fromJson(data.toUtf8()).object()["result"].toObject();
    }
    return QJsonObject();
}



