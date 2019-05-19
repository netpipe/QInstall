#include "qinstall.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QInstall w;
    w.show();

    return a.exec();
}
