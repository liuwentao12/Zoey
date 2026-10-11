#include "aiclient.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace zoey {

AiClient::AiClient(const ModelConfig &config, QObject *parent)
    : QObject(parent),
      m_config(config),
      m_networkManager(new QNetworkAccessManager(this))
{
    // NetworkAccessManager 是 Qt 子对象，由客户端析构时统一释放。
}

void AiClient::sendMessage(const QString &message)
{
    QNetworkRequest request{m_config.endpoint(QStringLiteral("/v1/chat/completions"))};
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setTransferTimeout(m_config.requestTimeoutMs);

    const QByteArray requestData =
        QJsonDocument(createRequestBody(message)).toJson(QJsonDocument::Compact);
    QNetworkReply *reply = m_networkManager->post(request, requestData);

    // this 作为回调 context：客户端销毁后不会再执行该回调。
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleFinishedReply(reply);
    });
}

QJsonObject AiClient::createRequestBody(const QString &message) const
{
    QJsonObject userMessage;
    userMessage["role"] = QStringLiteral("user");
    userMessage["content"] = message;

    // 界面的聊天记录仅用于展示；目前每次请求只发送这一条 user 消息。
    QJsonArray messages;
    messages.append(userMessage);

    QJsonObject body;
    body["model"] = m_config.modelId;
    body["messages"] = messages;
    body["stream"] = false;
    body["max_tokens"] = m_config.maxTokens;

    // 通过 llama-server 的模板参数请求关闭 Qwen thinking，保持当前回复方式。
    QJsonObject templateOptions;
    templateOptions["enable_thinking"] = false;
    body["chat_template_kwargs"] = templateOptions;
    return body;
}

void AiClient::handleFinishedReply(QNetworkReply *reply)
{
    // finished 回调中不直接 delete；提前安排释放，下面的错误分支也不会遗漏。
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        const QString error = reply->errorString();
        qWarning().noquote() << error;
        emit errorOccurred(error);
        return;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(reply->readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        emit errorOccurred(QStringLiteral("无法解析模型返回的 JSON"));
        return;
    }

    const QJsonArray choices = document.object().value("choices").toArray();
    if (choices.isEmpty()) {
        emit errorOccurred(QStringLiteral("模型返回结果缺少 choices"));
        return;
    }

    const QJsonObject messageObject = choices.at(0).toObject().value("message").toObject();
    const QJsonValue content = messageObject.value("content");
    if (!content.isString()) {
        emit errorOccurred(QStringLiteral("模型返回结果缺少回复文字"));
        return;
    }

    // 每次请求只有一个结果信号，控制器据此解除忙碌状态。
    emit replyReceived(content.toString());
}

} // namespace zoey
