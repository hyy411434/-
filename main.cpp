#include "pcwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    pcWindow w;
    w.show();

    return a.exec();
}
