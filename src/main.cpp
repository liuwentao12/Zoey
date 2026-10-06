#include "ai/modelservice.h"
#include "core/appcontroller.h"
#include "ui/mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QTimer>
#include <qobject.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName(QStringLiteral("Zoey"));
    QCoreApplication::setOrganizationName(QStringLiteral("Zoey"));

    zoey::ModelService modelService;
    zoey::AppController controller;
    zoey::MainWindow window(controller);

    QObject::connect(
    &modelService,
    &zoey::ModelService::statusChanged,
    &controller,
    &zoey::AppController::setStatus
    );
    
    window.show();

    QTimer::singleShot(
        0,
        &modelService,
        &zoey::ModelService::start
    );

    return app.exec();
}