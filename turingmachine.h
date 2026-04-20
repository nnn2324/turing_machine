#ifndef TURING_MACHINE_H
#define TURING_MACHINE_H

#include <QWidget>
#include <QPushButton>

// Наш главный класс наследуется от QWidget
class TuringMachine : public QWidget {
    Q_OBJECT // Обязательный макрос для работы сигналов и слотов

public:
    explicit TuringMachine(QWidget *parent = nullptr);

private slots:
    // Это слоты — функции, которые будут выполняться при нажатии кнопок
    void onSetAlphabetsClicked();
    void onSetStringClicked();
    void onRunClicked();
    void onStopClicked();
    void onStepClicked();
    void onResetClicked();
    void onSpeedUpClicked();
    void onSlowDownClicked();

private:
    // Объявляем сами кнопки
    QPushButton *btnSetAlphabets;
    QPushButton *btnSetString;
    QPushButton *btnRun;
    QPushButton *btnStop;
    QPushButton *btnStep;
    QPushButton *btnReset;
    QPushButton *btnSpeedUp;
    QPushButton *btnSlowDown;
};

#endif // TURING_MACHINE_H
