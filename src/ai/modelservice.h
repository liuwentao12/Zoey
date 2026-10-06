#pragma once

#include <QObject>
#include <QString>

class QProcess;
class QNetworkAccessManager;

namespace zoey {

class ModelService : public QObject
{
    Q_OBJECT

public:
    explicit ModelService(QObject *parent = nullptr);
    ~ModelService() override;

    void start();

signals:
    void statusChanged(const QString &status);

private:
    void checkHealth();

    QProcess *m_process;
    QNetworkAccessManager *m_networkManager;
};

} // namespace zoey