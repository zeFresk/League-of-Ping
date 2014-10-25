#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <deque>
#include "Ping.hpp"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots :
    void pingOnce();

private:
    void updateAll();
    void updateLast();
    void updateMoy();
    void updateEcart();

    Ui::MainWindow *ui;
    std::unique_ptr<Pinger> pinger;
    std::deque<unsigned long> timers;
};

QString getColored(unsigned long num);

#endif // MAINWINDOW_H
