#include "login_widget.h"

#include <QApplication>
#include<QPixmap>
#include<QSplashScreen>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPixmap pixmap("start.png");   //程序等待时间
    QSplashScreen splash(pixmap);
    splash.show();
    a.processEvents();
    Login_Widget w;
    w.show();
    splash.finish(&w);
    return a.exec();
}
