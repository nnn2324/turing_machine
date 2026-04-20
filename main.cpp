#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Создаем главное окно
    QWidget window;
    window.setWindowTitle("Эмулятор Машины Тьюринга");

    // Создаем элементы
    QLabel *title = new QLabel("Введите алфавиты для начала работы");
    QLineEdit *alphabetInput = new QLineEdit();
    alphabetInput->setPlaceholderText("Например: abc");

    QPushButton *nextBtn = new QPushButton("Далее");

    // Расставляем их вертикально
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(title);
    layout->addWidget(alphabetInput);
    layout->addWidget(nextBtn);

    window.setLayout(layout);

    // Оживляем кнопку: при нажатии покажем сообщение
    QObject::connect(nextBtn, &QPushButton::clicked, [&]() {
        QString text = alphabetInput->text();
        QMessageBox::information(&window, "Готово!", "Алфавит принят: " + text);
    });

    window.resize(300, 200);
    window.show();

    return app.exec();
}
