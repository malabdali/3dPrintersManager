#ifndef PRINTCONTROLLER_H
#define PRINTCONTROLLER_H

#include "./devicecomponent.h"
#include <QDebug>
class PrintController:public DeviceComponent
{
    Q_OBJECT
public://types
    enum Status:int{
        Nothing=0,
        SendHeatUpNozzleCommands=1,
        HeatUpNozzle=2,
        SendHeatUpBedCommands=3,
        HeatUpBed=4,
        SendPrintCommand=5,
        Printing=6,
        SendStopCommand=7,
        SendHeatOffCommand=8,
        Stopped=9,
        PreprintPrepare=10,
        Paused=11,
        Resuming=12
    };

protected://fields
    Status _current_status,_wanted_status;

public:
    explicit PrintController(class Device*);
    virtual void StartPrint(QByteArray file)=0;
    virtual void ContinuePrint()=0;
    virtual void StopPrint()=0;
    virtual void PausePrint()=0;
    virtual bool CanContinuePrinting()=0;
    virtual bool IsPrinting();
    virtual Status GetWantedStatus();
    virtual Status GetCurrentStatus();
    virtual void SetWantedStatus(Status status);
    void Save()override;
    void Load()override;
public:
    void Setup() override;
signals:
    void PrintStarted();
    void PrintStopped();
    void WantedStatusChanged();
    void StatusChanged();
    void PrintingFinished();




    // DeviceComponent interface
public:
    void Disable() override;
};

#endif // PRINTCONTROLLER_H
