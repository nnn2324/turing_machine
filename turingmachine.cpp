#include "turingmachine.h"
#include "ui_mainwindow.h"
#include <QMessageBox>

TuringMachine::TuringMachine(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // 1. Начальные настройки данных
    speed = 600;
    currentState = 0;
    headPos = 10; // Начальная позиция каретки

    // 2. Инициализация таймера для автоматического запуска
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TuringMachine::machineStep);

    // 3. Создание каретки (стрелочки ▲) программно
    lbl_carriage = new QLabel("▲", this);
    lbl_carriage->setStyleSheet("color: #3498db; font-size: 24px; font-weight: bold;");
    lbl_carriage->setFixedSize(30, 30);
    lbl_carriage->setAlignment(Qt::AlignCenter);
    lbl_carriage->raise(); // Чтобы стрелка была поверх других элементов

    // 4. Настройка анимации для плавного движения стрелки
    anim = new QPropertyAnimation(lbl_carriage, "geometry");

    // 5. Настройка внешнего вида ленты (таблицы)
    ui->tableWidget_tape_2->setRowCount(1);
    ui->tableWidget_tape_2->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tableWidget_tape_2->horizontalHeader()->hide();
    ui->tableWidget_tape_2->verticalHeader()->hide();
    ui->tableWidget_tape_2->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff); // Скрываем полосу прокрутки
    ui->tableWidget_tape_2->setSelectionMode(QAbstractItemView::NoSelection);

    // Заполняем ленту начальными 100 ячейками с символом пустоты Λ
    for(int i = 0; i < 100; i++) {
        ui->tableWidget_tape_2->insertColumn(i);
        ui->tableWidget_tape_2->setColumnWidth(i, 50);
        QTableWidgetItem *item = new QTableWidgetItem("Λ");
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget_tape_2->setItem(0, i, item);
    }

    ui->stackedWidget->setCurrentIndex(0); // Начинаем с экрана ввода алфавита
}

// ДЕСТРУКТОР (обязателен для исправления ошибки undefined reference)
TuringMachine::~TuringMachine() {
    delete ui;
}

// Блокировка/разблокировка интерфейса при работе машины
void TuringMachine::setControlsEnabled(bool enabled) {
    ui->tableWidget_program->setEnabled(enabled);
    ui->lineEdit_inputWord->setEnabled(enabled);
    ui->btn_setString->setEnabled(enabled);
    ui->btn_addState->setEnabled(enabled);
    ui->btn_removeState->setEnabled(enabled);
    ui->btn_setAlphabets_back->setEnabled(enabled);
}

// --- ЛОГИКА ВИЗУАЛИЗАЦИИ (ДВИЖЕНИЕ КАРЕТКИ И ЛЕНТЫ) ---

void TuringMachine::moveCarriage(int pos) {
    // Выделяем текущую ячейку
    ui->tableWidget_tape_2->setCurrentCell(0, pos);
    QTableWidgetItem *item = ui->tableWidget_tape_2->item(0, pos);
    if (!item) return;

    // Получаем координаты ячейки в окне
    QRect cellRect = ui->tableWidget_tape_2->visualItemRect(item);
    int viewportWidth = ui->tableWidget_tape_2->viewport()->width();
    int margin = 150; // Расстояние от края, после которого лента начнет скроллиться

    // Если каретка подошла к краю экрана — центрируем ленту на этой ячейке
    if (cellRect.left() < margin || cellRect.right() > viewportWidth - margin) {
        ui->tableWidget_tape_2->scrollToItem(item, QAbstractItemView::PositionAtCenter);
        // Обновляем координаты после сдвига ленты
        cellRect = ui->tableWidget_tape_2->visualItemRect(item);
    }

    // Вычисляем финальную позицию стрелочки под ячейкой
    int globalX = cellRect.center().x() + ui->tableWidget_tape_2->pos().x() - (lbl_carriage->width() / 2);
    int globalY = ui->tableWidget_tape_2->pos().y() + ui->tableWidget_tape_2->height() - 5;

    // Запуск плавной анимации перемещения стрелки
    anim->stop();
    anim->setDuration(speed * 0.8); // Анимация чуть быстрее, чем такт машины
    anim->setStartValue(lbl_carriage->geometry());
    anim->setEndValue(QRect(globalX, globalY, lbl_carriage->width(), lbl_carriage->height()));
    anim->setEasingCurve(QEasingCurve::OutQuad);
    anim->start();
}

// --- ОСНОВНАЯ ЛОГИКА ВЫПОЛНЕНИЯ ШАГА ---

