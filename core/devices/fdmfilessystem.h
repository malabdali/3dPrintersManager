#ifndef FDMFILESSYSTEM_H
#define FDMFILESSYSTEM_H

#include "../devicefilessystem.h"
class FDMFilesSystem:public DeviceFilesSystem
{

private://fields
    GCode::UploadFile* _uploading_file;
public:
    FDMFilesSystem(class Device*);
    void Setup() override;
    void UpdateFileList()override;
    void DeleteFile(const QByteArray& file)override;
    void UploadFile(QByteArray fileName)override;
    void UpdateLineNumber()override;
    void Disable() override;

protected slots:
    void WhenDeviceStatusChanged(Device::DeviceStatus b);
    void WhenPortClosed();
    void WhenCommandFinished(GCodeCommand* command,bool);
    void WhenStatsUpdated();
    void WhenLineNumberUpdated(GCode::LineNumber*);
    void WhenFileUploaded(GCode::UploadFile*);
    void WhenFileListUpdated(GCode::FilesList*);

protected:

    // DeviceFilesSystem interface
public:
    bool IsStillUploading() override;
    double GetUploadProgress() override;
    void StopUpload(QByteArray ba) override;
};

#endif // FDMFILESSYSTEM_H
