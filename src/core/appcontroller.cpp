#include "appcontroller.h"

namespace zoey {

AppController::AppController(QObject *parent)
    : QObject(parent)
{
}

QString AppController::status() const
{
    return m_status;
}

bool AppController::isBusy() const
{
    return m_busy;
}

void AppController::startListening()
{
    setStatus(QStringLiteral("Listening..."));
}

void AppController::setStatus(const QString &status)
{
    if (m_status == status) {
        return;
    }

    m_status = status;
    emit statusChanged(m_status);
}

void AppController::sendMessage(const QString &message)
{
    const QString text = message.trimmed();

    // Core 也做校验，确保未来从 UI 之外调用时仍遵守一次只处理一条请求。
    if (text.isEmpty() || m_busy) {
        return;
    }

    // 先通知 UI 进入忙碌状态，再交给客户端发送，避免发送阶段重复提交。
    setBusy(true);
    emit messageRequested(text);
}

void AppController::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }

    m_busy = busy;
    emit busyChanged(m_busy);
}

void AppController::handleReply(const QString &text)
{
    // 成功和失败都先恢复可发送状态，再通知 UI 展示本次结果。
    setBusy(false);
    emit replyReceived(text);
}

void AppController::handleError(const QString &message)
{
    setBusy(false);
    emit errorOccurred(message);
}

} // namespace zoey
