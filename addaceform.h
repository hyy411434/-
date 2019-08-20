#ifndef ADDACEFORM_H
#define ADDACEFORM_H

#include <QWidget>
#include "facethread.h"

namespace Ui {
class AddaceForm;
}

class AddaceForm : public QWidget
{
    Q_OBJECT

public:
    explicit AddaceForm(QWidget *parent = nullptr);
    ~AddaceForm();
signals:
   void  send_userpredict(int);

private slots:
    void get_cut_Image(QImage image);
    void get_Image(QImage image);

    void get_predict(int predict);
    void on_back_BT_clicked();

    void get_finish();
private:
    Ui::AddaceForm *ui;
    faceThread add_Thread;
};

#endif // ADDACEFORM_H
