#pragma once

#include <QDir>
#include <QString>
#include <QUrl>
#include <QtGlobal>

namespace zoey {

// 进程启动与 HTTP 请求共享的参数。当前由 main 创建，尚不读取 JSON 配置文件。
struct ModelConfig
{
    QString modelId = QStringLiteral("Qwen/Qwen3-4B-GGUF:Q4_K_M");
    QString host = QStringLiteral("127.0.0.1");
    quint16 port = 8080;
    QString llamaDirectory = QDir::homePath() + QStringLiteral("/llama.cpp");

    int contextSize = 8192; // 服务上下文容量，单位为 token。
    int maxTokens = 128; // 单次回复的 token 上限。
    int requestTimeoutMs = 180000; // HTTP 传输无数据活动时的超时，单位为毫秒。
    int healthTimeoutMs = 2000;
    int healthRetryIntervalMs = 1000;

    // /health 检查服务是否就绪；/v1/chat/completions 用于对话。
    // 使用 QUrl 分别设置各部分，避免进程地址与客户端 URL 各自硬编码。
    QUrl endpoint(const QString &path) const
    {
        QUrl url;
        url.setScheme(QStringLiteral("http"));
        url.setHost(host);
        url.setPort(port);
        url.setPath(path);
        return url;
    }
};

} // namespace zoey
