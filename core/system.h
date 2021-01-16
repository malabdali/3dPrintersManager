#ifndef SYSTEM_H
#define SYSTEM_H

#include <QObject>

class System : public QObject
{
    Q_OBJECT
private: //static
    static System* _Instance;
public: //static
    static System *GetInstance();
private:
    explicit System(QObject *parent = nullptr);
private://fields
    QByteArray _network_id;

public://methods
    void SetNetworkID(const QByteArray& id);
    QByteArray GetNetworkID()const;
    ~System();
signals:

private slots:
    void onApplicationQuit();

};

#endif // SYSTEM_H
