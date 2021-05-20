#ifndef CHITUGCODECOMMAND_H
#define CHITUGCODECOMMAND_H

#include "gcodecommand.h"
class ChituGcodeCommand:public GCodeCommand
{
    Q_OBJECT
public:
    ChituGcodeCommand(Device *device, QByteArray command,uint32_t noResponseTimeout=DEFAULT_Command_No_RESPONSE_TIMEOUT);

    // GCodeCommand interface
protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnAllDataWritten(bool) override;
    void InsideStart() override;
    void InsideStop() override;
};

#endif // CHITUGCODECOMMAND_H
