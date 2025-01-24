#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QString>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>

QT_BEGIN_NAMESPACE

namespace Ui {
class MainWindow;
}

QT_END_NAMESPACE

struct Reel {
    QString label;
    double flange;
    double width;
    double diameter;
    double volume;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private slots:
    void onButtonClicked();

private:
    QLineEdit   *footageInput;
    QLineEdit   *numOfCablesInput;
    QComboBox   *comboBox;
    QPushButton *inputSubmit;
    QLabel      *resultLabel;

    double getQuadA(double traverse, double cableDiameter);
    double getQuadB(double quadA, double drum, int stackCount, double xValue = 0.0);
    double getQuadC(int numberOfCables, double footage);
    double getX(double quadA, double quadB, double quadC);
    double getClearance(double flange, double drum, double x);
    double getMBR(double cableDiameter);

    QList<Reel> preloadReels();
    Reel selectBestReelQuadrant(int footage, int numLegs, double wireDiameter);

    void populateWireDiameters();
};

#endif // MAINWINDOW_H
