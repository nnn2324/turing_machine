#include "turingmachine.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    // Создаем объект приложения
    QApplication a(argc, argv);

    // Создаем твое окно
    TuringMachine w;

    // Показываем его
    w.show();

    // Запускаем цикл обработки событий (чтобы кнопки нажимались)
    return a.exec();
}
