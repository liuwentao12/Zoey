#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QDebug>
#include <QJsonParseError>

#include "aiclient.h"

#include <QNetworkAccessManager>

namespace zoey {

AiClient::AiClient(QObject *parent)
    : QObject(parent),
      m_networkManager(new QNetworkAccessManager(this))
{
}

void AiClient::sendMessage(const QString &message)
{
    QJsonObject userMessage;
    userMessage["role"] = QStringLiteral("user");
    userMessage["content"] = message;

    QJsonArray messages;
    messages.append(userMessage);

    QJsonObject body;
    body["model"] = QStringLiteral("Qwen/Qwen3-4B-GGUF:Q4_K_M");
    body["messages"] = messages;
    body["stream"] = false;
    body["max_tokens"] = 128;

    QJsonObject templateOptions;
    templateOptions["enable_thinking"] = false;
    body["chat_template_kwargs"] = templateOptions;

    QNetworkRequest request{
        QUrl(QStringLiteral("http://127.0.0.1:8080/v1/chat/completions"))
    };
    request.setHeader(
        QNetworkRequest::ContentTypeHeader,
        QStringLiteral("application/json")
    );
    request.setTransferTimeout(180000);

    const QByteArray requestData =
        QJsonDocument(body).toJson(QJsonDocument::Compact);

    QNetworkReply *reply =
        m_networkManager->post(request, requestData);

    connect(reply, &QNetworkReply::finished,
            this, [this, reply]() {
        const QByteArray responseData = reply->readAll();

        if (reply->error() != QNetworkReply::NoError) {
            const QString error = reply->errorString();
            qWarning().noquote() << error;
            emit errorOccurred(error);
        } else {
            QJsonParseError parseError;
            const QJsonDocument document =
                QJsonDocument::fromJson(responseData, &parseError);

            if (parseError.error != QJsonParseError::NoError) {
                emit errorOccurred(QStringLiteral("无法解析模型返回的 JSON"));
            } else {
                const QJsonArray choices =
                    document.object().value("choices").toArray();

                if (choices.isEmpty()) {
                    emit errorOccurred(QStringLiteral("模型返回结果缺少 choices"));
                } else {
                    const QJsonObject messageObject =
                        choices.at(0).toObject().value("message").toObject();

                    const QJsonValue content = messageObject.value("content");

                    if (!content.isString()) {
                        emit errorOccurred(QStringLiteral("模型返回结果缺少回复文字"));
                    } else {
                        emit replyReceived(content.toString());
                    }
                }
            }
        }

        reply->deleteLater();
    });
}

} // namespace zoey