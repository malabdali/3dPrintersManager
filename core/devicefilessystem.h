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

class DeviceFilesSystem : public QObject
{
    Q_OBJECT
    friend class Device;
private:
    class Device* _device;
    QMap<QByteArray,size_t> _files;
    GCode::UploadFile* _uploading_file;
    quint64 _line_number;
    QList<QByteArray> _uploaded_files,_failed_uploads,_wait_for_upload;
    bool _sd_supported;
public:
    explicit DeviceFilesSystem(class Device*,QObject *parent = nullptr);
    void Initiate();
    bool SdSupported()const;
    void UpdateFileList();
    void DeleteFile(const QByteArray& file);
    void UploadFile(QByteArray fileName);
    void Print();
    bool IsStillUploading();
    double GetUploadProgress();
    QList<QByteArray> GetUploadedFiles();
    QList<QByteArray> GetFailedUploads();
    QMap<QByteArray,size_t> GetFileList(bool=true);
    void StopUpload(QByteArray ba);
    QList<QByteArray> GetWaitUploadingList();
    void UpdateLineNumber();
    uint64_t GetLineNumber();
    QByteArray GetLocaleDirectory();
    void SaveLocaleFile(const QString& path,const QByteArray& data,std::function<void(bool)> callback);
    bool DeleteLocaleFile(const QByteArray& path);
    void ReadLocaleFile(const QByteArray& path,std::function<void(QByteArray)> callback);
    void CopyLocaleFile(const QByteArray& fpath,const QByteArray& tpath,std::function<void (bool)> callback);
    QStringList GetLocaleFiles(const QByteArray& path , const QByteArray& suffix);


signals:
    void FileDeleted(QByteArray);
    void FileUploaded(QByteArray);
    void UploadFileFailed(QByteArray);
    void FileListUpdated();
    void LineNumberUpdated(bool);

private slots:
    void WhenDeviceReady(bool b);
    void WhenPortClosed();
    void WhenCommandFinished(GCodeCommand* command,bool);
    void WhenStatsUpdated();
    void WhenLineNumberUpdated(GCode::LineNumber*);
    void WhenFileUploaded(GCode::UploadFile*);

private: //methods
    void SetSdSupported(bool b);
    void CallFunction(const char* function);

};

#endif // DEVICEFILESSYSTEM_H
