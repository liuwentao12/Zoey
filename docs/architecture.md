# Zoey 架构与开发约定

## 当前范围

Zoey 是运行在 ROCK 5 ITX 上的 Qt/C++ 本地 AI 与 IoT 应用。目前已实现 Qt 对话界面、应用状态协调、llama-server 自动启动与健康检查，以及多轮非流式 HTTP 对话。设备管理、MQTT、工具调用仍属于后续规划。

应用采用 UI、Core、AI 分层，共享模型参数；进程退出状态和窗口初始状态通过控制器同步。

## 目录职责

| 路径 | 职责 |
| --- | --- |
| `CMakeLists.txt` / `CMakePresets.json` | 统一构建入口、依赖、Debug/Release 配置与测试入口 |
| `src/main.cpp` | 创建配置和模块、连接信号、显示窗口并安排服务启动 |
| `src/common/chatmessage.h` | Core 与 AI 共享的消息角色、内容及消息列表，不包含 HTTP 细节 |
| `src/ui/` | 手写 Widgets 窗口、用户输入、状态与对话记录展示 |
| `src/core/` | 服务状态与对话忙碌状态、请求校验、会话历史与裁剪、结果转发；不依赖 Widgets |
| `src/ai/modelconfig.h` | 共享模型标识、地址、路径及超时参数 |
| `src/ai/modelservice.h/.cpp` | 本应用创建的 llama-server 进程及加载健康检查 |
| `src/ai/aiclient.h/.cpp` | HTTP 对话请求、JSON 响应解析与错误返回 |
| `src/devices/` / `src/mqtt/` | 后续模块的开发约定，目前没有业务实现 |
| `config/` | 配置示例；当前不由应用读取，本机配置不提交 |
| `tests/` | Core 状态、窗口发送操作和窗口生命周期测试 |
| `docs/` | 架构、开发指南、设计与实施计划 |
| `build/` | 编译产物，不提交 |

不创建没有调用者的 Manager 类、接口层或空 CMake 库。新增功能先实现一个完整流程，再根据实际职责划分模块。

## 当前对象与数据流

```mermaid
flowchart LR
    Entry[main.cpp] --> Config[ModelConfig]
    Entry --> Window[MainWindow]
    Entry --> Core[AppController]
    Entry --> Client[AiClient]
    Entry --> Service[ModelService]
    Config --> Client
    Config --> Service
    Window -->|sendMessage| Core
    Core -->|messageRequested| Client
    Client -->|HTTP 对话| Server[llama-server]
    Client -->|replyReceived / errorOccurred| Core
    Core -->|状态与结果| Window
    Service -->|启动 / health / 退出清理| Server
    Service -->|statusChanged| Core
```

图中的连线表示对象组装和运行时消息流。例如，`AppController` 发出 `messageRequested`，由 `main.cpp` 中的连接把消息交给 `AiClient`。

从 [CMake 目标](../src/CMakeLists.txt) 看，`Zoey` 包含入口和窗口，链接 `zoey_core`、`zoey_ai` 和 Qt Widgets。`zoey_core` 依赖 Qt Core，`zoey_ai` 依赖 Qt Core 和 Qt Network。Core 和 AI 共享 `ChatMessages` 类型，可以分别编译。

### 一次对话

1. 窗口将输入交给 `AppController::sendMessage()`。
2. Core 忽略空消息和忙碌期间的请求；过长输入直接返回错误。接受输入后发出 `busyChanged(true)` 和 `messageAccepted`，窗口再展示“你：”并清空输入框。
3. Core 将保留的完整历史与本次 user 输入组成 `ChatMessages`，通过 `messageRequested` 交给 AiClient，后者转换为 JSON 并异步 POST 到 `/v1/chat/completions`。
4. 网络完成后解析 JSON，成功时取第一个 choice 的 `message.content`，失败时发出对应错误。
5. 成功时 Core 将本次输入与 assistant 回复成对加入历史并裁剪；失败时丢弃待提交输入，保留此前成功历史。两条路径均恢复可发送状态并转发结果，窗口追加回复或错误文字。

Core 使用 `src/common/chatmessage.h` 中的共享类型保存历史，AI 与 Core 不直接包含彼此的头文件。历史默认保留最近 8 个完整问答，消息内容采用 4096 个 UTF-8 字节预算；发送前为当前输入腾出空间，始终按完整轮次从最旧记录开始裁剪。请求副本的裁剪不影响此前成功历史，只有成功回复才提交新轮次。界面保留全部展示记录，模型只接收预算内的历史。

“新对话”调用 `newConversation()` 清空历史，通过 `conversationCleared` 通知窗口清空记录与输入框。生成期间按钮禁用，Core 也拒绝清空，避免旧回复进入新会话。会话不写入磁盘。当前不支持流式输出、取消请求和自动重试。

