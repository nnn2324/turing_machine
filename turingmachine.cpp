#include "turingmachine.h"

TuringMachine::TuringMachine(QWidget *parent) : QWidget(parent) {
    globalLayout = new QVBoxLayout(this);

    // --- СОЗДАЕМ ОКНО 1 (Настройка) ---
    setupWidget = new QWidget();
    QVBoxLayout *setupLayout = new QVBoxLayout(setupWidget);

    setupLayout->addWidget(new QLabel("Алфавит строки:"));
    lineAlphabet = new QLineEdit("abc");
    setupLayout->addWidget(lineAlphabet);

    setupLayout->addWidget(new QLabel("Алфавит доп. символов:"));
    extraAlphabet = new QLineEdit("#ABC");
    setupLayout->addWidget(extraAlphabet);

    QPushButton *btnApply = new QPushButton("Задать алфавиты");
    setupLayout->addWidget(btnApply);

    // --- СОЗДАЕМ ОКНО 2 (Эмулятор) ---
    mainWidget = new QWidget();
    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);

    mainLayout->addWidget(new QLabel("Строка на ленте:"));
    tapeInput = new QLineEdit();
    mainLayout->addWidget(tapeInput);

    // Таблица программы
    programTable = new QTableWidget(3, 5); // Для примера 3 состояния, 5 символов
    mainLayout->addWidget(programTable);

    // Кнопки управления (в ряд)
    QHBoxLayout *controls = new QHBoxLayout();
    controls->addWidget(new QPushButton("▶ Запуск"));
    controls->addWidget(new QPushButton("⏸ Стоп"));
    controls->addWidget(new QPushButton("Step"));
    mainLayout->addLayout(controls);

    QPushButton *btnBack = new QPushButton("Изменить алфавиты");
    mainLayout->addWidget(btnBack);

    // Добавляем оба виджета в главный слой
    globalLayout->addWidget(setupWidget);
    globalLayout->addWidget(mainWidget);

    mainWidget->hide(); // Сначала показываем только Окно 1

    // --- СОЕДИНЯЕМ КНОПКИ ---
    connect(btnApply, &QPushButton::clicked, this, &TuringMachine::onSetAlphabets);
    connect(btnBack, &QPushButton::clicked, this, &TuringMachine::onReset);
}

void TuringMachine::onSetAlphabets() {
    setupWidget->hide();
    mainWidget->show();
}

void TuringMachine::onReset() {
    mainWidget->hide();
    setupWidget->show();
}
