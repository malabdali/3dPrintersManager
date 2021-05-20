#ifndef CHITURESUMEPAUSEPRINT_H
#define CHITURESUMEPAUSEPRINT_H

#include "../../chitugcodecommand.h"
namespace GCode{
namespace Chitu{
class ChituResumePausePrint;
}
}

class GCode::Chitu::ChituResumePausePrint:public ChituGcodeCommand
{
    Q_OBJECT
private:
    bool _cannot_resum_print;
    bool _resume_print_command;
public:
    explicit ChituResumePausePrint(Device* _device,bool resume);
    bool IsResumCommand()const;

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

#endif // CHITURESUMEPAUSEPRINT_H
