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
private:
    QList<FileInfo> _files;
    GCode::UploadFile* _uploading_file;
    QByteArray _uploading_file_content;
    quint64 _line_number;
    unsigned _upload_speed;
    QList<QByteArray> _failed_uploads,_wait_for_upload;
    bool _sd_supported;
    class LoadGCodeFuture* _load_file;
public:
    explicit DeviceFilesSystem(class Device*);
    void Initiate();
    bool SdSupported()const;
    void UpdateFileList();
    void DeleteFile(const QByteArray& file);
    void UploadFile(QByteArray fileName);
    void Print();
    bool IsStillUploading();
    void SetUploadSpeed(unsigned spedd);
    unsigned GetUploadSpeed();
    double GetUploadProgress();
    FileInfo GetUploadedFileInfo(const QByteArray& ba);
    QList<QByteArray> GetFailedUploads();
    QList<FileInfo>& GetFileList();
    void StopUpload(QByteArray ba);
    void Stop();
    QList<QByteArray> GetWaitUploadingList();
    void UpdateLineNumber();
    uint64_t GetLineNumber();
    QByteArray GetLocaleDirectory(const QByteArray& subdir="");
    void SaveLocaleFile(const QString& path,const QByteArray& data,std::function<void(bool)> callback);
    bool DeleteLocaleFile(const QByteArray& path);
    void ReadLocaleFile(const QByteArray& path,std::function<void(QByteArray)> callback);
    void CopyLocaleFile(const QByteArray& fpath,const QByteArray& tpath,std::function<void (bool)> callback);
    QStringList GetLocaleFiles(const QByteArray& path , const QByteArray& suffix);
    void Setup()override;

    QJsonDocument ToJson() ;
    void FromJson(QJsonDocument json);
    void Save();
    void Load();


signals:
    void FileDeleted(QByteArray);
    void FileUploaded(QByteArray);
    void UploadFileFailed(QByteArray);
    void FileListUpdated();
    void LineNumberUpdated(bool);
    void SdSupportChanged(bool);

private slots:
    void WhenDeviceStatusChanged(Device::DeviceStatus b);
    void WhenPortClosed();
    void WhenCommandFinished(GCodeCommand* command,bool);
    void WhenStatsUpdated();
    void WhenLineNumberUpdated(GCode::LineNumber*);
    void WhenFileUploaded(GCode::UploadFile*);
    void WhenFileListUpdated(GCode::FilesList*);

private: //methods
    void SetSdSupported(bool b);
    void CallFunction(const char* function);


    // DeviceComponent interface
public:
    void Disable() override;
};

#endif // DEVICEFILESSYSTEM_H
