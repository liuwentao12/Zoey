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

    if (text.isEmpty() || m_busy) {
        return;
    }
    setBusy(true);
    emit messageRequested(text);
}

bool AppController::isBusy() const
{
    return m_busy;
}

void AppController::setBusy(bool busy)
{
    if (m_busy == busy) {
        return;
    }

    m_busy = busy;
    emit busyChanged(m_busy);
}

void AppController::handleReply(const QString &text)
{
    setBusy(false);
    emit replyReceived(text);
}

void AppController::handleError(const QString &message)
{
    setBusy(false);
    emit errorOccurred(message);
}

} // namespace zoey
