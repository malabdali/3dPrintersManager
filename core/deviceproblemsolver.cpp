#include "deviceproblemsolver.h"
#include "device.h"
#include "deviceport.h"
#include "deviceconnection.h"
#include "gcodecommand.h"
#include <QVariantHash>
DeviceProblemSolver::DeviceProblemSolver(Device *device):DeviceComponent(device)
{
    _last_command_error=GCodeCommand::NoError;
    _last_device_error=-1;

}

DeviceProblemSolver::SolvingType DeviceProblemSolver::GetSolvingType()const
{
    if(_last_command_error!=GCodeCommand::NoError){
        return SolvingType::GCode;
    }
    else if(_last_device_error!=-1)
    {
        return SolvingType::OpenPort;
    }
    else
        return SolvingType::NON;
}

void DeviceProblemSolver::Setup()
{
    QObject::connect(_device,&Device::CommandFinished,this,&DeviceProblemSolver::WhenCommandFinished);
    QObject::connect(_device,&Device::CommandStarted,this,&DeviceProblemSolver::WhenCommandStarted);
    QObject::connect(_device,&Device::Closed,this,&DeviceProblemSolver::WhenPortClosed);
    QObject::connect(_device,&Device::Opened,this,&DeviceProblemSolver::WhenPortOpened);
    QObject::connect(_device,&Device::DeviceStatsUpdateFailed,this,&DeviceProblemSolver::WhenStatsUpdateFailed);
    QObject::connect(_device,&Device::ErrorOccurred,this,&DeviceProblemSolver::WhenErrorOccured);
    QObject::connect(_device,&Device::BeforeSaveDeviceData,this,&DeviceProblemSolver::Save);
}

bool DeviceProblemSolver::IsThereProblem(){
    return _last_command_error!=GCodeCommand::NoError || _last_device_error!=-1;

}

QJsonDocument DeviceProblemSolver::ToJson()
{
    QVariantHash vh;
    vh.insert("ERROR",ErrorToText());
    vh.insert("TIME",_last_error_time.toString(Qt::DateFormat::ISODateWithMs));
    return QJsonDocument(QJsonObject::fromVariantHash(vh));

}

void DeviceProblemSolver::WhenLinesAvailable(QList<QByteArray> lines)
{
    while(_device->GetDeviceConnection()->IsThereAvailableLines()){
        QByteArray data=_device->GetDeviceConnection()->ReadLine();
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
    if(error!=_last_device_error)
    {
        _last_error_time=QDateTime::currentDateTime();
    }
    _last_command_error=GCodeCommand::NoError;
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
    else{
        _last_command_error=GCodeCommand::NoError;
    }
}

void DeviceProblemSolver::WhenStatsUpdateFailed(GCodeCommand *command)
{
    CheckCommandError(command);
}

QByteArray DeviceProblemSolver::ErrorToText()const
{
    QByteArray error="";
    if(_last_command_error!=GCodeCommand::NoError){
        switch (_last_command_error) {
        case GCodeCommand::Busy:
            error="device is busy";
            break;

        case GCodeCommand::CommandError::NoChecksum:
            error="check sum error";
            break;

        case GCodeCommand::CommandError::TimeOut:
            error="time out error";
            break;

        case GCodeCommand::CommandError::UnknownError:
            error="unknown error";
            break;

        case GCodeCommand::CommandError::WriteError:
            error="write error";
            break;

        default:
            error="";
            break;

        }
    }
    else if(_last_device_error!=-1){
        error=_device->GetDeviceConnection()->GetErrorText();
    }
    return error;
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
    else if(_last_device_error!=-1)
    {
        _device->GetDeviceConnection()->Open();
    }
}

void DeviceProblemSolver::CheckCommandError(GCodeCommand *command)
{
    if(!command->IsSuccess()){
        if(command->GetError()!=GCodeCommand::NoError && command->GetError()!=GCodeCommand::PortClosed && command->GetError()!=GCodeCommand::PortError)
        {

            if(command->GetError()!=_last_command_error)
            {
                _last_error_time=QDateTime::currentDateTime();
            }
            _last_command_error=GCodeCommand::NoError;
            _last_device_error=-1;
            _last_command_error=command->GetError();
            emit ProblemDetected();
        }
    }
}

void DeviceProblemSolver::SolveNoChecksumProblem()
{
    _commands_was_played=_device->CommandsIsPlayed();
    _device->PauseCommands();
    QObject::connect(_device->GetDeviceConnection(),&DevicePort::NewLinesAvailable,this,&DeviceProblemSolver::WhenLinesAvailable);
    _device->GetDeviceConnection()->Write("M29 \n");
}

void DeviceProblemSolver::WhenProblemSolved()
{
    _last_command_error=GCodeCommand::NoError;
    _last_device_error=-1;
    QObject::disconnect(_device->GetDeviceConnection(),&DevicePort::NewLinesAvailable,this,&DeviceProblemSolver::WhenLinesAvailable);
    emit SolveFinished();
}

void DeviceProblemSolver::FromJson(QJsonDocument json)
{

}

void DeviceProblemSolver::Save()
{
    _device->AddData("Errors",ToJson().object());
}

void DeviceProblemSolver::Load()
{

}





void DeviceProblemSolver::Disable()
{
    QObject::disconnect(_device,&Device::CommandFinished,this,&DeviceProblemSolver::WhenCommandFinished);
    QObject::disconnect(_device,&Device::CommandStarted,this,&DeviceProblemSolver::WhenCommandStarted);
    QObject::disconnect(_device,&Device::Closed,this,&DeviceProblemSolver::WhenPortClosed);
    QObject::disconnect(_device,&Device::Opened,this,&DeviceProblemSolver::WhenPortOpened);
    QObject::disconnect(_device,&Device::DeviceStatsUpdateFailed,this,&DeviceProblemSolver::WhenStatsUpdateFailed);
    QObject::disconnect(_device,&Device::ErrorOccurred,this,&DeviceProblemSolver::WhenErrorOccured);
    QObject::disconnect(_device,&Device::BeforeSaveDeviceData,this,&DeviceProblemSolver::Save);
}
