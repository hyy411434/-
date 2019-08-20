#include "facethread.h"

#include "QDebug"

#include <opencv2/opencv.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/face.hpp>

#include <opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include "QImage"

#include <QDir>
#include <face.hpp>
#include <face.hpp>

#include <face/facerec.hpp>
using namespace cv;
using namespace  std;

#include "pcwindow.h"


faceThread::faceThread(QObject *parent):QThread (parent)
{
    //2、导入分类器文件
    bool isload=cascede.load("D:\\3-18\\qt_project\\xml\\haarcascade_frontalface_alt2.xml"); //路径不对，一直出错
    if(!isload)
    {
        qDebug()<<"cascede.load erro";
    }

    faceCompare_index = 0;

    image_vector_index = 0; //用来录入脸部的

    is_addface = 0; //默认不是录入脸

    iscontinue = true;

}

faceThread::~faceThread()
{

}


void faceThread::run()
{

    Ptr<face::FaceRecognizer> model1 =  face::FisherFaceRecognizer::create();
    model1->read("D:\\3-18\\qt_project\\xml\\MyFaceFisherModel.xml");

    Ptr<face::FaceRecognizer> model2 =  face::LBPHFaceRecognizer::create();
    model2->read("D:\\3-18\\qt_project\\xml\\MyFaceLBPHModel.xml");

    Ptr<face::FaceRecognizer> model3 =  face::EigenFaceRecognizer::create();
    model3->read("D:\\3-18\\qt_project\\xml\\MyFacePCAModel.xml");

    if(is_addface != 0)
    {

        QDir dir;
        dir.cd("D:\\3-18\\qt_project\\facepic");  //进入某文件夹

        filename = QString("%1002").arg(pcWindow::predict_id);
        filepath = QString("D:\\3-18\\qt_project\\facepic\\%1").arg(filename);

        qDebug()<<filename;
        qDebug()<<filepath;

        if(!dir.exists(filename))//判断需要创建的文件夹是否存在
        {
            dir.mkdir(filename); //创建文件夹
        }
    }


    //打开摄像头
    VideoCapture vfd(0);
    while(iscontinue)//不断的读取摄像头的帧
    {
        //1、获取摄像头帧
        vfd>>videoframe;
       // qDebug()<<videoframe.type();

        //3、红蓝色转换rgb
        cvtColor(videoframe,rgb_image,CV_BGR2RGB);
        //4、转成使用灰度图
        cvtColor(videoframe,grayframe,CV_BGR2RGBA);

        //4、使用分类器提取人脸

       // qDebug()<<grayframe.rows<<"  "<<grayframe.cols;

        cascede.detectMultiScale(grayframe,faceRect,1.1,4,CV_HAAR_DO_ROUGH_SEARCH,Size(70,70));

        if(faceRect.size()>0)
        {
            Mat face_cut=videoframe(faceRect[0]);//截取Mat 原图图里面的一部分

            Mat face_cut2 = face_cut;
            ::resize(face_cut,face_cut2,Size(92,112)); //所有图片统一大小

           cvtColor(face_cut2,face_cut2,CV_RGB2GRAY); //这里一定一定要用原图！！！！

           // qDebug()<<face_cut2.cols<<","<<face_cut2.rows;

            int label1 = model1->predict(face_cut2);
            int label2 = model2->predict(face_cut2);
            int label3 = model3->predict(face_cut2);

            qDebug()<<"label1:"<<label1;
            qDebug()<<"label2:"<<label2;
            qDebug()<<"label3:"<<label3; //获取三个分类器的预测值

            if(label1 == label2 && label1 ==label3)
            {
                faceCompare_index++;
                if(faceCompare_index == 5)
                {
                    qDebug()<<"此人在数据库中";
                    send_predict(label1);
                    faceCompare_index = 0;
                    break;
                }

                 image_vector_index =0; //说明脸是在数据库中的
            }

            if(label1 != label2 || label1 !=label3 || label2 !=label3 )
            {
                faceCompare_index=0;
                qDebug()<<"此人不在数据库中";

                if(is_addface != 0 )
                {
                    image_vector.push_back(face_cut2);

                    image_vector_index++; //当它加到50，就提出出来
                    if(image_vector_index == 50)
                    {
                        Mat add_farme  = image_vector.at(49);
                         QImage image_cut = QImage(add_farme.data,add_farme.cols,add_farme.rows,add_farme.step,QImage::Format_Grayscale8);

                        emit send_cut_image(image_cut); //发送截图

                        QString filenamge= QString("%1\\%2.jpg").arg(filepath).arg(file_id);


                         ::imwrite(filenamge.toStdString(),add_farme); //保存录入的图片



                        if(file_id == 10) //如果照了10张照片
                        {
                            //生成csv文件
                            scvfile();

                            //生成训练文件
                            tranfile();

                            send_finish();
                            break;
                        }
                        file_id++;
                        image_vector_index = 0;
                    }

                }

            }





            //截图到的人脸发送
            int i=0;
            qDebug()<<"SIZE:"<<faceRect.size();
            for (;i<faceRect.size();i++) {
                //qDebug()<<"1个人";
                rectangle(rgb_image,faceRect[i],Scalar(255,255,0),2,8,0); //在原图像上绘制图形
            }

            qDebug()<<"9999";
            String s="ing...";

             cv::putText(rgb_image,s,Size(faceRect.at(0).x,faceRect.at(0).y+5),CV_FONT_HERSHEY_SIMPLEX,6,cv::Scalar(0, 255, 255),6);
        }




        //原图发送数据
        QImage image =QImage(rgb_image.data,rgb_image.cols,rgb_image.rows, rgb_image.step, QImage::Format_RGB888);//转成其它格式,frame.data是char *指针

        emit send_image(image);

       msleep(10); //延迟10ms
      //qDebug()<<"666";

    }

}

