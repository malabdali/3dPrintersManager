#ifndef CHITUFILESLIST_H
#define CHITUFILESLIST_H

#include <QObject>
#include "../../chitugcodecommand.h"
#include <functional>

namespace GCode {
namespace Chitu {
    class ChituFilesList;
}
}
class GCode::Chitu::ChituFilesList : public ChituGcodeCommand
{
    Q_OBJECT
public:
    explicit ChituFilesList(Device* _device);

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

#endif // CHITUFILESLIST_H
