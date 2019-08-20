#ifndef FACEFORM_H
#define FACEFORM_H

#include <QWidget>
#include "facethread.h"

#include "QTimer"


namespace Ui {
class faceForm;
}

class faceForm : public QWidget
{
    Q_OBJECT

public:
    explicit faceForm(QWidget *parent = nullptr);
    ~faceForm();
signals:
    void send_userpredict(int);
    void send_outtime();

private slots:
    void on_pushButton_clicked();

    void get_Image(QImage image);
    void get_cut_Image(QImage image);
    void get_predict(int predict);
    void time_out();
private:
    Ui::faceForm *ui;

   faceThread face_thread;
    QTimer timer;


};

#endif // FACEFORM_H
