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

public slots:
    void startListening();
    void setStatus(const QString &status);
    void sendMessage(const QString &message);

signals:
    void statusChanged(const QString &status);
    void messageRequested(const QString &message);
    void replyReceived(const QString &text);
    void errorOccurred(const QString &message);

private:
    QString m_status = QStringLiteral("System Ready");
};

} // namespace zoey
