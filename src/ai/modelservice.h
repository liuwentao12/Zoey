#pragma once

#include "modelconfig.h"

#include <QObject>
#include <QString>

class QProcess;
class QNetworkAccessManager;

namespace zoey {

// 管理本应用启动的 llama-server；健康检查成功才表示模型就绪。
// 对话协议独立放在 AiClient 中，服务状态通过 Core 传给窗口。
class ModelService : public QObject
{
    Q_OBJECT

public:
    explicit ModelService(const ModelConfig &config, QObject *parent = nullptr);
    ~ModelService() override;

    void start(); // 已启动或正在启动时，重复调用不再创建新进程。

signals:
    void statusChanged(const QString &status);

private:
    void checkHealth();

    const ModelConfig m_config;
    // 两者均为 Qt 子对象；析构函数只额外负责停止外部进程。
    QProcess *m_process;
    QNetworkAccessManager *m_networkManager;
};

} // namespace zoey
