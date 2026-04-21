#include "turingmachine.h"
#include "ui_mainwindow.h"

TuringMachine::TuringMachine(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TuringMachine::machineStep);

    speed = 600;
    ui->stackedWidget->setCurrentIndex(0); // Начинаем с окна ввода алфавитов

    // Настройка таблицы ленты
    ui->tableWidget_tape_2->setRowCount(1);
    ui->tableWidget_tape_2->verticalHeader()->hide();
    ui->tableWidget_tape_2->horizontalHeader()->hide();
    ui->tableWidget_tape_2->setSelectionMode(QAbstractItemView::NoSelection);

    // Инициализация пустой ленты (100 ячеек)
    for(int i = 0; i < 100; i++) {
        ui->tableWidget_tape_2->insertColumn(i);
        ui->tableWidget_tape_2->setColumnWidth(i, 50);
        QTableWidgetItem *item = new QTableWidgetItem("Λ");
        item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget_tape_2->setItem(0, i, item);
    }
}

TuringMachine::~TuringMachine() { delete ui; }

// --- Управление интерфейсом ---

void TuringMachine::setControlsEnabled(bool enabled) {
    ui->tableWidget_program->setEnabled(enabled);
    ui->lineEdit_inputWord->setEnabled(enabled);
    ui->btn_setString->setEnabled(enabled);
    ui->btn_addState->setEnabled(enabled);
    ui->btn_removeState->setEnabled(enabled);
    ui->btn_setAlphabets_back->setEnabled(enabled);
}

void TuringMachine::on_btn_setAlphabets_clicked() {
    baseAlphabet = ui->lineEdit_alphaBase->text().trimmed();
    if(baseAlphabet.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите основной алфавит!");
        return;
    }

    // Собираем полный алфавит: основной + доп + пустой символ
    QString full = baseAlphabet + ui->lineEdit_alphaExtra->text().trimmed() + "Λ";

    ui->tableWidget_program->setColumnCount(full.length());
    QStringList labels;
    for(QChar c : full) labels << QString(c);
    ui->tableWidget_program->setHorizontalHeaderLabels(labels);

    if(ui->tableWidget_program->rowCount() == 0) {
        ui->tableWidget_program->setRowCount(1);
        ui->tableWidget_program->setVerticalHeaderLabels(QStringList() << "q0");
    }

    ui->stackedWidget->setCurrentIndex(1); // Переходим к окну эмуляции
}

void TuringMachine::on_btn_setAlphabets_back_clicked() {
    ui->stackedWidget->setCurrentIndex(0);
}

void TuringMachine::on_btn_setString_clicked() {
    QString word = ui->lineEdit_inputWord->text().trimmed();

    // Очистка ленты
    for(int i = 0; i < ui->tableWidget_tape_2->columnCount(); i++) {
        ui->tableWidget_tape_2->item(0, i)->setText("Λ");
    }

    // Запись слова в центр ленты (примерно)
    int startIdx = 10;
    for(int i = 0; i < word.length() && (startIdx + i) < ui->tableWidget_tape_2->columnCount(); i++) {
        ui->tableWidget_tape_2->item(0, startIdx + i)->setText(QString(word[i]));
    }

    headPos = startIdx;
    currentState = 0;
    moveCarriage(headPos);
    ui->tableWidget_program->selectRow(0);
}

// --- Логика машины ---

void TuringMachine::machineStep() {
    // 1. Считываем символ под кареткой
    QTableWidgetItem *tapeItem = ui->tableWidget_tape_2->item(0, headPos);
    QString sym = tapeItem->text().trimmed();

    // 2. Ищем столбец символа в таблице программ
    int col = -1;
    for(int i = 0; i < ui->tableWidget_program->columnCount(); i++) {
        if(ui->tableWidget_program->horizontalHeaderItem(i)->text() == sym) {
            col = i;
            break;
        }
    }

    if(col == -1) {
        on_btn_stop_clicked();
        QMessageBox::critical(this, "Ошибка", "Символ '" + sym + "' не найден в алфавите таблицы!");
        return;
    }

    // 3. Получаем команду
    QTableWidgetItem *cmdItem = ui->tableWidget_program->item(currentState, col);
    if(!cmdItem || cmdItem->text().trimmed().isEmpty()) {
        on_btn_stop_clicked();
        QMessageBox::information(this, "Стоп", "Пустая ячейка. Машина остановлена.");
        return;
    }

    QString cmdText = cmdItem->text().trimmed();
    if(cmdText == "!") {
        on_btn_stop_clicked();
        QMessageBox::information(this, "Конец", "Программа завершена успешно!");
        return;
    }

    // 4. Парсинг команды (поддержка форматов: R, L, a.R, R.q1, a.R.q1)
    QStringList p = cmdText.split(".");
    QString writeSym = sym;
    QString dir = "";
    int nextState = currentState;

    if (p.size() == 3) { // a.R.q1
        writeSym = p[0]; dir = p[1].toUpper(); nextState = p[2].mid(1).toInt();
    } else if (p.size() == 2) {
        if (p[1].startsWith("q")) { // R.q1
            dir = p[0].toUpper(); nextState = p[1].mid(1).toInt();
        } else { // a.R
            writeSym = p[0]; dir = p[1].toUpper();
        }
    } else { // R или L
        dir = p[0].toUpper();
    }

    // 5. Выполнение
    tapeItem->setText(writeSym);

    if(dir == "R") headPos++;
    else if(dir == "L") headPos--;

    // Проверка границ ленты
    if(headPos < 0) {
        on_btn_stop_clicked();
        QMessageBox::warning(this, "Ошибка", "Выход за левую границу ленты!");
        return;
    }
    if(headPos >= ui->tableWidget_tape_2->columnCount()) {
        // Добавляем новую ячейку справа
        int newCol = ui->tableWidget_tape_2->columnCount();
        ui->tableWidget_tape_2->insertColumn(newCol);
        ui->tableWidget_tape_2->setColumnWidth(newCol, 50);
        ui->tableWidget_tape_2->setItem(0, newCol, new QTableWidgetItem("Λ"));
    }

    currentState = nextState;
    ui->tableWidget_program->selectRow(currentState);
    moveCarriage(headPos);
}

void TuringMachine::moveCarriage(int pos) {
    ui->tableWidget_tape_2->setCurrentCell(0, pos);
    ui->tableWidget_tape_2->scrollToItem(ui->tableWidget_tape_2->item(0, pos), QAbstractItemView::PositionAtCenter);
}

// --- Кнопки управления ---

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
    speed = qMax(50, speed - 150);
    if(timer->isActive()) timer->start(speed);
}

void TuringMachine::on_btn_speedDown_clicked() {
    speed += 150;
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
