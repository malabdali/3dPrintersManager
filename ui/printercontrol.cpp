#include "printercontrol.h"
#include "ui_printercontrol.h"
#include "../core/deviceport.h"
#include "../core/deviceinfo.h"

PrinterControl::PrinterControl(Device *dev, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PrinterControl),
    _device(dev)
{
    ui->setupUi(this);
}

PrinterControl::~PrinterControl()
{
    delete ui;
}

void PrinterControl::on__home_button_clicked()
{
    _device->GetDeviceConnection()->Write("G28 \n");
}

void PrinterControl::on__middle_button_clicked()
{
    _device->GetDeviceConnection()->Write("G1 F5000\n");
    _device->GetDeviceConnection()->Write("G1 F1200 Z30\n");
    _device->GetDeviceConnection()->Write((QString("G1 F5000 X%0 Y%1 \n").arg(_device->GetDeviceInfo()->GetX()/2).arg(_device->GetDeviceInfo()->GetY()/2)).toUtf8());
    _device->GetDeviceConnection()->Write("G1 F1200 Z0 \n");
}

void PrinterControl::on__top_left_button_clicked()
{
    _device->GetDeviceConnection()->Write("G1 F5000\n");
    _device->GetDeviceConnection()->Write("G1 F1200 Z30\n");
    _device->GetDeviceConnection()->Write((QString("G1 F5000 X%0 Y%1 \n").arg(30).arg(_device->GetDeviceInfo()->GetY()-30)).toUtf8());
    _device->GetDeviceConnection()->Write("G1 F1200 Z0 \n");
}

void PrinterControl::on_top_right_button_clicked()
{
    _device->GetDeviceConnection()->Write("G1 F5000\n");
    _device->GetDeviceConnection()->Write("G1 F1200 Z30\n");
    _device->GetDeviceConnection()->Write((QString("G1 F5000 X%0 Y%1 \n").arg(_device->GetDeviceInfo()->GetX()-30).arg(_device->GetDeviceInfo()->GetY()-30)).toUtf8());
    _device->GetDeviceConnection()->Write("G1 F1200 Z0 \n");
}

void PrinterControl::on__bottom_right_button_clicked()
{
    _device->GetDeviceConnection()->Write("G1 F5000\n");
    _device->GetDeviceConnection()->Write("G1 F1200 Z30\n");
    _device->GetDeviceConnection()->Write((QString("G1 F5000 X%0 Y%1 \n").arg(_device->GetDeviceInfo()->GetX()-30).arg(30)).toUtf8());
    _device->GetDeviceConnection()->Write("G1 F1200 Z0 \n");
}

void PrinterControl::on__bottom_left_button_clicked()
{
    _device->GetDeviceConnection()->Write("G1 F5000\n");
    _device->GetDeviceConnection()->Write("G1 F1200 Z30\n");
    _device->GetDeviceConnection()->Write((QString("G1 F5000 X%0 Y%1 \n").arg(30).arg(30)).toUtf8());
    _device->GetDeviceConnection()->Write("G1 F1200 Z0 \n");
}

void PrinterControl::on__change_degree_button_clicked()
{
    _device->GetDeviceConnection()->Write((QString("M104 S%0\n").arg(ui->_hotend->value())).toUtf8());
    _device->GetDeviceConnection()->Write((QString("M140 S%0\n").arg(ui->_bed->value())).toUtf8());
}

void PrinterControl::on__change_positiom_button_clicked()
{
    _device->GetDeviceConnection()->Write((QString("G1 F1000 Z%0 \n").arg(ui->_z->value())).toUtf8());
    _device->GetDeviceConnection()->Write((QString("G1 F3000 X%0 Y%1 \n").arg(ui->_x->value()).arg(ui->_y->value())).toUtf8());

}
