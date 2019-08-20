#ifndef PCWINDOW_H
#define PCWINDOW_H

#include "faceform.h"

#include <QMainWindow>
#include <QTcpSocket>

namespace Ui {
class pcWindow;
}

class pcWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit pcWindow(QWidget *parent = nullptr);
    ~pcWindow();
    static pcWindow *myobject; //静态指针
        static int  predict_id; //保存这个用户的主人的预测值

private slots:
    void con_success();
    void read_data();
    void on_loginBT_clicked();

    void on_regBT_clicked();

    void get_userpredict(int predict);
    void get_outtime();
private:
    Ui::pcWindow *ui;
    QTcpSocket PcSocket;

    faceForm *win_face;

    int predict;


    int  predict_otherid;


};

#endif // PCWINDOW_H
