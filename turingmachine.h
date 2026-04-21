#ifndef TURINGMACHINE_H
#define TURINGMACHINE_H

#include <QMainWindow>
#include <QTimer>
#include <QPropertyAnimation>
#include <QTableWidgetItem>
#include <QLabel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class TuringMachine : public QMainWindow {
    Q_OBJECT

public:
    TuringMachine(QWidget *parent = nullptr);
    ~TuringMachine();

private slots:
    // Переходы между экранами и настройка алфавита
    void on_btn_setAlphabets_clicked();
    void on_btn_setAlphabets_back_clicked();

    // Работа с входной строкой
    void on_btn_setString_clicked();

    // Управление выполнением программы
    void on_btn_start_clicked();
    void on_btn_stop_clicked();
    void on_btn_reset_clicked();
    void on_btn_step_clicked();

    // Настройка скорости и таблицы состояний
    void on_btn_speedUp_clicked();
    void on_btn_speedDown_clicked();
    void on_btn_addState_clicked();
    void on_btn_removeState_clicked();

    // Основной цикл машины (вызывается таймером)
    void machineStep();

private:
    Ui::MainWindow *ui;

    // Системные объекты для работы
    QTimer *timer;               // Таймер для автоматического режима
    QPropertyAnimation *anim;    // Анимация для движения каретки
    QLabel *lbl_carriage;        // Виджет-стрелочка (каретка)

    // Состояние машины
    QString baseAlphabet;        // Основной алфавит
    int currentState;            // Номер текущего состояния (q0, q1...)
    int headPos;                 // Индекс текущей ячейки на ленте
    int speed;                   // Задержка между шагами в мс

    // Вспомогательные функции
    void moveCarriage(int pos);          // Логика перемещения стрелки и скролла ленты
    void setControlsEnabled(bool enabled); // Блокировка кнопок при запуске
};

#endif // TURINGMACHINE_H
