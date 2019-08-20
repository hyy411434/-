#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <QTcpSocket>
#include <QTcpServer>
#include <QMap>
#include <QSqlTableModel>


class TcpServer:public QObject
{
public:
    explicit TcpServer(QObject *parent=nullptr);
    virtual ~TcpServer();
protected:
    void set_key(QTcpSocket *new_socket);
    void login_validate(QJsonObject &obj, QTcpSocket *new_socket);
    void client_data(QJsonObject &obj, QString Uid, QTcpSocket *new_socket);
    void arm_data(QJsonObject &obj, int arm_id, QTcpSocket *new_socket);
    void register_validate(QJsonObject &obj, QTcpSocket *new_socket);
    void face_validate(QJsonObject &obj, QTcpSocket *new_socket);
    void face_changeinfo(QJsonObject &obj, QTcpSocket *new_socket);
private slots:
    void new_client();
    void read_data();
private:
    QTcpServer mserver;
    QMap<int,QTcpSocket*> client_socket;
    QSqlTableModel *model_uesr;
   QSqlTableModel * model_dev;
   int login;

   int socket_fd; //管理所有套接字
   int  row_uesr;


};

#endif // TCPSERVER_H
