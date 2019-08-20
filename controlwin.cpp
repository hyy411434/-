#include "controlwin.h"
#include "ui_controlwin.h"

#include <QJsonDocument>
#include <QJsonObject>
#include "QTcpSocket"


ControlWin * ControlWin::myobject = NULL;

ControlWin::ControlWin(int predict,QTcpSocket *Crlsocket,QString username, QWidget *parent):
    QWidget(parent),
    ui(new Ui::ControlWin),
    Crlsocket(Crlsocket),
    username(username),predict(predict)
{
    ui->setupUi(this);

    state = 0; //设备开始为0
     ui->lightLb->setText("关");
    connect(Crlsocket,&QTcpSocket::readyRead,this,&ControlWin::read_data);

}

ControlWin::~ControlWin()
{
    delete ui;
}


void ControlWin::read_data()
{
    QByteArray jsondata=Crlsocket->readAll();
    qDebug()<<jsondata;

    QJsonParseError erro;
    QJsonDocument doc=QJsonDocument::fromJson(jsondata,&erro);
    if(erro.error != QJsonParseError::NoError )
    {
        qDebug()<<"json_data erro";
    }

    QJsonObject obj;
    obj = doc.object();
    int i=obj.value("type").toInt();
    if( i == 4 ) //更改成功
    {
         int state=obj.value("state").toInt();
        if(state == 0)
        {
            qDebug()<<"更改成功";
            ui->tis_lb->setText("账号绑定其它人成功!");
            QThread::sleep(3);
             ui->tis_lb->setText("");
        }
    }

}

void ControlWin::get_userpredict(int predict)
{
      /*  {type = 4,uname:xxx, otherID:xxx}  更改otherid*/
    QJsonDocument crl_doc;
    QJsonObject crl_obj;
     crl_obj.insert("type",4);
    crl_obj.insert("username",username);
    crl_obj.insert("otherID",predict); //用户绑定这个0xff00设备
    crl_doc.setObject(crl_obj);

    QByteArray arry=crl_doc.toJson();//封装

    Crlsocket->write(arry);

}

void ControlWin::on_crlBt_clicked()
{
    if(state==1)
    {
        state=0; //关
        ui->lightLb->setText("关");
    }

    else if(state==0)
    {
        state=1; //开--准备拿去发送消息
         ui->lightLb->setText("开");
    }

    /*  {UID:xxx, dev:{id:0xff00, state:1}}*/
    QJsonDocument crl_doc;
    QJsonObject crl_obj;
    crl_obj.insert("UID",username);

    QJsonObject crl_dev_obj;
    crl_dev_obj.insert("id",0xff00); //用户绑定这个0xff00设备
    crl_dev_obj.insert("state",state); //提交开关

    crl_obj.insert("dev",crl_dev_obj);

    crl_doc.setObject(crl_obj);//打包
    QByteArray arry=crl_doc.toJson();//封装

    Crlsocket->write(arry);

}




void ControlWin::on_addface_BT_clicked()
{
    addface = new AddaceForm();
    addface->show();
    this->hide();
    myobject = this;

    connect(addface,&AddaceForm::send_userpredict,this,&ControlWin::get_userpredict);
}
