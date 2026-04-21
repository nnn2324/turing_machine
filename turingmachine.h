#ifndef TURINGMACHINE_H
#define TURINGMACHINE_H

#include <QMainWindow>
#include <QTimer>
#include <QPropertyAnimation>
#include <QTableWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class TuringMachine : public QMainWindow {
    Q_OBJECT

public:
    TuringMachine(QWidget *parent = nullptr);
    ~TuringMachine();

private slots:
    void on_btn_setAlphabets_clicked();
    void on_btn_setString_clicked();
    void on_btn_start_clicked();
    void on_btn_stop_clicked();
    void on_btn_reset_clicked();      // Кнопка Сброс
    void on_btn_step_clicked();       // Кнопка Шаг
    void on_btn_speedUp_clicked();
    void on_btn_speedDown_clicked();
    void on_btn_addState_clicked();
    void on_btn_removeState_clicked();
    void on_btn_setAlphabets_back_clicked();
    void machineStep();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QPropertyAnimation *anim;

    QString baseAlphabet;
    QString extraAlphabet;
    int currentState;
    int headPos;
    int speed;

    void moveCarriage(int pos);
    void setControlsEnabled(bool enabled); // Блокировка интерфейса по ТЗ
};
#endif
