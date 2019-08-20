#include "tcpserver.h"


#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>

#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QSqlTableModel>

#include "QJsonArray"
#include "QJsonObject"
#include "QJsonDocument"
#include "QJsonValue"

TcpServer::TcpServer(QObject *parent):QObject(parent)
{
    mserver.listen(QHostAddress::Any,8080); //服务器一启动就监听

    login = 0; //未登录


    connect(&mserver,&QTcpServer::newConnection,this,&TcpServer::new_client); //有新客户端来

    //数据库

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");//指定数据库的驱动（类型）
    db.setDatabaseName("D:\\3-18\\qt_project\\build-8-7-pc-Desktop_Qt_5_12_0_MinGW_64_bit-Debug\\Smart_home.db");//设置数据库名字
    if(!db.open()) //打开数据库，打不开就终结
    {
        qDebug()<<"user.db open error";
        exit(-1);
    }

    //插入表格
    QString sql = QString("create table user (ID integer primary key autoincrement,uname char(32) not NULL ,  pword  text not NULL ,socketfd int not NULL)");
    QSqlQuery query;
    if( !query.exec(sql))
    {
        qDebug()<<"create table user error";
       // exit(-1);
    }

     sql = QString("create table device (ID integer primary key autoincrement,socketfd int not NULL ,  DevState int not NULL ,UID char(32) not NULL)");
    if( !query.exec(sql))
    {
        qDebug()<<"create table device error";
       // exit(-1);
    }


    //设置两个模型
    model_uesr=new QSqlTableModel();
    model_uesr->setTable("user");

    model_dev=new QSqlTableModel();
    model_dev->setTable("device");

    model_uesr->select(); //查询所有
    row_uesr=model_uesr->rowCount(); //获取row值

    socket_fd = 1;


}

TcpServer::~TcpServer()
{

}

void TcpServer::new_client()
{
    QTcpSocket *new_socket=mserver.nextPendingConnection(); //链接上的客户端套接字
  //  set_key(new_socket); //设置键值对

    connect(new_socket,&QTcpSocket::readyRead,this,&TcpServer::read_data); //等待数据


}

void TcpServer::set_key(QTcpSocket *new_socket)
{
    //
    QSqlTableModel model;
    model.setTable("user");
    model.setFilter("hhh");//设置过滤器
    if(!model.select())
    {
        qDebug()<<"找到该用户";
    }
    QSqlRecord record =model.record();
    qDebug()<<record.value(0);

   // client_socket.insert();
}

void TcpServer::read_data()
{
    QTcpSocket *new_socket = dynamic_cast<QTcpSocket *>( sender() ) ;
    QByteArray jsondata=new_socket->readAll();
    qDebug()<<jsondata;

    //解析json数据
    QJsonParseError erro; //判断json数据有没有问题
    QJsonDocument doc=QJsonDocument::fromJson(jsondata,&erro);
    if(erro.error != QJsonParseError::NoError )
    {
        qDebug()<<"json_data erro";
      //  exit(-1);
    }


    /* {username:hqd,  password:*******, type:1}  登陆*/
    QJsonObject obj;
    obj = doc.object();
    int i=obj.value("type").toInt();
    if( i == 1 ) //登陆
    {
        login_validate(obj,new_socket);
    }

    if( i == 2 ) //注册
    {
        register_validate(obj,new_socket);
    }
    if(i==3) //人脸
    {
        face_validate(obj,new_socket);
    }

    if(i==4) //更改亲属脸部信息
    {
        face_changeinfo(obj,new_socket);
    }





     /*  {UID:xxx, dev:{id:0xff00, state:1}}  控制*/
    QString Uid =obj.value("UID").toString(); //控制设备
    if(Uid!="")
    {
        //解析客户端的控制信息
        client_data(obj,Uid,new_socket);
    }

     /*   ID:0xff00 , DevState:0  开发板初始化信息*/
    int arm_id= obj.value("ID").toInt(); //
    if(arm_id !=0)
    {
        arm_data(obj,arm_id,new_socket);//ARM的数据信息
    }

}

