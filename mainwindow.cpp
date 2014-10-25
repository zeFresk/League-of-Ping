#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <stdexcept>
#include <QMessageBox>
#include <algorithm>
#include <QTimer>

using namespace std;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    do {
        try {
            pinger = unique_ptr<Pinger>(new Pinger("riot.de"));
        }
        catch (std::exception const& e) {
            QMessageBox::critical(this, "Erreur", e.what());
        }
    } while (pinger == nullptr);

    pingOnce(); // pas de début à 0 !
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(pingOnce()));
    timer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateAll()
{
    updateLast();
    updateMoy();
    updateEcart();
}

void MainWindow::updateLast()
{
    ui->LastPing->setText(getColored(timers[timers.size() - 1]));
}

void MainWindow::updateMoy()
{
    unsigned long total = 0;
    for (auto &a : timers)
    {
        total += a;
    }
    ui->MoyennePing->setText(getColored(total / timers.size()));
    ui->HealthBar->setValue(static_cast<int>(total / timers.size()));
}

void MainWindow::updateEcart()
{
    unsigned long min = *std::min_element(timers.begin(), timers.end());
    unsigned long max = *std::max_element(timers.begin(), timers.end());
    ui->EcartPing->setText(getColored(max-min));
}

QString getColored(unsigned long num)
{
    QString color = "";
    if (num < 30)
        color = "80dfff";
    else if (num < 55)
        color = "76eb00";
    else if (num < 80)
        color = "ffdf80";
    else if (num < 100)
        color = "eb7600";
    else
        color = "d80000";
    return "<html><head/><body><p align=\"center\"><span style=\" font-size:11pt; font-weight:600; font-style:italic; color:#" + color + ";\">"+ QString::number(num) +"</span></p></body></html>";
}

void MainWindow::pingOnce()
{
    try {
        pinger->ping();
        timers.push_back(pinger->getTime());
        if (timers.size() > 60*2*2) timers.pop_front(); //on garde les deux dernières minutes, sinon on enlève au début
        updateAll();
    }
    catch (std::exception const& e)
    {
        // rien
    }
}
