#pragma once

#include <QObject>
#include <QString>

namespace zoey {

// 应用协调层：保存状态、限制同时发出的对话请求，并向 UI 转发结果。
// 不依赖 Widgets 或 HTTP；具体模块通过 main 中的信号连接协作。
class AppController : public QObject
{
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);
    QString status() const;
    bool isBusy() const;

public slots:
    // 保留原型入口：只改变演示状态，尚未启动录音。
    void startListening();
    void setStatus(const QString &status);
    void sendMessage(const QString &message);
    void handleReply(const QString &text);
    void handleError(const QString &message);

signals:
    void statusChanged(const QString &status);
    void messageRequested(const QString &message);
    void replyReceived(const QString &text);
    void errorOccurred(const QString &message);
    void busyChanged(bool busy);

private:
    void setBusy(bool busy);

    // 服务状态与对话忙碌状态独立：生成结束后 UI 恢复显示最新服务状态。
    QString m_status = QStringLiteral("System Ready");
    bool m_busy = false;
};

} // namespace zoey
