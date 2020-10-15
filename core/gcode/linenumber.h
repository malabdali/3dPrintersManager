#ifndef LINENUMBER_H
#define LINENUMBER_H

#include <QObject>
#include "../gcodecommand.h"
#include <functional>

namespace GCode {
    class LineNumber;
}
class GCode::LineNumber : public GCodeCommand
{
    Q_OBJECT
public:
    explicit LineNumber(Device* _device);

private://fields
    quint64 _line_number;
    bool _29_sent,_is_open,_is_fail;

    // GCodeCommand interface
public:
    quint64 GetLineNumber();


protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten() override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};


#endif // LINENUMBER_H
