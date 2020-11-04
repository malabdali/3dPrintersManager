#ifndef SERIALWIDGET_H
#define SERIALWIDGET_H

#include <QWidget>

namespace Ui {
class SerialWidget;
}

class SerialWidget : public QWidget
{
    Q_OBJECT

private://fields
    class Device* _device;
public:
    explicit SerialWidget( Device* dev,QWidget *parent = nullptr);
    ~SerialWidget();

private slots:
    void WhenDataAvailable(QList<QByteArray> ba);
    void WhenDataWritten();

    void on__send_button_clicked();

    void on__gcode_input_returnPressed();
    void on__gcode_input_textEdited(const QString &arg1);

private://methods
    void Send();
private:
    Ui::SerialWidget *ui;
};

#endif // SERIALWIDGET_H
