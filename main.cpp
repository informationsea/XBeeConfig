#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Q_INIT_RESOURCE(xbeecommandresource);
    Q_INIT_RESOURCE(icons);
    MainWindow w;
    w.show();
    return a.exec();
}
