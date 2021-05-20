#ifndef CHITUSTARTPRINT_H
#define CHITUSTARTPRINT_H

#include "../../chitugcodecommand.h"

namespace GCode{
namespace Chitu{
class ChituStartStopPrint;
}
}

class GCode::Chitu::ChituStartStopPrint:public ChituGcodeCommand
{
    Q_OBJECT
private:
    QByteArray _file_name;
    bool _cannot_start_print;
    bool _start_print_command;
public:
    explicit ChituStartStopPrint(Device* _device,QByteArray fileName,bool start);
    QByteArray GetFileName()const;
    bool IsPrintCommand()const;

signals:


    // GCodeCommand interface
public:

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool success) override;
    void Finish(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};

#endif // CHITUSTARTPRINT_H
