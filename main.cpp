#include <QApplication>
#include "mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile file(":/styles/Style.qss");
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        QString styleSheet = stream.readAll();
        app.setStyleSheet(styleSheet);
        file.close();
        qDebug() << "style.qss opened";
    } else {
        qDebug() << "Unable to open style.qss file.";
    }

    MainWindow w;
    w.show();
    return app.exec();
}
