#ifndef DEVICEFUNCTIONS_H
#define DEVICEFUNCTIONS_H

#include <QObject>
#include <QByteArray>
#include <QThread>
#include <functional>
class DeviceFunctions : public QObject
{
    Q_OBJECT
public: //nested types
    enum class Function{
        FileList,
        UploadFile,
        DeleteFile
    };
private:
    class Device* _device;
    Function _function;
    const QByteArray _data1,_data2;
    QByteArray _result;
    bool _finished;
    QByteArrayList _outputs;
    bool _started;
    uint32_t _counter;
    bool _resend,_upload_stage;
    std::function<void(bool,QVariant)> _callback;
public:
    explicit DeviceFunctions(Device* device,Function fun,std::function<void (bool, QVariant)>,QByteArray data1="",QByteArray data2="");
    DeviceFunctions& operator=(DeviceFunctions&)=delete;
    DeviceFunctions& operator=(DeviceFunctions&&)=delete;
    void Start();
    bool IsFinished();
    void Stop();
    bool IsStarted()const;
    ~DeviceFunctions();

private:
    //files List
    void FileListFunctionDataAvailable(QByteArray);
    void FileListFunctionWriteLine();
    void FileListFunctionFinishWrite(bool);
    void FileListFunctionStop();
    void FileListFunctionFinished(bool success);
    //end files List

    //upload file
    void UploadFileFunctionDataAvailable(QByteArray);
    void UploadFileFunctionWriteLine();
    void UploadFileFunctionFinishWrite(bool);
    void UploadFileFunctionStop();
    void UploadFileFunctionFinished(bool success);
    //end upload file

    //deleteFile
    void DeleleFileFunctionDataAvailable(QByteArray);
    void DeleteFileFunctionFinished(bool success);
    void DeleteFileFunctionStop();
    //end deleteFile

    void Finish(bool);

private slots:
    void WhenLineAvailable(QByteArrayList);
    void WhenWriteFinished(bool b);
    void WhenWriteLine();
signals:
    void Finished(bool);

};

#endif // DEVICEFUNCTIONS_H