void TcpServer::face_changeinfo(QJsonObject &obj,QTcpSocket *new_socket)
{
    QJsonDocument change_doc;
    QJsonObject change_obj;
    qDebug()<<obj.value("username").toString()<<"   "<<obj.value("otherID").toInt()<<endl;

    QString username = obj.value("username").toString();
    int otherid = obj.value("otherID").toInt();

    QString filter=QString("uname = '%1'").arg(username);

    model_uesr->setFilter(filter);//设置过滤器
    model_uesr->select();
    QSqlRecord record =model_uesr->record(0); //记得第几条记录

    //更改信息
    record.setValue("otherID",otherid); //更改otherID这个列
    model_uesr->setRecord(0,record); //第0行的记录更改,然后放入模板中
    model_uesr->submitAll(); //模板提交
    model_uesr->database().commit();//数据库更新

    //表示更改成功
    change_obj.insert("type",4);
    change_obj.insert("state",0);
    change_doc.setObject(change_obj);//打包
    QByteArray arry=change_doc.toJson();//封装
    new_socket->write(arry);

}

void TcpServer::face_validate(QJsonObject &obj,QTcpSocket *new_socket)
{
    QJsonDocument login_doc;
    QJsonObject login_obj;
    qDebug()<<obj.value("username").toString()<<"   "<<obj.value("ID").toInt()<<endl;

    QString username = obj.value("username").toString();
    int id = obj.value("ID").toInt();

    QString filter=QString("uname = '%1'").arg(username).arg(id);

    model_uesr->setFilter(filter);//设置过滤器
    model_uesr->select();
    QSqlRecord record =model_uesr->record(0); //记得第几条记录
    if(record.isNull(0))
    {
        qDebug()<<"人脸识别失败";
        /*{type:0, state:1}; state 0识别成功，1识别失败*/
        login_obj.insert("type",1);
        login_obj.insert("state",1);
        login_doc.setObject(login_obj);//打包
        QByteArray arry=login_doc.toJson();//封装
        new_socket->write(arry); //发送
    }
    if(!record.isNull(0))
    {
        qDebug()<<"人脸识别成功";
        login_obj.insert("type",1);
        login_obj.insert("state",0);
        login_doc.setObject(login_obj);//打包
        QByteArray arry=login_doc.toJson();//封装

        //更新记录
        record.setValue("socketfd",socket_fd);
        model_uesr->setRecord(0,record);
        model_uesr->submitAll();
        model_uesr->database().commit();

        client_socket.insert(socket_fd,new_socket);//一个用户名对应一个socketfd
        client_socket.value(socket_fd)->write(arry); //发送

        socket_fd++;
    }
}

void TcpServer::register_validate(QJsonObject &obj,QTcpSocket *new_socket)
{
    QJsonDocument reg_doc; //回复信息用的
    QJsonObject reg_obj;

    qDebug()<<"注册"<<obj.value("username").toString()<<"   "<<obj.value("password").toString()<<endl;

    QString username = obj.value("username").toString();
    QString userpass = obj.value("password").toString();

    QString filter=QString("uname='%1'").arg(username);

    model_uesr->setFilter(filter);//设置过滤器
    model_uesr->select();
    QSqlRecord record =model_uesr->record(0); //记得第几条记录
    if(record.isNull(0)) //如果没有找到这个用户，就可以注册
    {
        //插入注册信息
        char buf[1024]={0}; //插进去好像不能用Qstring，有int
        row_uesr++;
        //，和空格 注意点，很容易出错
        sprintf(buf,"insert into user  values (%d,'%s','%s',%d)",row_uesr,username.toStdString().c_str(),userpass.toStdString().c_str(),socket_fd);
        QSqlQuery query;
       if(!query.exec(buf))
       {
            qDebug()<<"失败";
       }

        //{type:2, state:1}; state 0注册成功，1用户以及存在， 3，失败
        reg_obj.insert("type",2);
        reg_obj.insert("state",0);
        reg_doc.setObject(reg_obj);//打包
        QByteArray arry=reg_doc.toJson();//封装

        client_socket.insert(socket_fd,new_socket); //存起来
        client_socket.value(socket_fd)->write(arry); //发送

        socket_fd++;
    }
    if(!record.isNull(0))
    {
        qDebug()<<"注册失败";
        reg_obj.insert("type",2);
        reg_obj.insert("state",1);
        reg_doc.setObject(reg_obj);//打包
        QByteArray arry=reg_doc.toJson();//封装

        client_socket.insert(socket_fd,new_socket); //存起来
        client_socket.value(socket_fd)->write(arry); //发送
    }

}

