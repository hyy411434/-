#include "faceform.h"
#include "ui_faceform.h"

#include "pcwindow.h"



faceForm::faceForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::faceForm)
{
    ui->setupUi(this);
    face_thread.start();

    //接受人脸线程发过来的信号
    connect(&face_thread,SIGNAL(send_image(QImage)),this,SLOT(get_Image(QImage)));
    connect(&face_thread,SIGNAL(send_cut_image(QImage)),this,SLOT(get_cut_Image(QImage)));

    connect(&face_thread,SIGNAL(send_predict(int)),this,SLOT(get_predict(int)));

    connect(&timer,&QTimer::timeout,this,&faceForm::time_out);
    timer.start(15000);//20s

}

faceForm::~faceForm()
{
     face_thread.wait();
    delete ui;
}
void faceForm::time_out()
{

   face_thread.iscontinue = false;

    timer.stop();
    emit send_outtime(); //发送超时信号，人脸识别失败
    pcWindow::myobject->show();
}


void faceForm::get_predict(int predict)
{
    send_userpredict(predict);//发送得到的预测值
    face_thread.iscontinue = false;
    pcWindow::myobject->show();
}



void faceForm::get_cut_Image(QImage image) //获取截到的脸部
{
    QPixmap map=QPixmap::fromImage(image);
   map= map.scaled(ui->show_lb->size());
    ui->show_lb->setPixmap(map);
}

void faceForm::get_Image(QImage image) ////获取摄像头拍到的
{
  //  qDebug()<<"e111";
    QPixmap map=QPixmap::fromImage(image);
   map= map.scaled(ui->face_LB->size());
    ui->face_LB->setPixmap(map);

}



void faceForm::on_pushButton_clicked()
{
    pcWindow::myobject->show();
   // this->close();
}


