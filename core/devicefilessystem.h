#ifndef DEVICEFILESSYSTEM_H
#define DEVICEFILESSYSTEM_H

#include <QObject>
#include <QMutex>
#include <QMap>

class DeviceFilesSystem : public QObject
{
    Q_OBJECT
    friend class Device;
private:
    class Device* _device;
    QMap<QByteArray,size_t> _files;
    QMutex _mutex;
    bool _sd_supported;
public:
    explicit DeviceFilesSystem(class Device*,QObject *parent = nullptr);
    bool SdSupported()const;
    void UpdateFileList();
    void DeleteFile(const QByteArray& file);
    void UploadFile(QByteArray fileName,const QByteArray& data);
    bool IsStillUploading();

    QMap<QByteArray,size_t> GetFileList();

signals:
    void FileDeleted(QByteArray);
    void FileUploaded(QByteArray);
    void UploadFileFailed();
    void FileListUpdated();
private slots:

private: //methods
    void SetSdSupported(bool b);

};

#endif // DEVICEFILESSYSTEM_H
