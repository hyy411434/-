#ifndef CONTROLWIN_H
#define CONTROLWIN_H

#include <QTcpSocket>
#include <QWidget>
#include "addaceform.h"

namespace Ui {
class ControlWin;
}

class ControlWin : public QWidget
{
    Q_OBJECT

public:
    explicit ControlWin(int predict ,QTcpSocket *Crlsocket,QString username,QWidget *parent = nullptr);
    ~ControlWin();
public:
   static ControlWin * myobject;
private slots:
    void on_crlBt_clicked();

    void on_addface_BT_clicked();

    void get_userpredict(int predict);
    void read_data();
private:
    Ui::ControlWin *ui;
    QTcpSocket *Crlsocket; //已连接上的套接字
    QString username; //用户名
    int state;
    AddaceForm *addface;
    int predict;



};

#endif // CONTROLWIN_H
