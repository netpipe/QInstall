#include "qinstall.h"
#include <QApplication>
#include <QFile>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QInstall w;
    w.show();

    QPixmap m( ./ + "logo.png");


MainWindow w;

QSplashScreen splash(m);
splash.show();

splash.finish(&w);


    QFile file("Resource/themes/qdarkstyle/qdarkstyle.qss");
    file.open(QFile::ReadOnly);
    QString styleSheet = QLatin1String(file.readAll());

    w.setStyleSheet(styleSheet);

    return a.exec();
}
