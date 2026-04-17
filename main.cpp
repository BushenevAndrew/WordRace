#include <QApplication>
#include "mainwindow.h"


int main(int argc, char *argv[])
{
    // Создание объекта QApplication
    QApplication app(argc, argv);

    // Установка метаданных приложения
    app.setApplicationName("Word Race Game");
    app.setOrganizationName("MyCompany");

    // Создание и отображение главного окна
    MainWindow w;
    w.show();

    // Запуск главного цикла обработки событий
    return app.exec();
}
