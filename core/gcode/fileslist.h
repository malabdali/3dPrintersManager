#ifndef FILESLIST_H
#define FILESLIST_H

#include <QObject>
#include "../gcodecommand.h"
#include <functional>

namespace GCode {
    class FilesList;
}
class GCode::FilesList : public GCodeCommand
{
    Q_OBJECT
public:
    explicit FilesList(Device* _device,std::function<void(bool,QByteArrayList,QList<quint32>)> callback);

private://fields
    std::function<void(bool,QByteArrayList,QList<uint32_t>)> _callback;
    bool _is_begin,_files_list_end;
    QByteArrayList _result_files;
    QList<quint32> _sizes;
    // GCodeCommand interface
public:
    void Start();
    void Stop();

protected:
    void OnAvailableData(const QByteArray &ba);
    void OnDataWritten();
    void OnAllDataWritten(bool);
    void Finish(bool);
};

#endif // FILESLIST_H
