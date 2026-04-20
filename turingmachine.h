#ifndef TURINGMACHINE_H
#define TURINGMACHINE_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>

class TuringMachine : public QWidget {
    Q_OBJECT

public:
    TuringMachine(QWidget *parent = nullptr);

private slots:
    void onSetAlphabets(); // Логика перехода к Окну 2
    void onReset();        // Возврат к Окну 1

private:
    // Элементы Окна 1
    QWidget *setupWidget;
    QLineEdit *lineAlphabet;
    QLineEdit *extraAlphabet;

    // Элементы Окна 2
    QWidget *mainWidget;
    QLineEdit *tapeInput;
    QTableWidget *programTable;

    // Слой для переключения между окнами
    QVBoxLayout *globalLayout;
};

#endif
