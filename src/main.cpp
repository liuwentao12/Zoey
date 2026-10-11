#include "ai/aiclient.h"
#include "ai/modelconfig.h"
#include "ai/modelservice.h"
#include "core/appcontroller.h"
#include "ui/mainwindow.h"

#include <QApplication>
#include <QCoreApplication>
#include <QObject>
#include <QTimer>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("Zoey"));
    QCoreApplication::setOrganizationName(QStringLiteral("Zoey"));

    // 入口只负责组装：服务与客户端使用同一配置，网络和进程细节留在 AI 模块。
    const zoey::ModelConfig config;
    zoey::ModelService modelService(config);
    zoey::AiClient aiClient(config);
    zoey::AppController controller;
    zoey::MainWindow window(controller);

    // 栈对象按声明的逆序销毁：窗口先于控制器，所有模块先于 QApplication。
    // 窗口引用控制器但不拥有它；Qt 在对象销毁时自动断开相关信号连接。
    QObject::connect(&modelService, &zoey::ModelService::statusChanged,
                     &controller, &zoey::AppController::setStatus);

    // 对话数据流：UI → Core → HTTP 客户端 → Core → UI。
    QObject::connect(&controller, &zoey::AppController::messageRequested,
                     &aiClient, &zoey::AiClient::sendMessage);
    QObject::connect(&aiClient, &zoey::AiClient::replyReceived,
                     &controller, &zoey::AppController::handleReply);
    QObject::connect(&aiClient, &zoey::AiClient::errorOccurred,
                     &controller, &zoey::AppController::handleError);

    window.show();

    // 等事件循环开始后再启动进程，使窗口能先显示并异步接收加载状态。
    QTimer::singleShot(0, &modelService, &zoey::ModelService::start);
    return app.exec();
}
