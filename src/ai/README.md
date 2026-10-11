# AI 接入模块

当前已实现模型进程管理与单次非流式对话，依赖 Qt Core/Network。

| 文件 | 职责 |
| --- | --- |
| `modelconfig.h` | 服务与客户端共享的默认模型、地址、路径和超时；生成服务 URL |
| `modelservice.h/.cpp` | 启动 llama-server、加载健康检查、进程错误及退出通知、关闭时清理 |
| `aiclient.h/.cpp` | 构造对话请求、异步发送、解析 JSON 与返回结果 |

`main.cpp` 创建配置并传给两个 AI 对象，通过信号将它们与 AppController 连接。AI 模块不依赖窗口、Core、设备或 MQTT，窗口不拼装请求 JSON。

## 模型进程

使用用户主目录下 `llama.cpp/build/bin/llama-server`，以 `-hf` 指定当前 Qwen3-4B GGUF 模型，启用 `--jinja`。QProcess 的 started 信号触发加载状态；`GET /health` 成功且状态码为 200 才表示就绪。

健康检查在加载阶段按“本次请求完成 → 等待 → 下一次请求”进行，就绪后停止。进程退出或错误另由 QProcess 信号通知；应用退出时先 terminate，再在必要时 kill 本应用创建的进程。

## 一次对话

`sendMessage()` 发起 `POST /v1/chat/completions`，`createRequestBody()` 组织 JSON，`handleFinishedReply()` 处理结束结果。请求保留 `stream=false`、回复上限 128 token，以及关闭 thinking 的模板参数。每次只发送当前一条 user 消息，UI 记录不等于模型上下文。

成功发出 `replyReceived(text)`；网络失败、非法 JSON、缺少 choices 或回复文字时发出 `errorOccurred(message)`。每个完成回调只发出一个终结结果，并安排 reply 延迟释放。Core 根据结果解除忙碌状态，AI 模块自身不控制按钮或业务状态。

当前尚未实现取消、流式输出、多轮上下文、服务自动重启、加载总超时或工具调用。后续工具请求应交给 Core 校验与分派，不能从 AI 模块直接执行设备控制。

详细数据流、所有权和阅读路线见 [架构文档](../../docs/architecture.md)。
