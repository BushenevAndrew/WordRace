#include <QApplication>
#include "mainwindow.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Word Race Game");
    app.setOrganizationName("MyCompany");

    MainWindow w;
    w.show();

    return app.exec();
}
