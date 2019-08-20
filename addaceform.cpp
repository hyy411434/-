#include "addaceform.h"
#include "ui_addaceform.h"
#include "pcwindow.h"

#include "controlwin.h"

AddaceForm::AddaceForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AddaceForm)
{
    ui->setupUi(this);

    add_Thread.is_addface = 1;
    add_Thread.start();

    connect(&add_Thread,SIGNAL(send_image(QImage)),this,SLOT(get_Image(QImage)));
    connect(&add_Thread,SIGNAL(send_cut_image(QImage)),this,SLOT(get_cut_Image(QImage)));

    connect(&add_Thread,SIGNAL(send_predict(int)),this,SLOT(get_predict(int)));

     connect(&add_Thread,SIGNAL(send_finish()),this,SLOT(get_finish()));
}

AddaceForm::~AddaceForm()
{
    add_Thread.iscontinue=false;
    add_Thread.wait();
    delete ui;
}

void AddaceForm::get_predict(int predict)
{
    if(pcWindow::predict_id  ==  predict) //说明这是账号的主人
    {
        ui->tis_LB->setText("录取失败,识别结果是:号主");
        add_Thread.iscontinue=false;
        add_Thread.wait();
    }

    else {
        send_userpredict(predict);//发送得到的预测值
        ControlWin::myobject->show();
        delete  this;
    }

}

void AddaceForm::get_finish()
{
     ui->tis_LB->setText("录取脸部结束");
    add_Thread.iscontinue=false;
    add_Thread.wait();
}

void AddaceForm::get_cut_Image(QImage image) //获取截到的脸部
{
    QPixmap map=QPixmap::fromImage(image);

    map= map.scaled(ui->show_bl->size());
    ui->show_bl->setPixmap(map);

}

void AddaceForm::get_Image(QImage image) ////获取摄像头拍到的
{
  //  qDebug()<<"e111";
    QPixmap map=QPixmap::fromImage(image);

    map= map.scaled(ui->face_bl->size());
     ui->face_bl->setPixmap(map);

}

void AddaceForm::on_back_BT_clicked()
{
    add_Thread.iscontinue=false;
    add_Thread.wait();

    ControlWin::myobject->show();
   delete  this;

}
