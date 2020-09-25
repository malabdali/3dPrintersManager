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
    std::function<void(bool)> _callback;
    QByteArray _file_name;
public:
    explicit StartPrinting(Device* _device,std::function<void(bool)> callback,QByteArray fileName);

signals:


    // GCodeCommand interface
public:
    void Start() override;
    void Stop() override;

protected:
    void OnAvailableData(const QByteArray &ba) override;
    void OnDataWritten() override;
    void OnAllDataWritten(bool) override;
    void Finish(bool) override;
};

#endif // STARTPRINTING_H
