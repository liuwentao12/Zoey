#include "mainwindow.h"
#include "core/appcontroller.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>

namespace zoey {

MainWindow::MainWindow(AppController &controller, QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(QStringLiteral("Zoey 端侧AI小助手"));
    resize(800, 400);

    // 控件和安装到窗口的布局由 Qt 父子关系管理，不需要手动 delete。
    auto *centralWidget = new QWidget(this);
    auto *layout = new QVBoxLayout(centralWidget);
    auto *titleLabel = new QLabel(QStringLiteral("Zoey"), centralWidget);
    auto *statusLabel = new QLabel(controller.status(), centralWidget);
    auto *conversationEdit = new QPlainTextEdit(centralWidget);
    auto *inputEdit = new QLineEdit(centralWidget);
    auto *sendButton = new QPushButton(QStringLiteral("发送"), centralWidget);
    auto *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(inputEdit);
    inputLayout->addWidget(sendButton);
    inputEdit->setPlaceholderText(QStringLiteral("输入消息..."));
    conversationEdit->setReadOnly(true);
    conversationEdit->appendPlainText(QStringLiteral("这里将显示对话记录："));

    statusLabel->setObjectName(QStringLiteral("statusLabel"));

    layout->addWidget(titleLabel);
    layout->addWidget(statusLabel);
    layout->addWidget(conversationEdit);
    layout->addLayout(inputLayout);
    setCentralWidget(centralWidget);

    // 忙碌时临时显示生成提示；服务状态仍由控制器保存，生成结束后恢复显示。
    const auto updateUi = [statusLabel, sendButton, &controller]() {
        const bool busy = controller.isBusy();
        sendButton->setEnabled(!busy);
        sendButton->setText(busy ? QStringLiteral("生成中...") : QStringLiteral("发送"));
        statusLabel->setText(busy ? QStringLiteral("正在生成...") : controller.status());
    };

    // this 是连接的 context：窗口关闭并销毁后，这些回调会自动断开。
    connect(&controller, &AppController::busyChanged, this, updateUi);
    connect(&controller, &AppController::statusChanged, this, updateUi);
    connect(sendButton, &QPushButton::clicked,
            this, [inputEdit, conversationEdit, &controller]() {
        const QString message = inputEdit->text().trimmed();

        if (message.isEmpty()) {
            return;
        }

        // 这里只维护展示记录；模型上下文由 AI 客户端决定。
        conversationEdit->appendPlainText(QStringLiteral("你：") + message);
        inputEdit->clear();
        controller.sendMessage(message);
    });
    connect(&controller, &AppController::replyReceived,
            this, [conversationEdit](const QString &text) {
        conversationEdit->appendPlainText(QStringLiteral("Zoey：") + text);
    });

    connect(&controller, &AppController::errorOccurred,
            this, [conversationEdit](const QString &message) {
        conversationEdit->appendPlainText(QStringLiteral("错误：") + message);
    });
    // 控制器可能在窗口创建前已经进入忙碌状态，不能只等待下一次信号。
    updateUi();
}

} // namespace zoey
