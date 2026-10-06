#include "mainwindow.h"
#include "core/appcontroller.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace zoey {

MainWindow::MainWindow(AppController &controller, QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Zoey"));
    resize(800, 400);

    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    auto *titleLabel = new QLabel(QStringLiteral("Zoey"), centralWidget);
    auto *statusLabel = new QLabel(controller.status(), centralWidget);
    auto *statusButton = new QPushButton(QStringLiteral("change Status"), centralWidget);

    statusLabel->setObjectName(QStringLiteral("statusLabel"));
    statusButton->setObjectName(QStringLiteral("statusButton"));

    layout->addWidget(titleLabel);
    layout->addWidget(statusLabel);
    layout->addWidget(statusButton);
    setCentralWidget(centralWidget);

    connect(statusButton, &QPushButton::clicked,
            &controller, &AppController::startListening);
    connect(&controller, &AppController::statusChanged,
            statusLabel, &QLabel::setText);
}

} // namespace zoey
