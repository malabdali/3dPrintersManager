#ifndef SLAFILESSYSTEM_H
#define SLAFILESSYSTEM_H

#include "../devicefilessystem.h"
#include "../gcode/CHITU/chitufileslist.h"
#include "../gcode/CHITU/chitudeletefile.h"
#include "../gcode/CHITU/chituuploadfile.h"
class SLAFilesSystem:public DeviceFilesSystem
{
private://fields
    GCode::Chitu::ChituUploadFile* _uploading_file;
public:
    SLAFilesSystem(class Device*);
    void Setup() override;
    void UpdateFileList()override;
    void DeleteFile(const QByteArray& file)override;
    void UploadFile(QByteArray fileName)override;
    void Disable() override;

private:
    void UpdateLineNumber()override;
    void UploadNext();

protected slots:
    void WhenDeviceStatusChanged(Device::DeviceStatus b);
    void WhenPortClosed();
    void WhenCommandFinished(GCodeCommand* command,bool);
    void WhenStatsUpdated();
    void WhenFileUploaded(GCode::Chitu::ChituUploadFile*);
    void WhenFileListUpdated(GCode::Chitu::ChituFilesList*);

protected:

    // DeviceFilesSystem interface
public:
    bool SdSupported() const override;

    // DeviceFilesSystem interface
public:
    bool IsStillUploading() override;
    double GetUploadProgress() override;
    void StopUpload(QByteArray ba) override;
};

#endif // SLAFILESSYSTEM_H
