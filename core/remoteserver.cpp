#include "remoteserver.h"
#include "device.h"
#include <QHttpMultiPart>
#include <QFile>
RemoteServer* RemoteServer::_singleton=nullptr;
RemoteServer::RemoteServer(QObject *parent) : QObject(parent)
{

    _singleton=this;
    _network=new QNetworkAccessManager(this);
    connect(_network,SIGNAL(finished(QNetworkReply*)),this,SLOT(OnFinish(QNetworkReply*)));
}

QNetworkReply *RemoteServer::SendRequest(QUrlQuery query, QString url,std::function<void(QNetworkReply*)> callback)
{
    QNetworkRequest request(QStringLiteral(REMOTE_SERVER_URL)+url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("user",REMOTE_SERVER_ADMIN_NAME);
    request.setRawHeader("pass",REMOTE_SERVER_ADMIN_PASS);
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

QNetworkReply *RemoteServer::SendSelectQuery(std::function<void (QNetworkReply *)> callback, QString table, QString where)
{
    QUrlQuery query;
    query.addQueryItem("table",table);
    query.addQueryItem("search",where);
    return SendRequest(query,"DB/Find",callback);
}

QNetworkReply *RemoteServer::SendUpdateQuery(std::function<void (QNetworkReply *)> callback, QString table, QVariantMap data, QByteArray id)
{
    QUrlQuery query;
    query.addQueryItem("table",table);
    query.addQueryItem("search",QString("{\"_id\":\"objectid(%1)\"}").arg(QString(id)));
    query.addQueryItem("data",QJsonDocument(QJsonObject::fromVariantMap(data)).toJson());
    return SendRequest(query,"DB/Update",callback);
}

QNetworkReply *RemoteServer::SendInsertQuery(std::function<void (QNetworkReply *)> callback, QString table, QVariantMap data)
{
    QUrlQuery query;
    query.addQueryItem("table",table);
    query.addQueryItem("data",QJsonDocument(QJsonObject::fromVariantMap(data)).toJson());
    return SendRequest(query,"DB/Insert",callback);
}

QNetworkReply *RemoteServer::SendDeleteQuery(std::function<void (QNetworkReply *)> callback, QString table, QByteArray id)
{
    QUrlQuery query;
    query.addQueryItem("table",table);
    query.addQueryItem("search",QString("{\"_id\":\"objectid(%1)\"}").arg(QString(id)));
    query.addQueryItem("admin_pass",REMOTE_SERVER_ADMIN_PASS);
    query.addQueryItem("admin_name",REMOTE_SERVER_ADMIN_NAME);
    return SendRequest(query,"DB/Delete",callback);
}

QNetworkReply *RemoteServer::Download(std::function<void (QNetworkReply *)> callback, QString file)
{
    QNetworkRequest request(QStringLiteral(REMOTE_SERVER_URL)+"Files/Download/"+file);
    request.setRawHeader("user",REMOTE_SERVER_ADMIN_NAME);
    request.setRawHeader("pass",REMOTE_SERVER_ADMIN_PASS);
    QNetworkReply* reply= _network->get(request);
    _callbacks.insert(reply,callback);
    return reply;
}

QNetworkReply *RemoteServer::UploadImage(QString filePath, QString uploadPath, std::function<void (QNetworkReply *)> callback)
{
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart textPart;
    textPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"path\"")));
    textPart.setBody(uploadPath.toUtf8());

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("image/jpg"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(QString("form-data; name=\"imageFile\"; filename=\"%1\"").arg(QUrl(filePath).fileName())));
    QFile *file = new QFile(filePath);
    file->open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(file);
    file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

    multiPart->append(textPart);
    multiPart->append(imagePart);

    QUrl url(QStringLiteral(REMOTE_SERVER_URL)+"Images/Upload");
    QNetworkRequest request(url);

    request.setRawHeader("user",REMOTE_SERVER_ADMIN_NAME);
    request.setRawHeader("pass",REMOTE_SERVER_ADMIN_PASS);
    QNetworkReply *reply = _network->post(request, multiPart);
    multiPart->setParent(reply); // delete the multiPart with the reply

    _callbacks.insert(reply,callback);
    return reply;
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
    return false;
}

bool RemoteServer::DownloadIsSuccess(QNetworkReply *reply)
{
    if(reply->error()!=QNetworkReply::NoError)
        return false;
    QString data=reply->peek(reply->size());
    if(data.startsWith("{")&& data.endsWith("}"))
        return false;
    return true;
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

void RemoteServer::RemoveRequest(QNetworkReply *reply)
{
    if(reply){
        this->_callbacks.remove(reply);
        reply->close();
        reply->abort();
        reply->deleteLater();
    }
}

void RemoteServer::OnFinish(QNetworkReply *rep)
{
    if(_callbacks.contains(rep))
    {
        _callbacks[rep](rep);
        _callbacks.remove(rep);
        emit Finished(rep);
    }
}



