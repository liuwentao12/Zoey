#include "appcontroller.h"

namespace zoey {

AppController::AppController(QObject *parent)
    : QObject(parent)
{
}

QString AppController::status() const
{
    return m_status;
}

void AppController::startListening()
{
    const QString nextStatus = QStringLiteral("Listening...");
    if (m_status == nextStatus) {
        return;
    }

    m_status = nextStatus;
    emit statusChanged(m_status);
}

void AppController::setStatus(const QString &status)
{
    if(m_status == status)return;

    m_status = status;

    emit statusChanged(m_status);
}

void AppController::sendMessage(const QString &message)
{
    const QString text = message.trimmed();

    if (text.isEmpty()) {
        return;
    }

    emit messageRequested(text);
}

} // namespace zoey
