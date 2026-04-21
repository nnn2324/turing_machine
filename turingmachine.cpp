#include "turingmachine.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QHeaderView>

TuringMachine::TuringMachine(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TuringMachine::machineStep);

    anim = new QPropertyAnimation(this); // Анимация для каретки
    speed = 800; // Начальная скорость человека [cite: 6]
    ui->stackedWidget->setCurrentIndex(0);
}

TuringMachine::~TuringMachine() { delete ui; }

// --- 1. Настройка алфавитов и таблицы программ ---
void TuringMachine::on_btn_setAlphabets_clicked() {
    baseAlphabet = ui->lineEdit_alphaBase->text();
    extraAlphabet = ui->lineEdit_alphaExtra->text();
    QString full = baseAlphabet + extraAlphabet + "Λ"; // Λ - пустой символ [cite: 96]

    ui->tableWidget_program->setColumnCount(full.length());
    QStringList labels;
    for(QChar c : full) labels << QString(c);
    ui->tableWidget_program->setHorizontalHeaderLabels(labels);

    // Красивая таблица: растягиваем колонки на всё окно [cite: 24]
    ui->tableWidget_program->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_program->setRowCount(1);
    ui->tableWidget_program->setVerticalHeaderLabels({"q0"});

    ui->stackedWidget->setCurrentIndex(1); // Переход к машине [cite: 22]
}

// --- 2. Задание строки и проверка алфавита ---
void TuringMachine::on_btn_setString_clicked() {
    QString word = ui->lineEdit_inputWord->text();

    // Проверка: только символы из алфавита строки [cite: 36, 89]
    for(QChar c : word) {
        if(!baseAlphabet.contains(c)) {
            QMessageBox::critical(this, "Ошибка", "Символ '" + QString(c) + "' запрещен!");
            return;
        }
    }

    ui->tableWidget_tape->setColumnCount(50);
    ui->tableWidget_tape->horizontalHeader()->setDefaultSectionSize(40); // Квадратные ячейки

    for(int i=0; i<50; i++) ui->tableWidget_tape->setItem(0, i, new QTableWidgetItem("Λ"));

    headPos = 5; // Ставим строку чуть отступив от края [cite: 40]
    for(int i=0; i<word.length(); i++) {
        ui->tableWidget_tape->item(0, headPos + i)->setText(QString(word[i]));
    }

    moveCarriage(headPos);
}

// --- 3. Плавная визуализация каретки ---
void TuringMachine::moveCarriage(int pos) {
    // Получаем координаты ячейки
    QRect r = ui->tableWidget_tape->visualRect(ui->tableWidget_tape->model()->index(0, pos));

    // Центрируем каретку ▲ под ячейкой [cite: 93]
    int targetX = ui->tableWidget_tape->x() + r.x() + (r.width()/2) - (ui->label_carriage->width()/2);
    int targetY = ui->tableWidget_tape->y() + ui->tableWidget_tape->height() + 2;

    anim->setTargetObject(ui->label_carriage);
    anim->setPropertyName("pos");
    anim->setDuration(speed * 0.8); // Анимация чуть быстрее такта
    anim->setEndValue(QPoint(targetX, targetY));
    anim->start();

    // Если каретка уходит за экран, двигаем ленту [cite: 94]
    ui->tableWidget_tape->scrollTo(ui->tableWidget_tape->model()->index(0, pos));
}

// --- 4. Логика шага и проверка на '!' ---
void TuringMachine::machineStep() {
    QTableWidgetItem *tapeItem = ui->tableWidget_tape->item(0, headPos);
    QString curSym = (tapeItem) ? tapeItem->text() : "Λ";

    int col = -1;
    for(int i=0; i<ui->tableWidget_program->columnCount(); i++)
        if(ui->tableWidget_program->horizontalHeaderItem(i)->text() == curSym) col = i;

    QTableWidgetItem *cmd = ui->tableWidget_program->item(currentState, col);

    // Если пусто или есть знак '!' — стоп
    if(!cmd || cmd->text().isEmpty() || cmd->text().contains("!")) {
        on_btn_stop_clicked();
        QMessageBox::information(this, "Готово", "Машина остановилась.");
        return;
    }

    ui->tableWidget_program->selectRow(currentState); // Подсветка состояния

    QStringList p = cmd->text().split("."); // Формат: Символ.Направление.Состояние
    if(p.size() < 3) return;

    // Запись нового символа (проверка алфавита) [cite: 90]
    QString newSym = p[0];
    if(baseAlphabet.contains(newSym) || extraAlphabet.contains(newSym) || newSym == "Λ") {
        ui->tableWidget_tape->item(0, headPos)->setText(newSym);
    }

    if(p[1] == "R") headPos++; else if(p[1] == "L") headPos--;

    currentState = p[2].mid(1).toInt();
    moveCarriage(headPos);
}

void TuringMachine::on_btn_start_clicked() {
    currentState = 0;
    ui->tableWidget_program->setEnabled(false); // Блокировка по ТЗ [cite: 39]
    timer->start(speed);
}

void TuringMachine::on_btn_stop_clicked() {
    timer->stop();
    ui->tableWidget_program->setEnabled(true);
}

void TuringMachine::on_btn_speedUp_clicked() { speed = qMax(100, speed-200); if(timer->isActive()) timer->setInterval(speed); }
void TuringMachine::on_btn_speedDown_clicked() { speed += 200; if(timer->isActive()) timer->setInterval(speed); }
void TuringMachine::on_btn_addState_clicked() {
    int r = ui->tableWidget_program->rowCount();
    ui->tableWidget_program->insertRow(r);
    ui->tableWidget_program->setVerticalHeaderItem(r, new QTableWidgetItem("q"+QString::number(r)));
}
void TuringMachine::on_btn_removeState_clicked() { if(ui->tableWidget_program->rowCount() > 1) ui->tableWidget_program->removeRow(ui->tableWidget_program->rowCount()-1); }
