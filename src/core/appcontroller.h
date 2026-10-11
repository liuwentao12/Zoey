#pragma once

#include <QObject>
#include <QString>

namespace zoey {

class AppController : public QObject
{
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);
    QString status() const;
    bool isBusy() const;

public slots:
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
    QString m_status = QStringLiteral("System Ready");
    bool m_busy = false;
    void setBusy(bool busy);
};

} // namespace zoey
