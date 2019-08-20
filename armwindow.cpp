#include "armwindow.h"
#include "ui_armwindow.h"

#include <QTcpSocket>
#include <QDebug>

#include <QCryptographicHash>

#include "QJsonArray"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonValue"
#include "QJsonParseError"



ArmWindow::ArmWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ArmWindow)
{
    ui->setupUi(this);
    Armsocket.connectToHost("192.168.12.74",8080);//连到这个服务器上
    connect(&Armsocket,&QTcpSocket::connected,this,&ArmWindow::con_success);//连接成功信号
    connect(&Armsocket,&QTcpSocket::readyRead,this,&ArmWindow::read_data);
}

ArmWindow::~ArmWindow()
{
    delete ui;
}

void ArmWindow::con_success()
{
    qDebug()<<"链接成功";

    //json 发送登陆数据给 服务器
    QJsonDocument doc; //创建个json文件

    QJsonObject obj; //数据打包
    obj.insert("ID",0xff00);//登陆  0xff00是这个设备的ID 唯一
    obj.insert("DevState",0);//登陆
    doc.setObject(obj); // 打包完所有数据

    QByteArray json_data=doc.toJson(); //变成json

    //发送数据给服务器
    //{ ID:0xff00 , DevState:0 }
    Armsocket.write(json_data);
}

void ArmWindow::read_data()
{
   QByteArray json_data= Armsocket.readAll();

   qDebug()<<json_data;

   QJsonParseError erro;
    QJsonDocument doc =QJsonDocument::fromJson(json_data,&erro);

   if(erro.error!=QJsonParseError::NoError) //json有问题，就终结程序
   {
       qDebug()<<"json_data erro";
     //  exit(-1);
   }

   //解析： {state:1}
   QJsonObject obj = doc.object();

   int  state_ = obj.value("state").toInt();
    //改变灯的样式

    qDebug()<<"state"<<state_;
    if(state_ == 1) //工作
    {
        QString style  = QString("background-color: rgb(255, 0, 0);");
        ui->label->setStyleSheet(style);
        qDebug()<<"1";

        ui->state_bl->setText("正在工作...");
    }

    if(state_ == 0) //不工作
    {
        QString style  = QString("background-color: rgb(0, 0, 0);");
        ui->label->setStyleSheet(style);
        qDebug()<<"0";

         ui->state_bl->setText("停止工作...");
    }

}
