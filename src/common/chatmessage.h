#pragma once

#include <QList>
#include <QMetaType>
#include <QString>

namespace zoey {

// Core 与 AI 共享的对话数据，不包含 HTTP 或界面细节。
struct ChatMessage
{
    enum class Role { User, Assistant };

    Role role;
    QString content;
};

using ChatMessages = QList<ChatMessage>;

} // namespace zoey

Q_DECLARE_METATYPE(zoey::ChatMessage)
Q_DECLARE_METATYPE(zoey::ChatMessages)
