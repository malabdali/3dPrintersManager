#ifndef DEVICEFILESSYSTEM_H
#define DEVICEFILESSYSTEM_H

#include <QObject>
#include <QMutex>
#include <QMap>
#include <QMetaMethod>
#include "gcode/fileslist.h"
#include "gcode/uploadfile.h"
#include "gcode/deletefile.h"
#include "gcode/linenumber.h"
#include "./fileinfo.h"
#include "./device.h"
#include "./devicecomponent.h"
class DeviceFilesSystem : public DeviceComponent
{
    Q_OBJECT
    friend class Device;
protected:
    QList<FileInfo> _files;
    QByteArray _uploading_file_content;
    const QByteArray _extension;
    quint64 _line_number;
    unsigned _upload_speed;
    QList<QByteArray> _failed_uploads,_wait_for_upload;
    bool _sd_supported;
    class LoadPrintableFuture* _load_file;
public:
    explicit DeviceFilesSystem(class Device*,QByteArray extension);
    virtual void Initiate();
    virtual bool SdSupported()const;
    virtual void UpdateFileList();
    virtual void DeleteFile(const QByteArray& file);
    virtual void UploadFile(QByteArray fileName);
    virtual bool IsStillUploading()=0;
    virtual void SetUploadSpeed(unsigned spedd);
    virtual unsigned GetUploadSpeed();
    virtual double GetUploadProgress()=0;
    virtual FileInfo GetUploadedFileInfo(const QByteArray& ba);
    virtual QList<QByteArray> GetFailedUploads();
    virtual QList<FileInfo>& GetFileList();
    virtual void StopUpload(QByteArray ba)=0;
    virtual void Stop();
    virtual QList<QByteArray> GetWaitUploadingList();
    virtual void UpdateLineNumber();
    virtual uint64_t GetLineNumber();
    virtual QByteArray GetLocaleDirectory(const QByteArray& subdir="");
    virtual void SaveLocaleFile(const QString& path,const QByteArray& data,std::function<void(bool)> callback);
    virtual bool DeleteLocaleFile(const QByteArray& path);
    virtual void ReadLocaleFile(const QByteArray& path,std::function<void(QByteArray)> callback);
    virtual void CopyLocaleFile(const QByteArray& fpath,const QByteArray& tpath,std::function<void (bool)> callback);
    virtual QStringList GetLocaleFiles(const QByteArray& path , const QByteArray& suffix);
    virtual QByteArray GetFileExtension();
    void Setup()override;

    QJsonDocument ToJson() override ;
    void FromJson(QJsonDocument json) override;
    void Save()override;
    void Load()override;


signals:
    void FileDeleted(QByteArray);
    void FileUploaded(QByteArray);
    void UploadFileFailed(QByteArray);
    void FileListUpdated();
    void LineNumberUpdated(bool);
    void SdSupportChanged(bool);


protected: //methods
    virtual void SetSdSupported(bool b);
    void CallFunction(const char* function);


    // DeviceComponent interface
public:
    void Disable() override;
};

#endif // DEVICEFILESSYSTEM_H
