#include "mainwindow.h"
#include "core/appcontroller.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <QPlainTextEdit>
#include <qboxlayout.h>
#include <qlineedit.h>
#include <qplaintextedit.h>
#include <qpushbutton.h>
#include <qstringliteral.h>
#include <QLineEdit>
#include <QHBoxLayout>

namespace zoey {

MainWindow::MainWindow(AppController &controller, QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Zoey 端侧AI小助手"));
    resize(800, 400);

    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    auto *titleLabel = new QLabel(QStringLiteral("Zoey"), centralWidget);
    auto *statusLabel = new QLabel(controller.status(), centralWidget);
    auto *statusButton = new QPushButton(QStringLiteral("change Status"), centralWidget);
    auto *conversationEdit = new QPlainTextEdit(centralWidget);
    auto *inputEdit = new QLineEdit(centralWidget);
    auto *sendButton = new QPushButton(QStringLiteral("发送"),centralWidget);
    auto *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(inputEdit);
    inputLayout->addWidget(sendButton);
    inputEdit->setPlaceholderText(QStringLiteral("输入消息..."));
    conversationEdit->setReadOnly(true);
    conversationEdit->appendPlainText(QStringLiteral("这里将显示对话记录："));

    statusLabel->setObjectName(QStringLiteral("statusLabel"));
    statusButton->setObjectName(QStringLiteral("statusButton"));

    layout->addWidget(titleLabel);
    layout->addWidget(statusLabel);
    layout->addWidget(conversationEdit);
    layout->addWidget(statusButton);
    layout->addLayout(inputLayout);
    setCentralWidget(centralWidget);

    connect(statusButton, &QPushButton::clicked,
            &controller, &AppController::startListening);
    connect(&controller, &AppController::statusChanged,
            statusLabel, &QLabel::setText);
    connect(sendButton, &QPushButton::clicked,
            this, [inputEdit, conversationEdit, &controller]() {
        const QString message = inputEdit->text().trimmed();

        if (message.isEmpty()) {
            return;
        }

        conversationEdit->appendPlainText(
            QStringLiteral("你：") + message
        );

        inputEdit->clear();
        controller.sendMessage(message);
    });
    connect(&controller, &AppController::replyReceived,
            this, [conversationEdit](const QString &text) {
        conversationEdit->appendPlainText(
            QStringLiteral("Zoey：") + text
        );
    });

    connect(&controller, &AppController::errorOccurred,
            this, [conversationEdit](const QString &message) {
        conversationEdit->appendPlainText(
            QStringLiteral("错误：") + message
        );
    });

}

} // namespace zoey
