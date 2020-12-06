#include "deviceproblemsolver.h"
#include "device.h"
#include "deviceport.h"
#include "gcodecommand.h"
#include <QVariantHash>
DeviceProblemSolver::DeviceProblemSolver(Device *device):DeviceComponent(device)
{
    _last_command_error=GCodeCommand::NoError;
    _last_device_error=Device::Errors::NoError;

}

DeviceProblemSolver::SolvingType DeviceProblemSolver::GetSolvingType()const
{
    if(_last_command_error!=GCodeCommand::NoError){
        return SolvingType::GCode;
    }
    else if(_last_device_error!=Device::Errors::NoError)
    {
        return SolvingType::OpenPort;
    }
}

void DeviceProblemSolver::Setup()
{
    QObject::connect(_device,&Device::CommandFinished,this,&DeviceProblemSolver::WhenCommandFinished);
    QObject::connect(_device,&Device::CommandStarted,this,&DeviceProblemSolver::WhenCommandStarted);
    QObject::connect(_device,&Device::PortClosed,this,&DeviceProblemSolver::WhenPortClosed);
    QObject::connect(_device,&Device::PortOpened,this,&DeviceProblemSolver::WhenPortOpened);
    QObject::connect(_device,&Device::DeviceStatsUpdateFailed,this,&DeviceProblemSolver::WhenStatsUpdateFailed);
    QObject::connect(_device,&Device::ErrorOccurred,this,&DeviceProblemSolver::WhenErrorOccured);
    QObject::connect(_device,&Device::BeforeSaveDeviceData,this,&DeviceProblemSolver::Save);
}

bool DeviceProblemSolver::IsThereProblem(){
    return _last_command_error!=GCodeCommand::NoError || _last_device_error!=Device::Errors::NoError;

}

QJsonDocument DeviceProblemSolver::ToJson()
{
    QVariantHash vh;
    if(GetSolvingType()==SolvingType::GCode)
        vh.insert("ERROR","GCode");
    else if(GetSolvingType()==SolvingType::OpenPort)
        vh.insert("ERROR","OpenPort");

    return QJsonDocument(QJsonObject::fromVariantHash(vh));

}

void DeviceProblemSolver::FromJson(QJsonDocument *json)
{

}

void DeviceProblemSolver::WhenLinesAvailable(QList<QByteArray> lines)
{
    while(_device->GetDevicePort()->IsThereAvailableLines()){
        QByteArray data=_device->GetDevicePort()->ReadLine();
        if(data.contains("ok"))
        {
            if(_commands_was_played)
                _device->PlayCommands();
            WhenProblemSolved();
            break;
        }
    }
}

void DeviceProblemSolver::WhenErrorOccured(int error)
{
    _last_command_error=GCodeCommand::NoError;
    _last_device_error=Device::Errors::NoError;
    _last_device_error=error;
    emit ProblemDetected();
}

void DeviceProblemSolver::WhenPortClosed()
{
}

void DeviceProblemSolver::WhenPortOpened()
{
    WhenProblemSolved();
}

void DeviceProblemSolver::WhenCommandStarted(GCodeCommand *command)
{
}

void DeviceProblemSolver::WhenCommandFinished(GCodeCommand *command, bool success)
{
    if(!success)
        CheckCommandError(command);
}

void DeviceProblemSolver::WhenStatsUpdateFailed(GCodeCommand *command)
{
    CheckCommandError(command);
}

void DeviceProblemSolver::SolveProblem()
{
    if(_last_command_error!=GCodeCommand::NoError){
        switch (_last_command_error) {
        case GCodeCommand::NoChecksum:
            SolveNoChecksumProblem();
            break;
        }
    }
    else if(_last_device_error!=Device::Errors::NoError)
    {
        _device->GetDevicePort()->Reconnect();
    }
}

void DeviceProblemSolver::CheckCommandError(GCodeCommand *command)
{
    if(!command->IsSuccess()){
        if(command->GetError()!=GCodeCommand::NoError && command->GetError()!=GCodeCommand::PortClosed && command->GetError()!=GCodeCommand::PortError)
        {
            _last_command_error=GCodeCommand::NoError;
            _last_device_error=Device::Errors::NoError;
            _last_command_error=command->GetError();
            emit ProblemDetected();
        }
    }
}

void DeviceProblemSolver::SolveNoChecksumProblem()
{
    _commands_was_played=_device->CommandsIsPlayed();
    _device->PauseCommands();
    QObject::connect(_device->GetDevicePort(),&DevicePort::NewLinesAvailable,this,&DeviceProblemSolver::WhenLinesAvailable);
    _device->GetDevicePort()->Write("M29 \n");
}

void DeviceProblemSolver::WhenProblemSolved()
{
    _last_command_error=GCodeCommand::NoError;
    _last_device_error=Device::Errors::NoError;
    QObject::disconnect(_device->GetDevicePort(),&DevicePort::NewLinesAvailable,this,&DeviceProblemSolver::WhenLinesAvailable);
    emit SolveFinished();
}

void DeviceProblemSolver::Save()
{
    _device->AddData("Errors",ToJson().object());
}

void DeviceProblemSolver::Load()
{

}