### 服务生命周期

入口在事件循环开始后调用 `ModelService::start()`。QProcess 的 started 信号只表示进程已启动，此时显示“模型加载中…”并请求 `/health`。请求成功且 HTTP 状态为 200 才显示“模型已就绪”。其余结果在进程仍运行时安排下一次检查，检查间隔默认 1000 ms；成功后不再轮询。

QProcess 的错误与退出信号在整个进程生命周期内有效，因此就绪后退出也会更新状态。崩溃错误保留具体原因，健康检查发现进程已停止时直接结束，避免通用停止提示覆盖错误。

尚未设置模型加载的总等待期限，也没有自动重启或就绪后的持续健康检查。窗口不根据服务状态禁止发送；提前发送由现有 HTTP 错误处理返回结果。

### 两个状态维度

`AppController::status()` 保存服务或演示状态，`isBusy()` 表示当前对话是否等待结果，两者独立。生成期间窗口显示“正在生成...”，并禁用发送按钮；后台服务状态仍会更新。生成结束后窗口显示最新保存的服务状态。窗口构造完成时主动刷新一次，以正确显示控制器已有的忙碌状态。

`startListening()` 只通过统一的 `setStatus()` 设置演示文字 `Listening...`，不表示录音已开始。保留这个入口是为了兼容原型调用。

## 对象所有权与退出

`main.cpp` 按配置、ModelService、AiClient、AppController、MainWindow 的顺序创建对象，退出时按逆序销毁；所有模块都先于 QApplication 销毁。窗口对控制器持有非拥有引用，控制器的生命期覆盖窗口。

控件和布局由 Qt 父子关系管理。两个 AI 对象分别拥有自己的 QNetworkAccessManager；ModelService 还拥有 QProcess。QProcess 对象的析构不应代替服务退出策略，因此 ModelService 先 terminate，最多等待 3000 ms，再在必要时 kill 并等待最多 1000 ms。

对进程结束的等待仅用于退出清理。运行期间的网络请求、健康检查和进程启动使用异步信号，不为推理新建线程，也不在窗口回调中等待。

网络 reply 在完成回调中调用 `deleteLater()`，每个结果分支都安排释放。所有捕获窗口控件或服务对象的连接与定时回调都有 QObject context，对象销毁时 Qt 自动断开关联。ModelService 退出清理前还断开进程到自身的状态回调，避免析构时继续发布进程状态。

## 共享参数

ModelConfig 的默认值集中定义在 [modelconfig.h](../src/ai/modelconfig.h)，由入口创建并传给两个 AI 对象；对象保存配置值。进程的模型、主机、端口与 HTTP 请求使用同一个来源，`endpoint()` 用 QUrl 生成各协议路径。

默认目录为用户主目录下的 `llama.cpp`，程序相对路径为 `build/bin/llama-server`。模型标识为 `Qwen/Qwen3-4B-GGUF:Q4_K_M`，地址为 `127.0.0.1:8080`，上下文容量 8192 token，回复上限 128 token。请求传输超时为 180000 ms，健康检查传输超时为 2000 ms；这是传输无数据活动的超时，不是推理或加载流程的总时限。

请求保留 `stream=false` 和 `chat_template_kwargs.enable_thinking=false`。JSON 配置示例尚未接入程序，详细约定见 [配置说明](../config/README.md)。

`maxHistoryTurns` 和 `maxConversationBytes` 分别控制历史轮数与消息内容字节预算。入口取字节预算与 `contextSize - maxTokens - 512` 的非负较小值传给 Core，512 为模板预留量。这是保守估算，字节不等于 token，模型服务仍可能返回上下文容量错误。

## 后续扩展边界

```text
用户操作 → UI → Core → AI 客户端 → llama-server
                    ↓
                 设备管理 → MQTT 客户端 → Mosquitto → 设备

设备消息 → MQTT 客户端 → 设备状态 → Core → UI
AI 工具请求 → Core 校验与分派 → 设备操作 → 执行结果 → AI/UI
```

- UI 不直接拼装 HTTP 请求、MQTT 消息或设备协议。
- AI 不直接依赖 MQTT，MQTT 不依赖 UI；设备模块负责状态和操作含义。
- 真实工具控制接入时，Core 校验工具白名单、参数和设备能力，区分命令发送、设备确认与实际执行成功。
- 只有存在实际替换或测试需要时才增加接口层；阻塞工作确有需要时才增加工作线程。
- 模型权重、外部 llama.cpp 源码、本机绝对路径和个人 IDE 配置不属于本仓库源码。

## 阅读路线

从 [开发指南的源码阅读与 Review](development.md#源码阅读与-review) 跟踪一次发送和回复，再单独观察模型服务启动。
