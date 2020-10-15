#include "deviceproblemsolver.h"
#include "device.h"
#include "deviceport.h"
#include "gcodecommand.h"

DeviceProblemSolver::DeviceProblemSolver(Device *device):_device(device)
{
    this->setParent(device);
    QObject::connect(_device,&Device::CommandFinished,this,&DeviceProblemSolver::WhenCommandFinished);
    QObject::connect(_device,&Device::CommandStarted,this,&DeviceProblemSolver::WhenCommandStarted);
    QObject::connect(_device,&Device::PortClosed,this,&DeviceProblemSolver::WhenPortClosed);
    QObject::connect(_device,&Device::PortOpened,this,&DeviceProblemSolver::WhenPortOpened);
    QObject::connect(_device,&Device::DeviceStatsUpdateFailed,this,&DeviceProblemSolver::WhenStatsUpdateFailed);
    QObject::connect(_device,&Device::ErrorOccurred,this,&DeviceProblemSolver::WhenErrorOccured);
    _last_command_error=GCodeCommand::NoError;
    _last_device_error=Device::Errors::NoError;

}

bool DeviceProblemSolver::IsThereProblem(){
    return _last_command_error!=GCodeCommand::NoError || _last_device_error!=Device::Errors::NoError;

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
    _commands_was_played=_device->CommandsIsPlayed();
    _device->PauseCommands();
    if(_last_command_error!=GCodeCommand::NoError){
        _device->CommandsIsPlayed();
        switch (_last_command_error) {
            case GCodeCommand::NoChecksum:
            SolveNoChecksumProblem();
            break;
        }
    }
    else if(_last_device_error!=Device::Errors::NoError)
    {

        switch (_last_device_error) {

        }
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

