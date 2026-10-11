#pragma once

#include "modelconfig.h"

#include <QObject>
#include <QString>

class QJsonObject;
class QNetworkAccessManager;
class QNetworkReply;

namespace zoey {

// 只处理对话 HTTP 协议；服务进程由 ModelService 管理，业务状态由 Core 管理。
class AiClient : public QObject
{
    Q_OBJECT

public:
    explicit AiClient(const ModelConfig &config, QObject *parent = nullptr);

    // 异步发送本次输入；通过 replyReceived / errorOccurred 返回结果。
    void sendMessage(const QString &message);

signals:
    void replyReceived(const QString &text);
    void errorOccurred(const QString &message);

private:
    QJsonObject createRequestBody(const QString &message) const;
    void handleFinishedReply(QNetworkReply *reply);

    const ModelConfig m_config; // 保存值，避免依赖入口局部配置的生命周期。
    QNetworkAccessManager *m_networkManager;
};

} // namespace zoey
