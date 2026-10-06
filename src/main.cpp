#include "core/appcontroller.h"
#include "ui/mainwindow.h"

#include <QApplication>
#include <QCoreApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("Zoey"));
    QCoreApplication::setOrganizationName(QStringLiteral("Zoey"));

    zoey::AppController controller;
    zoey::MainWindow window(controller);
    window.show();

    return app.exec();
}