//使用CSV文件去读图像和标签，主要使用stringstream和getline方法
static void read_csv(const string& filename, vector<Mat>& images, vector<int>& labels, char separator = ';') {
    ifstream file(filename.c_str(), ifstream::in);
    if (!file)
    {
        string error_message = "No valid input file was given, please check the given filename.";
        CV_Error(CV_StsBadArg, error_message);
    }
    string line, path, classlabel;
    while (getline(file, line)) //从文本文件中读取一行字符，未指定限定符默认限定符为“/n”
    {
        stringstream liness(line);//这里采用stringstream主要作用是做字符串的分割
        getline(liness, path, separator);//读入图片文件路径以分好作为限定符
        getline(liness, classlabel);//读入图片标签，默认限定符
        if (!path.empty() && !classlabel.empty()) //如果读取成功，则将图片和对应标签压入对应容器中
        {
            images.push_back(imread(path, 0));
            labels.push_back(atoi(classlabel.c_str()));
        }
    }
}


void faceThread::tranfile()
{
    //读取你的CSV文件路径.
       string fn_csv = "D:\\3-18\\qt_project\\faceTran\\test_face2\\build-myproject-Desktop_Qt_5_12_0_MinGW_64_bit-Debug\\myat.txt";

       // 2个容器来存放图像数据和对应的标签
       vector<Mat> images;
       vector<int> labels;
       // 读取数据. 如果文件不合法就会出错
       // 输入的文件名已经有了.
       try
       {
           read_csv(fn_csv, images, labels); //从csv文件中批量读取训练数据
       }
       catch (cv::Exception& e)
       {
           cerr << "Error opening file \"" << fn_csv << "\". Reason: " << e.msg << endl;
           // 文件有问题，我们啥也做不了了，退出了
           exit(1);
       }
       // 如果没有读取到足够图片，也退出.
       if (images.size() <= 1) {
           string error_message = "This demo needs at least 2 images to work. Please add more images to your data set!";
           CV_Error(CV_StsError, error_message);
       }

       for (int i = 0; i < images.size(); i++)
       {
           //cout<<images.size();
           if (images[i].size() != Size(92, 112))
           {
               cout << i << endl;
               cout << images[i].size() << endl;
           }

       }

       // 下面的几行代码仅仅是从你的数据集中移除最后一张图片，作为测试图片
       //[gm:自然这里需要根据自己的需要修改，他这里简化了很多问题]
       Mat testSample = images[images.size() - 1];
       int testLabel = labels[labels.size() - 1];
       images.pop_back();//删除最后一张照片，此照片作为测试图片
       labels.pop_back();//删除最有一张照片的labels


       Ptr<face::BasicFaceRecognizer> model = face::EigenFaceRecognizer::create();
       model->train(images, labels);
       model->save("D:\\3-18\\qt_project\\xml\\MyFacePCAModel.xml");//保存路径可自己设置，但注意用“\\”

       Ptr<face::BasicFaceRecognizer> model1 = face::FisherFaceRecognizer::create();
       model1->train(images, labels);
       model1->save("D:\\3-18\\qt_project\\xml\\MyFaceFisherModel.xml");

       Ptr<face::LBPHFaceRecognizer> model2 = face::LBPHFaceRecognizer::create();
       model2->train(images, labels);
       model2->save("D:\\3-18\\qt_project\\xml\\MyFaceLBPHModel.xml");

       // 下面对测试图像进行预测，predictedLabel是预测标签结果
       //注意predict()入口参数必须为单通道灰度图像，如果图像类型不符，需要先进行转换
       //predict()函数返回一个整形变量作为识别标签
       int predictedLabel = model->predict(testSample);//加载分类器
       int predictedLabel1 = model1->predict(testSample);
       int predictedLabel2 = model2->predict(testSample);

       qDebug()<<predictedLabel;
       qDebug()<<predictedLabel1;
       qDebug()<<predictedLabel2;

}

