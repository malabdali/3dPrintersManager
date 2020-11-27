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
    explicit FilesList(Device* _device);

private://fields
    bool _is_begin,_files_list_end;
    QByteArrayList _result_files;
    QList<quint32> _sizes;
    // GCodeCommand interface
public:
    QMap<QByteArray,size_t> GetFilesList();

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};

#endif // FILESLIST_H
