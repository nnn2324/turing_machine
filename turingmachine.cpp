#include "turingmachine.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QHeaderView>

TuringMachine::TuringMachine(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TuringMachine::machineStep);
    anim = new QPropertyAnimation(this);
    speed = 800;

    // --- ОБЩИЙ СТИЛЬ ДЛЯ ТЕКСТА (ФРАЗЫ И ПОДПИСИ) ---
    // Устанавливаем размер 18px и черный цвет для всех QLabel
    QString labelStyle = "QLabel { "
                         "  font-size: 16px; "
                         "  font-weight: bold; "
                         "  color: black; "
                         "}";

    // Применяем стиль ко всему окну, чтобы все фразы стали большими
    this->setStyleSheet(labelStyle);

    // --- СТИЛЬ ДЛЯ ПОЛЕЙ ВВОДА (ЧТОБЫ БЫЛО ВИДНО АЛФАВИТ) ---
    QString inputStyle = "QLineEdit { "
                         "  font-size: 16px; "
                         "  color: black; "
                         "  background-color: white; "
                         "  border: 2px solid #3498db; "
                         "  border-radius: 5px; "
                         "  padding: 5px; "
                         "}";

    ui->lineEdit_alphaBase->setStyleSheet(inputStyle);
    ui->lineEdit_alphaExtra->setStyleSheet(inputStyle);
    ui->lineEdit_inputWord->setStyleSheet(inputStyle);

    // --- СТИЛЬ ДЛЯ ЛЕНТЫ (ЯРКАЯ СЕТКА) ---
    QString tapeStyle = "QTableWidget { "
                        "  gridline-color: black; "
                        "  background-color: white; "
                        "  color: black; "
                        "  font-size: 16px; "
                        "  font-weight: bold; "
                        "  border: 2px solid black; "
                        "} "
                        "QHeaderView::section { color: black; font-weight: bold; border: 1px solid black; }";

    ui->tableWidget_tape_2->setStyleSheet(tapeStyle);
    ui->tableWidget_program->setStyleSheet(tapeStyle);

    // В конструктор turingmachine.cpp
    ui->tableWidget_tape_2->setStyleSheet("QTableWidget { gridline-color: black; font-size: 22px; color: black; border: 2px solid black; }");
    ui->tableWidget_tape_2->setFixedHeight(80); // Делаем ленту ощутимо выше
    ui->tableWidget_tape_2->verticalHeader()->setDefaultSectionSize(65);
    ui->tableWidget_tape_2->horizontalHeader()->setDefaultSectionSize(60);

    ui->label->setStyleSheet("font-size: 16px; font-weight: bold; color: white;");
    ui->label_2->setStyleSheet("font-size: 16px; font-weight: bold; color: white;");

    if (ui->stackedWidget) ui->stackedWidget->setCurrentIndex(0);
}

TuringMachine::~TuringMachine() { delete ui; }

// --- НАСТРОЙКА АЛФАВИТОВ ---
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

    // Растягиваем таблицу программы на всю ширину
    ui->tableWidget_program->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_program->setRowCount(1);
    ui->tableWidget_program->setVerticalHeaderLabels({"q0"});

    ui->stackedWidget->setCurrentIndex(1);
}

// --- ЗАДАНИЕ СТРОКИ ---
void TuringMachine::on_btn_setString_clicked() {
    QString word = ui->lineEdit_inputWord->text();

    for(QChar c : word) {
        if(!baseAlphabet.contains(c)) {
            QMessageBox::critical(this, "Ошибка", "Символ '" + QString(c) + "' не из алфавита!");
            return;
        }
    }

    ui->tableWidget_tape_2->setColumnCount(50); // Теперь tableWidget_tape_2
    ui->tableWidget_tape_2->horizontalHeader()->setDefaultSectionSize(45);

    for(int i=0; i<50; i++) ui->tableWidget_tape_2->setItem(0, i, new QTableWidgetItem("Λ"));

    headPos = 5;
    for(int i=0; i<word.length(); i++) {
        ui->tableWidget_tape_2->item(0, headPos + i)->setText(QString(word[i]));
    }

    moveCarriage(headPos);
}

// --- ПЛАВНАЯ КАРЕТКА ---
void TuringMachine::moveCarriage(int pos) {
    // Координаты ячейки новой ленты
    QRect r = ui->tableWidget_tape_2->visualRect(ui->tableWidget_tape_2->model()->index(0, pos));

    int targetX = ui->tableWidget_tape_2->x() + r.x() + (r.width()/2) - (ui->label_carriage->width()/2);
    int targetY = ui->tableWidget_tape_2->y() + ui->tableWidget_tape_2->height() + 2;

    anim->setTargetObject(ui->label_carriage);
    anim->setPropertyName("pos");
    anim->setDuration(speed * 0.8);
    anim->setEndValue(QPoint(targetX, targetY));
    anim->start();

    ui->tableWidget_tape_2->scrollTo(ui->tableWidget_tape_2->model()->index(0, pos));
}

// --- ЛОГИКА ---
void TuringMachine::machineStep() {
    QTableWidgetItem *tapeItem = ui->tableWidget_tape_2->item(0, headPos);
    QString curSym = (tapeItem) ? tapeItem->text() : "Λ";

    int col = -1;
    for(int i=0; i<ui->tableWidget_program->columnCount(); i++)
        if(ui->tableWidget_program->horizontalHeaderItem(i)->text() == curSym) col = i;

    QTableWidgetItem *cmd = ui->tableWidget_program->item(currentState, col);

    if(!cmd || cmd->text().isEmpty() || cmd->text().contains("!")) {
        on_btn_stop_clicked();
        QMessageBox::information(this, "Финиш", "Машина успешно завершила работу!");
        return;
    }

    ui->tableWidget_program->selectRow(currentState);

    QStringList p = cmd->text().split(".");
    if(p.size() < 3) return;

    QString newSym = p[0];
    if(baseAlphabet.contains(newSym) || extraAlphabet.contains(newSym) || newSym == "Λ") {
        ui->tableWidget_tape_2->item(0, headPos)->setText(newSym);
    }

    if(p[1] == "R") headPos++; else if(p[1] == "L") headPos--;

    currentState = p[2].mid(1).toInt();
    moveCarriage(headPos);
}

void TuringMachine::on_btn_start_clicked() {
    currentState = 0;
    ui->tableWidget_program->setEnabled(false);
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