void faceThread::scvfile()
{
    ofstream f1("./myat.txt");
    if(!f1)return;


    //获取文件夹下多少个文件
    QDir *dir=new QDir("D:\\3-18\\qt_project\\facepic");
     QStringList filter;
     QList<QFileInfo> *fileInfo =new QList<QFileInfo>(dir->entryInfoList(filter));
     QList<QString> filePath;
      QList<QString> savePath;
       QList<QString> savefilename;

     qDebug()<<fileInfo->size()-2;

     for(int i =0 ;i<fileInfo->size();i++)
     {
         if(fileInfo->at(i).fileName().at(0)!='.') //排除.. .
         {
              QString str=QString("D:\\3-18\\qt_project\\facepic\\%1").arg(fileInfo->at(i).fileName());
              QString str_save=QString("D:/3-18/qt_project/facepic\\%1").arg(fileInfo->at(i).fileName());

              filePath.push_back(str); //PATH + name
              savePath.push_back(str_save); //PATH + name
              savefilename.push_back(fileInfo->at(i).fileName()); // name

              qDebug()<<str_save;
         }

     }

     for(int i=0; i<filePath.size(); i++) //image下有多少个用户文件夹
     {
         QDir dir(filePath.at(i));//进去这个image目录下的某个文件夹

         QList<QFileInfo> fileAll=dir.entryInfoList(); //获取dir下所有的文件信息

         for (int j=0;j<fileAll.size();j++) { //获取每一张图片

             if(fileAll.at(j).fileName().at(0)!='.') //字符串的第0个字符
             {
                 QString str=QString("%1/%2;%3").arg(savePath.at(i)).arg(fileAll.at(j).fileName()).arg(savefilename.at(i));
                 qDebug()<<str;
                 f1<<str.toStdString()<<endl;

             }
         }

      }

    f1.close();

}



