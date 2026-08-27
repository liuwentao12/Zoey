#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "qlabel.h"
#include "qpushbutton.h"
#include "qwidget.h"
#include <QLabel>
#include <QBoxLayout>
#include <QWidget>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("Zoey");
    resize(800, 400);

    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);

    titleLable = new QLabel("Zoey");
    statusLabel = new QLabel("System Ready");
    statusButton = new QPushButton("change Status");

    layout ->addWidget(titleLable);
    layout->addWidget(statusLabel);
    layout->addWidget(statusButton);

    setCentralWidget(centralWidget);

    connect(statusButton, &QPushButton::clicked, this, [this]() {
        statusLabel->setText("Listening...");
    });
}

MainWindow::~MainWindow()
{
    delete ui;
}

