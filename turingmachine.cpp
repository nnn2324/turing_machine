#include "turingmachine.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QHeaderView>

TuringMachine::TuringMachine(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TuringMachine::machineStep);
    anim = new QPropertyAnimation(this, "pos");
    speed = 800; // "Скорость человека" [cite: 6]
    ui->stackedWidget->setCurrentIndex(0);
}

TuringMachine::~TuringMachine() { delete ui; }

// --- НАСТРОЙКА АЛФАВИТОВ ---
void TuringMachine::on_btn_setAlphabets_clicked() {
    baseAlphabet = ui->lineEdit_alphaBase->text();
    extraAlphabet = ui->lineEdit_alphaExtra->text();
    QString full = baseAlphabet + extraAlphabet + "Λ"; // Λ - пустой символ [cite: 96]

    ui->tableWidget_program->setColumnCount(full.length());
    QStringList labels;
    for(QChar c : full) labels << QString(c);
    ui->tableWidget_program->setHorizontalHeaderLabels(labels);

    // Красивое растягивание таблицы
    ui->tableWidget_program->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_program->setRowCount(1);
    ui->tableWidget_program->setVerticalHeaderLabels({"q0"});

    ui->stackedWidget->setCurrentIndex(1);
}

// --- ЗАДАНИЕ СТРОКИ [cite: 34] ---
void TuringMachine::on_btn_setString_clicked() {
    QString word = ui->lineEdit_inputWord->text();

    // Проверка: символы только из алфавита строки [cite: 89]
    for(QChar c : word) {
        if(!baseAlphabet.contains(c)) {
            QMessageBox::critical(this, "Ошибка", "Символ '" + QString(c) + "' не входит в алфавит строки!");
            return;
        }
    }

    ui->tableWidget_tape->setColumnCount(50); // Запас ленты
    ui->tableWidget_tape->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableWidget_tape->horizontalHeader()->setDefaultSectionSize(40);

    for(int i=0; i<50; i++) ui->tableWidget_tape->setItem(0, i, new QTableWidgetItem("Λ"));

    // Помещаем строку на ленту (начинаем с 1-й ячейки для удобства)
    headPos = 5;
    for(int i=0; i<word.length(); i++) {
        ui->tableWidget_tape->item(0, headPos + i)->setText(QString(word[i]));
    }

    moveCarriage(headPos); // Каретка под первый символ
}

// --- ПЛАВНАЯ КАРЕТКА  ---
void TuringMachine::moveCarriage(int pos) {
    // Получаем координаты ячейки относительно таблицы
    QRect r = ui->tableWidget_tape->visualRect(ui->tableWidget_tape->model()->index(0, pos));

    // Координаты внутри окна (с учетом расположения таблицы)
    int targetX = ui->tableWidget_tape->x() + r.x() + (r.width()/2) - (ui->label_carriage->width()/2);
    int targetY = ui->tableWidget_tape->y() + ui->tableWidget_tape->height() + 5;

    anim->setTargetObject(ui->label_carriage);
    anim->setDuration(speed * 0.7); // Анимация чуть быстрее такта
    anim->setStartValue(ui->label_carriage->pos());
    anim->setEndValue(QPoint(targetX, targetY));
    anim->start();

    // Автопрокрутка ленты, если каретка уходит за край [cite: 94]
    ui->tableWidget_tape->scrollTo(ui->tableWidget_tape->model()->index(0, pos));
}

// --- ЗАПУСК И ПРОВЕРКИ ---
void TuringMachine::on_btn_start_clicked() {
    // Проверка на наличие хотя бы одного символа остановки '!'
    bool hasHalt = false;
    for(int r=0; r<ui->tableWidget_program->rowCount(); r++) {
        for(int c=0; c<ui->tableWidget_program->columnCount(); c++) {
            QTableWidgetItem *it = ui->tableWidget_program->item(r, c);
            if(it && it->text().contains("!")) hasHalt = true;
        }
    }

    if(!hasHalt) {
        QMessageBox::warning(this, "Ошибка", "В таблице нет ни одного состояния остановки (!)");
        return;
    }

    currentState = 0;
    ui->tableWidget_program->setEnabled(false); // Блокировка при запуске [cite: 39]
    timer->start(speed);
}

// --- ШАГ МАШИНЫ ---
void TuringMachine::machineStep() {
    QTableWidgetItem *tapeItem = ui->tableWidget_tape->item(0, headPos);
    QString curSym = (tapeItem) ? tapeItem->text() : "Λ";

    int col = -1;
    for(int i=0; i<ui->tableWidget_program->columnCount(); i++)
        if(ui->tableWidget_program->horizontalHeaderItem(i)->text() == curSym) col = i;

    QTableWidgetItem *cmd = ui->tableWidget_program->item(currentState, col);

    // Если ячейка пуста или содержит '!', останавливаемся
    if(!cmd || cmd->text().isEmpty() || cmd->text().contains("!")) {
        on_btn_stop_clicked();
        QMessageBox::information(this, "Стоп", "Машина завершила работу.");
        return;
    }

    // Подсветка текущего состояния [cite: 103]
    ui->tableWidget_program->selectRow(currentState);

    QStringList p = cmd->text().split("."); // Формат: Символ.Направление.Состояние
    if(p.size() < 3) return;

    // 1. Запись символа (проверка алфавита )
    QString newSym = p[0];
    if(!baseAlphabet.contains(newSym) && !extraAlphabet.contains(newSym) && newSym != "Λ") {
        timer->stop();
        QMessageBox::critical(this, "Ошибка", "Символ '" + newSym + "' не входит в алфавиты!");
        return;
    }
    ui->tableWidget_tape->item(0, headPos)->setText(newSym);

    // 2. Движение
    if(p[1] == "R") headPos++; else if(p[1] == "L") headPos--;

    // 3. Новое состояние
    currentState = p[2].mid(1).toInt();

    moveCarriage(headPos);
}

void TuringMachine::on_btn_stop_clicked() {
    timer->stop();
    ui->tableWidget_program->setEnabled(true);
}
