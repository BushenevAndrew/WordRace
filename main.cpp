#include <QApplication>
#include "mainwindow.h"

/**
 * @brief Главная функция приложения
 * Создает экземпляр QApplication, инициализирует главное окно и запускает цикл обработки событий
 *
 * @param argc - количество аргументов командной строки
 * @param argv - массив аргументов командной строки
 * @return int - код возврата приложения
 */
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
