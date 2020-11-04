#include "serialwidget.h"
#include "ui_serialwidget.h"
#include "../core/device.h"
#include "../core/deviceport.h"
#include <QString>

SerialWidget::SerialWidget(Device *dev, QWidget *parent) :
    QWidget(parent),_device(dev),
    ui(new Ui::SerialWidget)
{
    ui->setupUi(this);
    QObject::connect(_device->GetDevicePort(),&DevicePort::NewLinesAvailable,this,&SerialWidget::WhenDataAvailable);
    QObject::connect(_device->GetDevicePort(),&DevicePort::DataWritten,this,&SerialWidget::WhenDataWritten);
}

SerialWidget::~SerialWidget()
{
    delete ui;
}

void SerialWidget::WhenDataAvailable(QList<QByteArray> ba)
{
    for(QByteArray& text:ba){
        ui->_serial_output->setTextColor(QColor(50,250,50));
        ui->_serial_output->insertPlainText(text+"\n");
    }
}

void SerialWidget::WhenDataWritten()
{
    ui->_send_button->setEnabled(true);
    ui->_gcode_input->setEnabled(true);
}


void SerialWidget::on__send_button_clicked()
{
    Send();
}

void SerialWidget::on__gcode_input_returnPressed()
{
    Send();
}

void SerialWidget::Send()
{
    if(ui->_gcode_input->text().length()<2)
        return;
    ui->_send_button->setEnabled(false);
    ui->_gcode_input->setEnabled(false);
    QByteArray ba="";
    ba=ui->_gcode_input->text().toUtf8().toUpper()+" \n";
    _device->GetDevicePort()->Write(ba);
    ui->_gcode_input->setText("");
    ui->_serial_output->setTextColor(QColor(250,150,50));
    ui->_serial_output->insertPlainText(ba);
}

void SerialWidget::on__gcode_input_textEdited(const QString &arg1)
{
    if(arg1.length()>1)
        ui->_send_button->setEnabled(true);
    else
        ui->_send_button->setEnabled(false);

}
