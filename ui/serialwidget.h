#ifndef SERIALWIDGET_H
#define SERIALWIDGET_H

#include <QWidget>

namespace Ui {
class SerialWidget;
}

class SerialWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SerialWidget(QWidget *parent = nullptr);
    ~SerialWidget();

private:
    Ui::SerialWidget *ui;
};

#endif // SERIALWIDGET_H
