#pragma once

#include "common/chatmessage.h"

#include <QObject>
#include <QString>

namespace zoey {

struct ConversationLimits
{
    int maxHistoryTurns = 8; // 只保留完整的 user/assistant 轮次。
    int maxInputBytes = 4096; // 请求中消息内容的 UTF-8 字节预算，不是精确 token 数。
};

// 应用协调层：保存状态、限制同时发出的对话请求，并向 UI 转发结果。
// 不依赖 Widgets 或 HTTP；具体模块通过 main 中的信号连接协作。
class AppController : public QObject
{
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);
    explicit AppController(const ConversationLimits &limits, QObject *parent = nullptr);
    QString status() const;
    bool isBusy() const;

public slots:
    // 保留原型入口：只改变演示状态，尚未启动录音。
    void startListening();
    void setStatus(const QString &status);
    void sendMessage(const QString &message);
    void handleReply(const QString &text);
    void handleError(const QString &message);
    // 生成期间不清空，防止旧请求的回复进入新会话。
    void newConversation();

signals:
    void statusChanged(const QString &status);
    void messageAccepted(const QString &message);
    void messageRequested(const ChatMessages &messages);
    void replyReceived(const QString &text);
    void errorOccurred(const QString &message);
    void busyChanged(bool busy);
    void conversationCleared();

private:
    void setBusy(bool busy);
    void trimHistory();

    const ConversationLimits m_limits;
    ChatMessages m_history;
    QString m_pendingMessage; // 只有收到成功回复，才与回复一起加入历史。
    // 服务状态与对话忙碌状态独立：生成结束后 UI 恢复显示最新服务状态。
    QString m_status = QStringLiteral("System Ready");
    bool m_busy = false;
};

} // namespace zoey
