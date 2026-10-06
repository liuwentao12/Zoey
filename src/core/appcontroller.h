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
    // Demonstration state only; no microphone or inference service is started.
    void startListening();

signals:
    void statusChanged(const QString &status);

private:
    QString m_status = QStringLiteral("System Ready");
};

} // namespace zoey
