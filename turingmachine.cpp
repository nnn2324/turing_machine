#include "turingmachine.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

TuringMachine::TuringMachine(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TuringMachine::machineStep);
    anim = new QPropertyAnimation(this);
    speed = 600;
    ui->stackedWidget->setCurrentIndex(0);
}

TuringMachine::~TuringMachine() { delete ui; }

// Блокировка элементов во время работы (ТЗ 2.3)
void TuringMachine::setControlsEnabled(bool enabled) {
    ui->tableWidget_program->setEnabled(enabled);
    ui->lineEdit_inputWord->setEnabled(enabled);
    ui->btn_setString->setEnabled(enabled);
    ui->btn_addState->setEnabled(enabled);
    ui->btn_removeState->setEnabled(enabled);
    ui->btn_step->setEnabled(enabled);
}

void TuringMachine::on_btn_setAlphabets_clicked() {
    baseAlphabet = ui->lineEdit_alphaBase->text();
    extraAlphabet = ui->lineEdit_alphaExtra->text();

    if(baseAlphabet.isEmpty()) {
        QMessageBox::warning(this, "Внимание", "Введите основной алфавит!");
        return;
    }

    QString full = baseAlphabet + extraAlphabet + "Λ";
    ui->tableWidget_program->setColumnCount(full.length());
    QStringList labels;
    for(QChar c : full) labels << QString(c);
    ui->tableWidget_program->setHorizontalHeaderLabels(labels);
    ui->tableWidget_program->setRowCount(1);
    ui->tableWidget_program->setVerticalHeaderLabels({"q0"});
    ui->stackedWidget->setCurrentIndex(1);
}

// ИСПРАВЛЕНО: Теперь лента инициализируется правильно и не вылетает
void TuringMachine::on_btn_setString_clicked() {
    QString word = ui->lineEdit_inputWord->text();

    // Проверка алфавита (ТЗ 4.1)
    for(QChar c : word) {
        if(!baseAlphabet.contains(c)) {
            QMessageBox::critical(this, "Ошибка", "Символ '" + QString(c) + "' не входит в алфавит!");
            return;
        }
    }

    // Настройка таблицы ленты
    ui->tableWidget_tape_2->setRowCount(1);
    ui->tableWidget_tape_2->setColumnCount(100); // 100 ячеек для запаса
    ui->tableWidget_tape_2->verticalHeader()->hide();

    for(int i=0; i<100; i++) {
        ui->tableWidget_tape_2->setColumnWidth(i, 40);
        QTableWidgetItem *item = new QTableWidgetItem("Λ");
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget_tape_2->setItem(0, i, item);
    }

    headPos = 15; // Позиция головы
    for(int i=0; i<word.length(); i++) {
        ui->tableWidget_tape_2->item(0, headPos + i)->setText(QString(word[i]));
    }

    currentState = 0;
    moveCarriage(headPos);
}

void TuringMachine::moveCarriage(int pos) {
    // Получаем координаты ячейки
    QRect r = ui->tableWidget_tape_2->visualRect(ui->tableWidget_tape_2->model()->index(0, pos));
    if (!r.isValid()) return;

    int targetX = ui->tableWidget_tape_2->x() + r.x() + (r.width()/2) - (ui->label_carriage->width()/2);

    anim->stop();
    anim->setTargetObject(ui->label_carriage);
    anim->setPropertyName("pos");
    anim->setDuration(speed * 0.7);
    anim->setEndValue(QPoint(targetX, ui->label_carriage->y()));
    anim->start();

    ui->tableWidget_tape_2->scrollTo(ui->tableWidget_tape_2->model()->index(0, pos), QAbstractItemView::PositionAtCenter);
}

void TuringMachine::machineStep() {
    QTableWidgetItem *cell = ui->tableWidget_tape_2->item(0, headPos);
    QString sym = (cell) ? cell->text() : "Λ";

    int col = -1;
    for(int i=0; i<ui->tableWidget_program->columnCount(); i++)
        if(ui->tableWidget_program->horizontalHeaderItem(i)->text() == sym) col = i;

    QTableWidgetItem *cmd = ui->tableWidget_program->item(currentState, col);

    if(!cmd || cmd->text().isEmpty() || cmd->text().contains("!")) {
        on_btn_stop_clicked();
        QMessageBox::information(this, "Стоп", "Программа завершена.");
        return;
    }

    // Парсинг команды Символ.Направление.Состояние (напр: a.R.q1)
    QStringList p = cmd->text().split(".");
    if(p.size() < 3) {
        on_btn_stop_clicked();
        QMessageBox::warning(this, "Ошибка", "Неверный формат команды!");
        return;
    }

    ui->tableWidget_tape_2->item(0, headPos)->setText(p[0]);
    if(p[1].toUpper() == "R") headPos++;
    else if(p[1].toUpper() == "L") headPos--;

    currentState = p[2].mid(1).toInt();
    ui->tableWidget_program->selectRow(currentState);
    moveCarriage(headPos);
}

void TuringMachine::on_btn_start_clicked() {
    // Проверка на символ остановки (ТЗ 4.3)
    bool hasStop = false;
    for(int r=0; r<ui->tableWidget_program->rowCount(); r++)
        for(int c=0; c<ui->tableWidget_program->columnCount(); c++)
            if(ui->tableWidget_program->item(r,c) && ui->tableWidget_program->item(r,c)->text().contains("!")) hasStop = true;

    if(!hasStop) {
        QMessageBox::warning(this, "Ошибка", "Нет команды завершения (!)");
        return;
    }

    setControlsEnabled(false);
    timer->start(speed);
}

void TuringMachine::on_btn_stop_clicked() {
    timer->stop();
    setControlsEnabled(true);
}

void TuringMachine::on_btn_reset_clicked() {
    on_btn_stop_clicked();
    on_btn_setString_clicked();
}

void TuringMachine::on_btn_step_clicked() {
    if(!timer->isActive()) machineStep();
}

void TuringMachine::on_btn_speedUp_clicked() { speed = qMax(50, speed-150); if(timer->isActive()) timer->setInterval(speed); }
void TuringMachine::on_btn_speedDown_clicked() { speed += 150; if(timer->isActive()) timer->setInterval(speed); }

void TuringMachine::on_btn_addState_clicked() {
    int r = ui->tableWidget_program->rowCount();
    ui->tableWidget_program->insertRow(r);
    ui->tableWidget_program->setVerticalHeaderItem(r, new QTableWidgetItem("q"+QString::number(r)));
}

void TuringMachine::on_btn_removeState_clicked() {
    if(ui->tableWidget_program->rowCount() > 1) ui->tableWidget_program->removeRow(ui->tableWidget_program->rowCount()-1);
}

void TuringMachine::on_btn_setAlphabets_back_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
}
