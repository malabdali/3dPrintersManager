#ifndef STARTPRINTING_H
#define STARTPRINTING_H

#include "../gcodecommand.h"
#include <functional>

namespace GCode {
    class StartPrinting;
}

class GCode::StartPrinting : public GCodeCommand
{
    Q_OBJECT
private:
    QByteArray _file_name;
    bool _m24_sent,_file_selected;
public:
    explicit StartPrinting(Device* _device,QByteArray fileName);

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

#endif // STARTPRINTING_H
