#include "appcontroller.h"

namespace zoey {

namespace {

qint64 contentBytes(const ChatMessages &messages)
{
    qint64 bytes = 0;
    for (const ChatMessage &message : messages) {
        bytes += message.content.toUtf8().size();
    }
    return bytes;
}

void removeOldestTurn(ChatMessages &messages)
{
    // 历史始终成对保存；一起删除，避免请求以 assistant 消息开头。
    messages.removeFirst();
    messages.removeFirst();
}

} // namespace

AppController::AppController(QObject *parent)
    : AppController(ConversationLimits{}, parent)
{
}

AppController::AppController(const ConversationLimits &limits, QObject *parent)
    : QObject(parent),
      m_limits{qMax(0, limits.maxHistoryTurns), qMax(0, limits.maxInputBytes)}
{
    qRegisterMetaType<ChatMessages>("ChatMessages");
    qRegisterMetaType<ChatMessages>("zoey::ChatMessages");
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

    const qint64 inputBytes = text.toUtf8().size();
    if (inputBytes > m_limits.maxInputBytes) {
        emit errorOccurred(QStringLiteral("消息过长，请缩短后重试。"));
        return;
    }

    // 只裁剪本次请求的副本；失败时保留此前成功的对话历史。
    ChatMessages messages = m_history;
    while (!messages.isEmpty()
           && contentBytes(messages) + inputBytes > m_limits.maxInputBytes) {
        removeOldestTurn(messages);
    }
    messages.append(ChatMessage{ChatMessage::Role::User, text});
    m_pendingMessage = text;

    // 先通知 UI 进入忙碌状态，再交给客户端发送，避免发送阶段重复提交。
    setBusy(true);
    emit messageAccepted(text);
    emit messageRequested(messages);
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
    if (!m_busy) {
        return;
    }

    m_history.append(ChatMessage{ChatMessage::Role::User, m_pendingMessage});
    m_history.append(ChatMessage{ChatMessage::Role::Assistant, text});
    trimHistory();
    m_pendingMessage.clear();

    // 成功和失败都先恢复可发送状态，再通知 UI 展示本次结果。
    setBusy(false);
    emit replyReceived(text);
}

void AppController::handleError(const QString &message)
{
    if (!m_busy) {
        return;
    }

    m_pendingMessage.clear();
    setBusy(false);
    emit errorOccurred(message);
}

void AppController::newConversation()
{
    if (m_busy) {
        return;
    }

    m_history.clear();
    m_pendingMessage.clear();
    emit conversationCleared();
}

void AppController::trimHistory()
{
    while (!m_history.isEmpty()
           && (m_history.size() / 2 > m_limits.maxHistoryTurns
               || contentBytes(m_history) > m_limits.maxInputBytes)) {
        removeOldestTurn(m_history);
    }
}

} // namespace zoey
