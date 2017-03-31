#include "ppwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PPWindow w;
    w.show();

    return a.exec();
}
