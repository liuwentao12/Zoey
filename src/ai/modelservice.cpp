#include "modelservice.h"

#include <QDir>
#include <QProcess>
#include <QStringList>
#include <qnetworkaccessmanager.h>
#include <qprocess.h>
#include <QDebug>
#include <qstringliteral.h>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QTimer>

namespace zoey {

ModelService::ModelService(QObject *parent)
    : QObject(parent),
      m_process(new QProcess(this)),
      m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_process, &QProcess::started,
            this, [this]() {
        qInfo() << "llama-server 进程已启动，正在加载模型。";
        emit statusChanged(QStringLiteral("模型加载中…"));
        checkHealth();
    });

    connect(m_process, &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError) {
        const QString message =
            QStringLiteral("模型进程错误：") + m_process->errorString();
        qWarning() << message;
        emit statusChanged(message);
    });
}

ModelService::~ModelService()
{
    if (m_process->state() == QProcess::NotRunning) {
        return;
    }

    m_process->terminate();

    if (!m_process->waitForFinished(3000)) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

void ModelService::start()
{
    if (m_process->state() != QProcess::NotRunning)
    {
        return;
    }

    const QString llamaDirectory = QDir::homePath() + "/llama.cpp";

    const QString program = llamaDirectory + "/build/bin/llama-server";

    const QStringList arguments = {
        "-hf", "Qwen/Qwen3-4B-GGUF:Q4_K_M",
        "--host", "127.0.0.1",
        "--port", "8080",
        "-c", "8192",
        "--jinja"
    };

    m_process->setWorkingDirectory(llamaDirectory);
    m_process->setProcessChannelMode(QProcess::ForwardedChannels);
    m_process->start(program, arguments);
}

void ModelService::checkHealth()
{
    if (m_process->state() != QProcess::Running) {
        emit statusChanged(QStringLiteral("模型进程已停止，请查看终端日志"));
        return;
    }

    QNetworkRequest request{
        QUrl(QStringLiteral("http://127.0.0.1:8080/health"))
    };

    request.setTransferTimeout(2000);

    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished,
            this, [this, reply]() {
        const int httpStatus =
            reply->attribute(
                QNetworkRequest::HttpStatusCodeAttribute
            ).toInt();

        const bool requestSucceeded =
            reply->error() == QNetworkReply::NoError;

        reply->deleteLater();

        if (m_process->state() != QProcess::Running) {
            emit statusChanged(
                QStringLiteral("模型进程已停止，请查看终端日志")
            );
            return;
        }

        if (requestSucceeded && httpStatus == 200) {
            emit statusChanged(QStringLiteral("模型已就绪"));
            return;
        }

        QTimer::singleShot(
            1000,
            this,
            &ModelService::checkHealth
        );
    });
}

} // namespace zoey