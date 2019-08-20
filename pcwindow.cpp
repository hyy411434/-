
#include "pcwindow.h"
#include "ui_pcwindow.h"

#include <QTcpSocket>
#include <QDebug>
#include <QThread>

#include <QCryptographicHash>

#include "QJsonArray"
#include "QJsonDocument"
#include "QJsonObject"
#include "QJsonValue"
#include "controlwin.h"

#include "faceform.h"

pcWindow * pcWindow::myobject = NULL;
int pcWindow::predict_id = 0;

pcWindow::pcWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::pcWindow)
{
    ui->setupUi(this);
    PcSocket.connectToHost("192.168.12.74",8080);//连到这个服务器上
    connect(&PcSocket,&QTcpSocket::connected,this,&pcWindow::con_success);//连接成功信号
    predict_otherid =0;


}

pcWindow::~pcWindow()
{
    delete ui;
}


void pcWindow::con_success()
{
    qDebug()<<"connect 192.168.12.74 success";
    connect(&PcSocket,&QTcpSocket::readyRead,this,&pcWindow::read_data); //可读数据信号
}

void pcWindow::get_outtime()
{
    qDebug()<<"人脸识别超时，验证失败";
    delete win_face;
     ui->label->setText("人脸识别超时，验证失败");
}

void pcWindow::get_userpredict(int predict )//获取从人脸识别中的预测值，发送预测值给服务器
{
    //发送账号和人脸识别出来的predict
    /*{username:hqd,  ID：predict type:3}---type =3人脸识别*/
    QString username=ui->userEdit->text();

    //json 发送登陆数据给 服务器
    QJsonDocument doc; //创建个json文件
    qDebug()<<predict_otherid <<","<<predict_id;

    if(predict ==predict_otherid || predict == predict_id)
    {
        QJsonObject obj; //数据打包
        obj.insert("username",username); //不用：
        obj.insert("ID",predict_id); //必定成功的，赶紧好像不用服务器的人脸识别
        obj.insert("type",3);//人脸识别
        doc.setObject(obj); // 打包完所有数据

        QByteArray json_data=doc.toJson(); //变成json
        qDebug()<<json_data; //看看是不是json数据，是就准备发送

        //发送数据给服务器
        PcSocket.write(json_data);

        this->predict = predict;
    }
    else{
        qDebug()<<"人脸识别失败";
        delete win_face;
        ui->label->setText("人脸识别失败");
    }


}

void  pcWindow::read_data() //解析服务器的数据
{
    QByteArray jsondata=PcSocket.readAll();
    qDebug()<<jsondata;

    //解析json数据
    QJsonParseError erro; //判断json数据有没有问题
    QJsonDocument doc=QJsonDocument::fromJson(jsondata,&erro);
    if(erro.error != QJsonParseError::NoError )
    {
        qDebug()<<"json_data erro";
        exit(-1);
    }

    /* {type:0, state:1}  */
    QJsonObject obj;
    obj = doc.object();
    int i=obj.value("type").toInt();

    QJsonDocument login_doc;
    QJsonObject login_obj;
    if(i == 0)//登陆返回的结果
    {
        qDebug()<<obj.value("state").toString()<<endl;

        int state = obj.value("state").toInt();
        if(state == 1)//失败
        {
            qDebug()<<"login 失败";
        }
        if(state == 0)//成功
        {
            qDebug()<<"login 成功，进入人脸界面";
            predict_id = obj.value("ID").toInt();
            predict_otherid  = obj.value("otherID").toInt(); //可能是绑定的id
          //  qDebug()<<predict_otherid <<","<<predict_id;

            win_face =new faceForm;
            win_face->show();
            this->hide();
            pcWindow::myobject = this; //静态对象，用于返回这个界面
            connect(win_face,&faceForm::send_userpredict,this,&pcWindow::get_userpredict); //等待预测值
            connect(win_face,&faceForm::send_outtime,this,&pcWindow::get_outtime); //等待超时
        }
    }

    if( i== 1) //人脸返回的结果
    {
        qDebug()<<obj.value("state").toString()<<endl;

        int state = obj.value("state").toInt();
        if(state == 1)//失败 不可能会失败
        {
            qDebug()<<"人脸识别失败";
            delete win_face;
             ui->label->setText("人脸识别失败");

        }
        if(state == 0)//成功
        {
            qDebug()<<"人脸识别成功";
            delete win_face; //释放人脸识别
            ControlWin *win =new ControlWin(predict,&PcSocket,ui->userEdit->text());
            win->show();
            this->hide();
            disconnect(&PcSocket,&QTcpSocket::readyRead,this,&pcWindow::read_data);

        }

    }

    if(i == 2)//注册返回的结果
    {
        qDebug()<<obj.value("state").toString()<<endl;

        int state = obj.value("state").toInt();
        if(state == 1)//失败
        {
            qDebug()<<"注册 失败";
             ui->label->setText("注册 失败  用户已存在");
        }
        if(state == 0)//成功
        {
            qDebug()<<"注册 成功";
            ui->label->setText("注册 成功");
        }
    }


}

void pcWindow::on_loginBT_clicked() //登陆
{
    //发送账号密码
    /*{username:hqd,  password:*******, type:1}---type =1登录*/
    QString username=ui->userEdit->text();
    QString userpass=ui->passEdit->text();

    //加密
    QCryptographicHash hash(QCryptographicHash::Md5); //hash对象
    hash.addData(userpass.toUtf8()); //通过hash对象来实现 加密
    QByteArray hash_ret=hash.result(); //返回加密的结果,中间变量接住
    userpass=hash_ret.toHex();  //通过中间对象来转成16进制，userpass装回密码
    qDebug()<<userpass; //变成16进制

    //json 发送登陆数据给 服务器
    QJsonDocument doc; //创建个json文件

    QJsonObject obj; //数据打包
    obj.insert("username",username); //不用：
    obj.insert("password",userpass);
    obj.insert("type",1);//登陆
    doc.setObject(obj); // 打包完所有数据

    QByteArray json_data=doc.toJson(); //变成json
    qDebug()<<json_data; //看看是不是json数据，是就准备发送

    //发送数据给服务器
    PcSocket.write(json_data);

}

void pcWindow::on_regBT_clicked() //注册
{
    /*{username:hqd,  password:*******, type:2}---type =2  注册*/
    QString username=ui->userEdit->text();
    QString userpass=ui->passEdit->text();

    //加密
    QCryptographicHash hash(QCryptographicHash::Md5); //hash对象
    hash.addData(userpass.toUtf8()); //通过hash对象来实现 加密
    QByteArray hash_ret=hash.result(); //返回加密的结果,中间变量接住
    userpass=hash_ret.toHex();  //通过中间对象来转成16进制，userpass装回密码
    qDebug()<<userpass; //变成16进制

    //json 发送登陆数据给 服务器
    QJsonDocument doc; //创建个json文件

    QJsonObject obj; //数据打包
    obj.insert("username",username); //不用：
    obj.insert("password",userpass);
    obj.insert("type",2);//注册
    doc.setObject(obj); // 打包完所有数据

    QByteArray json_data=doc.toJson(); //变成json
    qDebug()<<json_data; //看看是不是json数据，是就准备发送

    //发送数据给服务器
    PcSocket.write(json_data);


}




