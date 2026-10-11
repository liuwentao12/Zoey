#include "modelservice.h"

#include <QDebug>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QStringList>
#include <QTimer>

namespace zoey {

ModelService::ModelService(const ModelConfig &config, QObject *parent)
    : QObject(parent),
      m_config(config),
      m_process(new QProcess(this)),
      m_networkManager(new QNetworkAccessManager(this))
{
    connect(m_process, &QProcess::started, this, [this]() {
        // 进程启动不等于权重已加载，因此此时先轮询 /health。
        qInfo() << "llama-server 进程已启动，正在加载模型。";
        emit statusChanged(QStringLiteral("模型加载中…"));
        checkHealth();
    });

    connect(m_process, &QProcess::errorOccurred, this, [this](QProcess::ProcessError) {
        const QString message = QStringLiteral("模型进程错误：") + m_process->errorString();
        qWarning() << message;
        emit statusChanged(message);
    });

    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int, QProcess::ExitStatus exitStatus) {
        // 就绪后已停止轮询，仍需监听进程退出，防止界面保留“模型已就绪”。
        // 崩溃时 errorOccurred 已提供错误细节，不用通用停止提示覆盖它。
        if (exitStatus == QProcess::NormalExit) {
            emit statusChanged(QStringLiteral("模型进程已停止，请查看终端日志"));
        }
    });
}

ModelService::~ModelService()
{
    if (m_process->state() == QProcess::NotRunning) {
        return;
    }

    // 退出阶段不再向业务层发送进程状态；正常运行期间不阻塞等待模型。
    m_process->disconnect(this);
    m_process->terminate();
    if (!m_process->waitForFinished(3000)) {
        m_process->kill();
        m_process->waitForFinished(1000);
    }
}

void ModelService::start()
{
    if (m_process->state() != QProcess::NotRunning) {
        return;
    }

    const QString program = QDir(m_config.llamaDirectory).filePath(QStringLiteral("build/bin/llama-server"));
    const QStringList arguments = {
        QStringLiteral("-hf"), m_config.modelId,
        QStringLiteral("--host"), m_config.host,
        QStringLiteral("--port"), QString::number(m_config.port),
        QStringLiteral("-c"), QString::number(m_config.contextSize),
        QStringLiteral("--jinja")
    };

    m_process->setWorkingDirectory(m_config.llamaDirectory);
    m_process->setProcessChannelMode(QProcess::ForwardedChannels);
    m_process->start(program, arguments);
}

void ModelService::checkHealth()
{
    // 进程错误及退出由 QProcess 信号通知，避免旧轮询覆盖更具体的错误。
    if (m_process->state() != QProcess::Running) {
        return;
    }

    QNetworkRequest request{m_config.endpoint(QStringLiteral("/health"))};
    request.setTransferTimeout(m_config.healthTimeoutMs);
    QNetworkReply *reply = m_networkManager->get(request);

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        const int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        const bool requestSucceeded = reply->error() == QNetworkReply::NoError;
        reply->deleteLater();

        if (m_process->state() != QProcess::Running) {
            return;
        }

        if (requestSucceeded && httpStatus == 200) {
            emit statusChanged(QStringLiteral("模型已就绪"));
            return;
        }

        // 加载阶段暂不可用时，等本次请求完成再安排下一次，避免请求重叠。
        // this 是 timer context，服务对象销毁后不会再触发检查。
        QTimer::singleShot(m_config.healthRetryIntervalMs, this, &ModelService::checkHealth);
    });
}

} // namespace zoey
