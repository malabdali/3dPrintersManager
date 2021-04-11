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
    this->startTimer(REMOTE_SERVER_TIMER);
}

QNetworkReply *RemoteServer::SendRequest(QUrlQuery query, QString url,std::function<void(QNetworkReply*)> callback,uint16_t secondsTimeout)
{
    QNetworkRequest request(QStringLiteral(REMOTE_SERVER_URL)+url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    request.setRawHeader("user",REMOTE_SERVER_ADMIN_NAME);
    request.setRawHeader("pass",REMOTE_SERVER_ADMIN_PASS);
    QNetworkReply* reply= _network->post(request, query.toString().toUtf8());
    std::pair<std::chrono::time_point<std::chrono::steady_clock>,uint16_t> pair=std::make_pair(std::chrono::steady_clock::now(),secondsTimeout);
    _timers.emplace(reply,pair);
    _callbacks.emplace(reply,callback);
    return reply;
}

RemoteServer *RemoteServer::GetInstance()
{
    if(_singleton==nullptr)
        _singleton=new RemoteServer(qApp);
    return _singleton;
}

QNetworkReply *RemoteServer::SendSelectQuery(std::function<void (QNetworkReply *)> callback, QString table, QString where,uint16_t secondsTimeout)
{
    QUrlQuery query;
    query.addQueryItem("table",table);
    query.addQueryItem("search",where);
    return SendRequest(query,"DB/Find",callback,secondsTimeout);
}

QNetworkReply *RemoteServer::SendUpdateQuery(std::function<void (QNetworkReply *)> callback, QString table, QVariantMap data, QByteArray id,uint16_t secondsTimeout)
{
    QUrlQuery query;
    query.addQueryItem("table",table);
    query.addQueryItem("search",QString("{\"_id\":\"objectid(%1)\"}").arg(QString(id)));
    query.addQueryItem("data",QJsonDocument(QJsonObject::fromVariantMap(data)).toJson());
    return SendRequest(query,"DB/Update",callback,secondsTimeout);
}

QNetworkReply *RemoteServer::SendInsertQuery(std::function<void (QNetworkReply *)> callback, QString table, QVariantMap data,uint16_t secondsTimeout)
{
    QUrlQuery query;
    query.addQueryItem("table",table);
    query.addQueryItem("data",QJsonDocument(QJsonObject::fromVariantMap(data)).toJson());
    return SendRequest(query,"DB/Insert",callback,secondsTimeout);
}

QNetworkReply *RemoteServer::SendDeleteQuery(std::function<void (QNetworkReply *)> callback, QString table, QByteArray id,uint16_t secondsTimeout)
{
    QUrlQuery query;
    query.addQueryItem("table",table);
    query.addQueryItem("search",QString("{\"_id\":\"objectid(%1)\"}").arg(QString(id)));
    query.addQueryItem("admin_pass",REMOTE_SERVER_ADMIN_PASS);
    query.addQueryItem("admin_name",REMOTE_SERVER_ADMIN_NAME);
    return SendRequest(query,"DB/Delete",callback,secondsTimeout);
}

QNetworkReply *RemoteServer::Download(std::function<void (QNetworkReply *)> callback, QString file)
{
    QNetworkRequest request(QStringLiteral(REMOTE_SERVER_URL)+"Files/Download/"+file);
    request.setRawHeader("user",REMOTE_SERVER_ADMIN_NAME);
    request.setRawHeader("pass",REMOTE_SERVER_ADMIN_PASS);
    QNetworkReply* reply= _network->get(request);
    _callbacks.emplace(reply,callback);
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

    _callbacks.emplace(reply,callback);
    return reply;
}


bool RemoteServer::IsSuccess(QNetworkReply *reply)
{
    if(!reply->isFinished())
        return false;
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

void RemoteServer::timerEvent(QTimerEvent *te)
{
    Q_UNUSED(te);
    auto timers=_timers;
    for(auto[k,v]:timers){
        if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now()-v.first).count()>=v.second)
        {
            //this->_callbacks[k](k);
            k->close();
            k->abort();
            //_callbacks.erase(k);
            //_timers.erase(k);
        }
    }
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
        _callbacks.erase(reply);
        _timers.erase(reply);
        reply->close();
        reply->abort();
        reply->deleteLater();
    }
}

void RemoteServer::OnFinish(QNetworkReply *rep)
{
    if(_callbacks.find(rep)!=_callbacks.end())
    {
        _callbacks[rep](rep);
        _callbacks.erase(rep);
        _timers.erase(rep);
        emit Finished(rep);
    }
}



