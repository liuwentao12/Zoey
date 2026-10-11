#pragma once

#include <QObject>
#include <QString>

class QNetworkAccessManager;

namespace zoey {

class AiClient : public QObject
{
    Q_OBJECT

public:
    explicit AiClient(QObject *parent = nullptr);

    void sendMessage(const QString &message);

signals:
    void replyReceived(const QString &text);
    void errorOccurred(const QString &message);

private:
    QNetworkAccessManager *m_networkManager;
};

} // namespace zoey