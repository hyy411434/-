#include "armwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    ArmWindow w;
    w.show();

    return a.exec();
}
