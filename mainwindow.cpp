#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QApplication>
#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <cmath>
#include <limits>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    QWidget *central = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(central);

    QLabel *footageLabel = new QLabel("Footage per leg:", this);
    footageInput = new QLineEdit(this);

    QLabel *numCablesLabel = new QLabel("Number of legs:", this);
    numOfCablesInput = new QLineEdit(this);

    QLabel *diameterLabel = new QLabel("Select Wire Gauge:", this);
    comboBox = new QComboBox(this);

    inputSubmit = new QPushButton("Submit", this);

    // New: label to show result on screen
    resultLabel = new QLabel("Result: (none yet)", this);

    layout->addWidget(footageLabel);
    layout->addWidget(footageInput);
    layout->addWidget(numCablesLabel);
    layout->addWidget(numOfCablesInput);
    layout->addWidget(diameterLabel);
    layout->addWidget(comboBox);
    layout->addWidget(inputSubmit);
    layout->addWidget(resultLabel);

    setCentralWidget(central);
    setWindowTitle("Cerro Reel Calculator");
    setWindowIcon(QIcon(":/CerroLogo.png"));

    resize(800, 600);

    populateWireDiameters();

    connect(inputSubmit, &QPushButton::clicked,
            this, &MainWindow::onButtonClicked);
}

void MainWindow::populateWireDiameters()
{
    QStringList wireGauges = {"1/0 AWG", "2/0 AWG", "3/0 AWG", "4/0 AWG","250 MCM", "300 MCM", "350 MCM", "400 MCM", "500 MCM", "600 MCM", "750 MCM"};
    QList<double> diameters = {0.474, 0.518, 0.568 , 0.624, 0.678, 0.73, 0.777, 0.821, 0.902, 1.051, 1.156};

    for(int i = 0; i < wireGauges.size(); i++){
        comboBox->addItem(wireGauges[i]);
        comboBox->setItemData(i, diameters[i], Qt::UserRole);
    }
}

// Compute and display result on label
void MainWindow::onButtonClicked()
{
    // Parse user input
    bool okFootage = false;
    int footage = footageInput->text().toInt(&okFootage);

    bool okLegs = false;
    int numLegs = numOfCablesInput->text().toInt(&okLegs);

    int selIndex = comboBox->currentIndex();
    double wireDiameter = comboBox->itemData(selIndex, Qt::UserRole).toDouble();

    // Validate
    if(!okFootage || !okLegs || footage <= 0 || numLegs <= 0) {
        resultLabel->setText("Invalid input. Please enter positive integers.");
        return;
    }

    // Call reel selection
    Reel bestReel = selectBestReelQuadrant(footage, numLegs, wireDiameter);

    // If no reel found, label is empty
    if(bestReel.label.isEmpty()) {
        resultLabel->setText("No reel found that meets the requirement!");
        return;
    }

    // Otherwise, show reel info on the label
    QString info = QString("Flange: %2\n"
                           "Width: %3\n"
                           "Drum: %4")
                       .arg(bestReel.flange)
                       .arg(bestReel.width)
                       .arg(bestReel.diameter);

    resultLabel->setText(info);
}


// Formulas
double MainWindow::getQuadA(double traverse, double cableDiameter)
{
    if(cableDiameter <= 0.0) return 0.0;
    double count = std::floor(traverse / cableDiameter);
    return 0.262 * count / cableDiameter;
}

double MainWindow::getQuadB(double quadA, double drum, int stackCount, double xValue)
{
    if(stackCount <= 1) {
        return quadA * drum;
    } else {
        return quadA * (2.0 * xValue + drum);
    }
}

double MainWindow::getQuadC(int numberOfCables, double footage)
{
    if(numberOfCables <= 1) {
        return -(numberOfCables * footage * 1.04);
    } else {
        return -(numberOfCables * footage * 1.20);
    }
}

double MainWindow::getX(double quadA, double quadB, double quadC)
{
    if(quadA == 0.0) return 0.0;
    double disc = quadB * quadB - 4.0 * quadA * quadC;
    if(disc < 0.0) {
        return 0.0;
    }
    return (-quadB + std::sqrt(disc)) / (2.0 * quadA);
}

double MainWindow::getClearance(double flange, double drum, double x)
{
    return ((flange - drum) / 2.0) - x;
}


// Reel List + Selection of reel
QList<Reel> MainWindow::preloadReels()
{
    QList<Reel> reels;
    reels.append({ "A", 16, 11,  8,  16*11*8*M_PI/4 });
    reels.append({ "B", 24, 12, 10, 24*12*10*M_PI/4 });
    reels.append({ "C", 24, 16, 12, 24*16*12*M_PI/4 });
    reels.append({ "D", 30, 18, 14, 30*18*14*M_PI/4 });
    reels.append({ "E", 32, 22, 14, 32*22*14*M_PI/4 });
    reels.append({ "F", 36, 22, 14, 36*22*14*M_PI/4 });
    reels.append({ "G", 42, 23, 16, 42*23*16*M_PI/4 });
    reels.append({ "H", 48, 23, 16, 48*23*16*M_PI/4 });
    reels.append({ "I", 60, 32, 28, 60*32*28*M_PI/4 });
    return reels;
}

Reel MainWindow::selectBestReelQuadrant(int footage, int numLegs, double wireDiameter)
{
    QList<Reel> reels = preloadReels();

    // Keeps track of two "best" reels:
    Reel bestWith2Inch;
    double minFlange2Inch = std::numeric_limits<double>::max();

    // The best reel with clearance >= 0"
    Reel bestWith0Inch;
    double minFlange0Inch = std::numeric_limits<double>::max();

    const int stackCount = 1;
    const double xValue = 0.0;

    // Loop over each reel and compute clearance
    for(const Reel &r : reels)
    {
        double quadA = getQuadA(r.width, wireDiameter);
        double quadB = getQuadB(quadA, r.diameter, stackCount, xValue);
        double quadC = getQuadC(numLegs, footage);
        double xVal  = getX(quadA, quadB, quadC);

        double clearance = getClearance(r.flange, r.diameter, xVal);

        qDebug() << "[Reel" << r.label << "]"
                 << "flange=" << r.flange
                 << "width="  << r.width
                 << "drum="   << r.diameter
                 << "quadA="  << quadA
                 << "quadB="  << quadB
                 << "quadC="  << quadC
                 << "xVal="   << xVal
                 << "clearance=" << clearance;

        // Check if we have at least 0" clearance
        if(clearance >= 0.0)
        {
            // If it's smaller flange than our current best for >= 0"
            if(r.flange < minFlange0Inch) {
                minFlange0Inch = r.flange;
                bestWith0Inch = r;
            }
        }

        // Check if we have at least 2" clearance
        if(clearance >= 2.0)
        {
            // If it's smaller flange than our current best for >= 2"
            if(r.flange < minFlange2Inch) {
                minFlange2Inch = r.flange;
                bestWith2Inch = r;
            }
        }
    }

    // If there is any reel with >= 2" clearance, return that.
    // Otherwise, return the best reel with >= 0" clearance.
    // If even that is not found, it remains empty.
    if(!bestWith2Inch.label.isEmpty()) {
        return bestWith2Inch;
    } else {
        return bestWith0Inch;
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}
