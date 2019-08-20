#ifndef ARMWINDOW_H
#define ARMWINDOW_H

#include <QMainWindow>
#include <QTcpSocket>
#include <QDebug>

#include <QCryptographicHash>

#include "QJsonArray"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonValue"



namespace Ui {
class ArmWindow;
}

class ArmWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ArmWindow(QWidget *parent = nullptr);
    ~ArmWindow();

protected:
    void con_success();
    void read_data();
private:
    Ui::ArmWindow *ui;
    QTcpSocket Armsocket;

};

#endif // ARMWINDOW_H
