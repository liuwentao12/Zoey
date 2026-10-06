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

} // namespace zoey
