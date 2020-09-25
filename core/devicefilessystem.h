#ifndef DEVICEFILESSYSTEM_H
#define DEVICEFILESSYSTEM_H

#include <QObject>
#include <QMutex>
#include <QMap>
#include <QMetaMethod>
#include "gcode/fileslist.h"
#include "gcode/uploadfile.h"
#include "gcode/deletefile.h"

class DeviceFilesSystem : public QObject
{
    Q_OBJECT
    friend class Device;
private:
    class Device* _device;
    QMap<QByteArray,size_t> _files;
    QList<GCode::UploadFile*> _uploading_files;
    QList<QByteArray> _uploaded_files,_failed_uploads;
    QMutex _mutex;
    bool _sd_supported;
public:
    explicit DeviceFilesSystem(class Device*,QObject *parent = nullptr);
    bool SdSupported()const;
    void UpdateFileList();
    void DeleteFile(const QByteArray& file);
    void UploadFile(QByteArray fileName,const QByteArray& data);
    bool IsStillUploading();
    double GetUploadProgress();
    QList<QByteArray> GetUploadedFiles();
    QList<QByteArray> GetFailedUploads();
    QMap<QByteArray,size_t> GetFileList();
    void StopUpload(QByteArray ba);
    QList<QByteArray> GetWaitUploadingList()const;

signals:
    void FileDeleted(QByteArray);
    void FileUploaded(QByteArray);
    void UploadFileFailed(QByteArray);
    void FileListUpdated();
    void WaitListUpdated();
private slots:
    void WhenDeviceReady(bool b);

private: //methods
    void SetSdSupported(bool b);
    void CallFunction(const char* function);

};

#endif // DEVICEFILESSYSTEM_H
