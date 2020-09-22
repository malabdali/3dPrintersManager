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

RemoteServer *RemoteServer::GetInstance()
{
    if(_singleton==nullptr)
        _singleton=new RemoteServer(qApp);
    return _singleton;
}

QNetworkReply *RemoteServer::SendSelectQuery(std::function<void (QNetworkReply *)> callback, QString table, QString fields, QString where, QString orderColumn, QString orderBy)
{
    QUrlQuery query;
    query.addQueryItem("table",table);
    query.addQueryItem("where",where);
    query.addQueryItem("fields",fields);
    query.addQueryItem("orderColumn",orderColumn);
    query.addQueryItem("orderBy",orderBy);
    query.addQueryItem("admin_pass",REMOTE_SERVER_ADMIN_PASS);
    query.addQueryItem("admin_name",REMOTE_SERVER_ADMIN_NAME);
    return SendRequest(query,QStringLiteral(REMOTE_SERVER_URL)+"Query/Select",callback);
}

QNetworkReply *RemoteServer::SendUpdateQuery(std::function<void (QNetworkReply *)> callback, QString table, QVariantMap data, int id)
{
    QUrlQuery query;
    query.addQueryItem("table",table);
    query.addQueryItem("id",QString::number(id));
    query.addQueryItem("data",QJsonDocument(QJsonObject::fromVariantMap(data)).toJson());
    query.addQueryItem("admin_pass",REMOTE_SERVER_ADMIN_PASS);
    query.addQueryItem("admin_name",REMOTE_SERVER_ADMIN_NAME);
    return SendRequest(query,QStringLiteral(REMOTE_SERVER_URL)+"Query/Update",callback);
}

QNetworkReply *RemoteServer::SendInsertQuery(std::function<void (QNetworkReply *)> callback, QString table, QVariantMap data)
{
    QUrlQuery query;
    query.addQueryItem("table",table);
    query.addQueryItem("data",QJsonDocument(QJsonObject::fromVariantMap(data)).toJson());
    query.addQueryItem("admin_pass",REMOTE_SERVER_ADMIN_PASS);
    query.addQueryItem("admin_name",REMOTE_SERVER_ADMIN_NAME);
    return SendRequest(query,QStringLiteral(REMOTE_SERVER_URL)+"Query/Insert",callback);
}

QNetworkReply *RemoteServer::SendDeleteQuery(std::function<void (QNetworkReply *)> callback, QString table, int id)
{
    QUrlQuery query;
    query.addQueryItem("table",table);
    query.addQueryItem("id",QString::number(id));
    query.addQueryItem("admin_pass",REMOTE_SERVER_ADMIN_PASS);
    query.addQueryItem("admin_name",REMOTE_SERVER_ADMIN_NAME);
    return SendRequest(query,QStringLiteral(REMOTE_SERVER_URL)+"Query/Delete",callback);
}

bool RemoteServer::IsSuccess(QNetworkReply *reply)
{
    if(reply->error()!=QNetworkReply::NoError)
        return false;
    QString data=reply->peek(reply->size());
    data=data.simplified();
    data.replace("\\\"","\"");
    QString res=QJsonDocument::fromJson(data.toUtf8()).object()["result"].toString();
    if(res=="ok")
        return true;
    //qDebug()<<data;
    return false;
}

QJsonValue RemoteServer::GetJSONValue(QNetworkReply *reply)
{
    QJsonDocument document;
    if(IsSuccess(reply)){

        QString data=reply->peek(reply->size());
        data=data.simplified();
        data.replace("\\\"","\"");
        return QJsonDocument::fromJson(data.toUtf8()).object()["data"];
    }
    return QJsonObject();
}

void RemoteServer::OnFinish(QNetworkReply *rep)
{
    _callbacks[rep](rep);
    _callbacks.remove(rep);
    emit Finished(rep);
}