void TuringMachine::machineStep() {
    // Проверка границ
    if (headPos < 0 || headPos >= ui->tableWidget_tape_2->columnCount()) return;

    // 1. Читаем текущий символ
    QString sym = ui->tableWidget_tape_2->item(0, headPos)->text().trimmed();

    // 2. Ищем колонку этого символа в таблице программы
    int col = -1;
    for(int i = 0; i < ui->tableWidget_program->columnCount(); i++) {
        if(ui->tableWidget_program->horizontalHeaderItem(i)->text() == sym) {
            col = i;
            break;
        }
    }

    if(col == -1) {
        on_btn_stop_clicked();
        QMessageBox::warning(this, "Ошибка", "Символ '" + sym + "' не найден в таблице!");
        return;
    }

    // 3. Получаем текст команды
    QTableWidgetItem *cmdItem = ui->tableWidget_program->item(currentState, col);
    if(!cmdItem || cmdItem->text().trimmed().isEmpty()) {
        on_btn_stop_clicked();
        QMessageBox::information(this, "Стоп", "Пустая команда. Машина остановлена.");
        return;
    }

    QString cmdText = cmdItem->text().trimmed();

    // Проверка на символ остановки
    if(cmdText.contains("!")) {
        on_btn_stop_clicked();
        QMessageBox::information(this, "Конец", "Программа завершена успешно!");
        return;
    }

    // 4. ПАРСИНГ (поддержка: a.R.q1, a.R, R.q1, R)
    QStringList parts = cmdText.split(".");
    QString writeSym = sym;
    QString dir = "";
    int nextState = currentState;

    if (parts.size() == 3) { // a.R.q1
        writeSym = parts[0];
        dir = parts[1].toUpper();
        nextState = parts[2].mid(1).toInt();
    } else if (parts.size() == 2) {
        if (parts[1].startsWith("q")) { // R.q1
            dir = parts[0].toUpper();
            nextState = parts[1].mid(1).toInt();
        } else { // a.R
            writeSym = parts[0];
            dir = parts[1].toUpper();
        }
    } else { // R или L
        dir = parts[0].toUpper();
    }

    // 5. ПРИМЕНЕНИЕ ДЕЙСТВИЙ
    ui->tableWidget_tape_2->item(0, headPos)->setText(writeSym);

    if(dir == "R") headPos++;
    else if(dir == "L") headPos--;

    // 6. ПРОВЕРКА ГРАНИЦ И АВТО-РАСШИРЕНИЕ
    if(headPos < 0) {
        on_btn_stop_clicked();
        QMessageBox::critical(this, "Ошибка", "Выход за левую границу ленты!");
        return;
    }
    if(headPos >= ui->tableWidget_tape_2->columnCount()) {
        int newCol = ui->tableWidget_tape_2->columnCount();
        ui->tableWidget_tape_2->insertColumn(newCol);
        ui->tableWidget_tape_2->setColumnWidth(newCol, 50);
        ui->tableWidget_tape_2->setItem(0, newCol, new QTableWidgetItem("Λ"));
        ui->tableWidget_tape_2->item(0, newCol)->setTextAlignment(Qt::AlignCenter);
    }

    // Переходим в новое состояние
    currentState = nextState;
    ui->tableWidget_program->selectRow(currentState);

    // Обновляем визуальное положение
    moveCarriage(headPos);
}

// --- ОБРАБОТЧИКИ КНОПОК ---

void TuringMachine::on_btn_setAlphabets_clicked() {
    baseAlphabet = ui->lineEdit_alphaBase->text().trimmed();
    QString extra = ui->lineEdit_alphaExtra->text().trimmed();
    if(baseAlphabet.isEmpty()) return;

    QString full = baseAlphabet + extra + "Λ";
    ui->tableWidget_program->setColumnCount(full.length());

    QStringList labels;
    for(QChar c : full) labels << QString(c);
    ui->tableWidget_program->setHorizontalHeaderLabels(labels);

    if(ui->tableWidget_program->rowCount() == 0) {
        ui->tableWidget_program->setRowCount(1);
        ui->tableWidget_program->setVerticalHeaderLabels(QStringList() << "q0");
    }

    ui->stackedWidget->setCurrentIndex(1);
}

void TuringMachine::on_btn_setString_clicked() {
    QString word = ui->lineEdit_inputWord->text().trimmed();

    // Чистим ленту
    for(int i = 0; i < ui->tableWidget_tape_2->columnCount(); i++)
        ui->tableWidget_tape_2->item(0, i)->setText("Λ");

    // Записываем слово (начиная с 10-й ячейки для отступа)
    for(int i = 0; i < word.length() && (i + 10) < ui->tableWidget_tape_2->columnCount(); i++) {
        ui->tableWidget_tape_2->item(0, 10 + i)->setText(QString(word[i]));
    }

    headPos = 10;
    currentState = 0;
    moveCarriage(headPos);
    ui->tableWidget_program->selectRow(0);
}

void TuringMachine::on_btn_start_clicked() {
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
    machineStep();
}

void TuringMachine::on_btn_speedUp_clicked() {
    speed = qMax(50, speed - 100);
    if(timer->isActive()) timer->start(speed);
}

void TuringMachine::on_btn_speedDown_clicked() {
    speed += 100;
    if(timer->isActive()) timer->start(speed);
}

void TuringMachine::on_btn_addState_clicked() {
    int r = ui->tableWidget_program->rowCount();
    ui->tableWidget_program->insertRow(r);
    ui->tableWidget_program->setVerticalHeaderItem(r, new QTableWidgetItem("q" + QString::number(r)));
}

void TuringMachine::on_btn_removeState_clicked() {
    if(ui->tableWidget_program->rowCount() > 1)
        ui->tableWidget_program->removeRow(ui->tableWidget_program->rowCount() - 1);
}

void TuringMachine::on_btn_setAlphabets_back_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
}