void TcpServer::arm_data(QJsonObject &obj,int arm_id,QTcpSocket *new_socket)
{
    model_dev->setFilter(QString("ID=%1").arg(arm_id));
    model_dev->select();
    QSqlRecord record=model_dev->record(0);
    if(!record.isEmpty())//更新device的信息
    {
        int DevState=obj.value("DevState").toInt();
        record.setValue("DevState",DevState);
        record.setValue("socketfd",socket_fd);
        model_dev->setRecord(0,record);
        model_dev->submitAll();
        model_dev->database().commit();

        client_socket.insert(socket_fd,new_socket); //绑定套接字

        socket_fd++;
    }
}

void TcpServer::login_validate(QJsonObject &obj,QTcpSocket *new_socket) //登陆是否正确
{
    QJsonDocument login_doc;
    QJsonObject login_obj;
    qDebug()<<obj.value("username").toString()<<"   "<<obj.value("password").toString()<<endl;

    QString username = obj.value("username").toString();
    QString userpass = obj.value("password").toString();

    QString filter=QString("uname='%1' and pword='%2'").arg(username).arg(userpass);

    model_uesr->setFilter(filter);//设置过滤器
    model_uesr->select();
    QSqlRecord record =model_uesr->record(0); //记得第几条记录
    if(record.isNull(0))
    {
        qDebug()<<"登陆失败";
        /*{type:0, state:1}; state 0登录成功，1登录失败*/
        login_obj.insert("type",0);
        login_obj.insert("state",1);
        login_doc.setObject(login_obj);//打包
        QByteArray arry=login_doc.toJson();//封装
        new_socket->write(arry); //发送
    }
    if(!record.isNull(0))
    {
        int ID=record.value(0).toInt();
        int otherID=record.value(4).toInt();
        qDebug()<<"id"<<ID<<",otherID:"<<otherID;

        qDebug()<<"登陆成功";
        login_obj.insert("type",0);
        login_obj.insert("state",0);
        login_obj.insert("ID",ID);
        login_obj.insert("otherID",otherID);

        login_doc.setObject(login_obj);//打包
        QByteArray arry=login_doc.toJson();//封装
        new_socket->write(arry); //发送
    }
}

void TcpServer::client_data(QJsonObject &obj,QString Uid,QTcpSocket *new_socket) //PC的控制信息处理
{
    QJsonObject dev_obj =obj.value("dev").toObject();
    int id= dev_obj.value("id").toInt();


    QString filter=QString("UID='%1' and ID=%2").arg(Uid).arg(id); //xxx  0xff00
    model_dev->setFilter(filter);//设置过滤器
    model_dev->select();
    QSqlRecord record =model_dev->record(0); //提出来 第几条记录
    qDebug()<<record.value(2).toString(); //state

    int user_state= dev_obj.value("state").toInt(); //PC端发来的状态
    record.setValue("DevState",user_state); //更改DevState

    int arm_socketfd=record.value(1).toInt();//arm_socket用来发送消息------------------>关键点

    if(arm_socketfd!=0) //arm板子在线，才发送
    {
        //包装json数据，发给开发板
        //     {state:1}
        QJsonDocument doc; //创建个json文件
        QJsonObject obj_state; //数据打包
        obj_state.insert("state",user_state);

        doc.setObject(obj_state);
        QByteArray json_data=doc.toJson(); //变成json

        client_socket.value(arm_socketfd)->write(json_data);
    }

    model_dev->setRecord(0,record); //第0行的记录更改,然后放入模板中
    model_dev->submitAll(); //模板提交
    model_dev->database().commit();//数据库更新
}
