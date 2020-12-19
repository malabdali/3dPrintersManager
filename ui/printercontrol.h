#ifndef PRINTERCONTROL_H
#define PRINTERCONTROL_H

#include <QWidget>
#include "../core/device.h"
namespace Ui {
class PrinterControl;
}

class PrinterControl : public QWidget
{
    Q_OBJECT
    Device *_device;

public:
    explicit PrinterControl(Device* dev,QWidget *parent = nullptr);
    ~PrinterControl();

private slots:
    void on__home_button_clicked();

    void on__middle_button_clicked();

    void on__top_left_button_clicked();

    void on_top_right_button_clicked();

    void on__bottom_right_button_clicked();

    void on__bottom_left_button_clicked();

    void on__change_degree_button_clicked();

    void on__change_positiom_button_clicked();

private:
    Ui::PrinterControl *ui;
};

#endif // PRINTERCONTROL_H
