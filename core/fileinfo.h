#ifndef FILEINFO_H
#define FILEINFO_H

#include <QObject>
#include <QFileInfo>
class FileInfo
{
private://fields
    QByteArray _name ;
    QFileInfo _locale_path , _source_path;
    bool _is_uploaded;
    double _upload_percent;


public:
    explicit FileInfo(QByteArray name,QByteArray locale="",QByteArray source="");
    void SetLocalePath(const QByteArray& ba);
    void SetSourcePath(const QByteArray& ba);
    void SetUploadPercent(double d);
    void SetIsUploaded(bool b);
    QByteArray GetFileName()const;
    const QFileInfo& GetLocaleFileInfo()const;
    const QFileInfo& GetSourceFileInfo()const;
    double GetUploadPercent()const;
    bool IsUploaded()const;

    bool operator==(const FileInfo&)const;
    bool operator==(const QByteArray&)const;
    bool operator!=(const FileInfo&)const;
    bool operator!=(const QByteArray&)const;
    operator QByteArray()const;


signals:

};

#endif // FILEINFO_H
