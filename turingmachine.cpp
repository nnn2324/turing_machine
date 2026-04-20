#include "turing_machine.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug> // Для вывода сообщений в консоль

TuringMachine::TuringMachine(QWidget *parent) : QWidget(parent) {
    // 1. СОЗДАЕМ КНОПКИ
    btnSetAlphabets = new QPushButton("Задать алфавиты");
    btnSetString = new QPushButton("Задать строку");
    btnRun = new QPushButton("Запустить машину");
    btnStop = new QPushButton("Остановить машину");
    btnStep = new QPushButton("Выполнить один шаг");
    btnReset = new QPushButton("Сбросить программу");
    btnSpeedUp = new QPushButton("Ускорить (+)");
    btnSlowDown = new QPushButton("Замедлить (-)");

    // 2. РАЗМЕЩАЕМ КНОПКИ (Компоновка)
    // Горизонтальный слой для кнопок управления воспроизведением
    QHBoxLayout *playLayout = new QHBoxLayout;
    playLayout->addWidget(btnRun);
    playLayout->addWidget(btnStep);
    playLayout->addWidget(btnStop);
    playLayout->addWidget(btnReset);

    // Горизонтальный слой для скорости
    QHBoxLayout *speedLayout = new QHBoxLayout;
    speedLayout->addWidget(btnSlowDown);
    speedLayout->addWidget(btnSpeedUp);

    // Главный вертикальный слой, куда мы сложим всё вместе
    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(btnSetAlphabets);
    mainLayout->addWidget(btnSetString);
    mainLayout->addLayout(playLayout); // Добавляем слой со слоем
    mainLayout->addLayout(speedLayout);

    setLayout(mainLayout); // Устанавливаем главную компоновку окну

    // 3. ПОДКЛЮЧАЕМ КНОПКИ К ФУНКЦИЯМ
    connect(btnSetAlphabets, &QPushButton::clicked, this, &TuringMachine::onSetAlphabetsClicked);
    connect(btnSetString, &QPushButton::clicked, this, &TuringMachine::onSetStringClicked);
    connect(btnRun, &QPushButton::clicked, this, &TuringMachine::onRunClicked);
    connect(btnStop, &QPushButton::clicked, this, &TuringMachine::onStopClicked);
    connect(btnStep, &QPushButton::clicked, this, &TuringMachine::onStepClicked);
    connect(btnReset, &QPushButton::clicked, this, &TuringMachine::onResetClicked);
    connect(btnSpeedUp, &QPushButton::clicked, this, &TuringMachine::onSpeedUpClicked);
    connect(btnSlowDown, &QPushButton::clicked, this, &TuringMachine::onSlowDownClicked);
}

// 4. ОПИСЫВАЕМ, ЧТО ДЕЛАЮТ КНОПКИ (пока просто выводим текст в консоль)
void TuringMachine::onSetAlphabetsClicked() {
    qDebug() << "Нажата кнопка 'Задать алфавиты'. Тут будем строить таблицу!";
}

void TuringMachine::onSetStringClicked() {
    qDebug() << "Нажата кнопка 'Задать строку'.";
}

void TuringMachine::onRunClicked() {
    qDebug() << "Машина запущена!";
    // В ТЗ сказано: при запуске нужно блокировать изменения [cite: 185]
    btnSetAlphabets->setEnabled(false);
}

void TuringMachine::onStopClicked() {
    qDebug() << "Машина остановлена.";
    btnSetAlphabets->setEnabled(true); // Разблокируем обратно
}

void TuringMachine::onStepClicked() { qDebug() << "Шаг..."; }
void TuringMachine::onResetClicked() { qDebug() << "Сброс!"; }
void TuringMachine::onSpeedUpClicked() { qDebug() << "Быстрее!"; }
void TuringMachine::onSlowDownClicked() { qDebug() << "Медленнее..."; }
