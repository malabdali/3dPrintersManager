#include "fileinfo.h"


FileInfo::FileInfo(QByteArray name, QByteArray locale, QByteArray source)
{
    this->SetLocalePath(locale);
    this->SetSourcePath(source);
    _name=name;
    _upload_percent=0;
    _is_uploaded=false;

}

void FileInfo::SetLocalePath(const QByteArray &ba)
{
    if(!ba.isEmpty())
        _locale_path= QFileInfo (ba);
}

void FileInfo::SetSourcePath(const QByteArray &ba)
{

    if(!ba.isEmpty())
        _source_path= QFileInfo(ba);
}

void FileInfo::SetUploadPercent(double d)
{
    _upload_percent=d;
}

void FileInfo::SetIsUploaded(bool b)
{
    _is_uploaded=b;
}

QByteArray FileInfo::GetFileName() const
{
    return _name;
}

const QFileInfo& FileInfo::GetLocaleFileInfo() const
{
    return _locale_path;
}

const QFileInfo& FileInfo::GetSourceFileInfo() const
{
    return _source_path;
}

double FileInfo::GetUploadPercent() const
{
    return _upload_percent;
}

bool FileInfo::IsUploaded() const
{
    return _is_uploaded;
}

bool FileInfo::operator==(const FileInfo &fi)const
{
    return fi.GetFileName().toLower()==_name.toLower();
}

bool FileInfo::operator==(const QByteArray &name)const
{
    return _name.toLower()==name.toLower();
}

bool FileInfo::operator!=(const FileInfo &fi) const
{
    return fi.GetFileName().toLower()!=_name.toLower();
}

bool FileInfo::operator!=(const QByteArray &name) const
{
    return _name.toLower()!=name.toLower();
}

FileInfo::operator QByteArray()const{
    return _name;
}


