# 开发指南

## 从根目录构建

工具要求：CMake 3.21+、支持 C++17 的编译器、Make、Qt 5 或 Qt 6 的 Core/Widgets/Network 开发组件。执行测试需安装同版本 Qt Test。项目构建不下载 Qt、llama.cpp 或模型，也不依赖 Qt MQTT；运行时 ModelService 的 `-hf` 参数由外部 llama-server 解析，模型缓存由该服务管理。

以下命令供自行构建与运行时使用。

```bash
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
./build/debug/src/Zoey
```

`debug` 使用 `build/debug/`，`release` 使用 `build/release/`。测试在 offscreen 平台运行，无需桌面显示；正常启动窗口则需要可用桌面会话。

不使用预设时，可以自由选择新的构建目录：

```bash
cmake -S . -B build/custom -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF
cmake --build build/custom --parallel 2
```

Release 预设默认关闭测试；独立关闭测试时不会要求 Qt Test。安装示例：

```bash
cmake --preset release
cmake --build --preset release --parallel 2
cmake --install build/release --prefix /tmp/zoey-install
/tmp/zoey-install/bin/Zoey
```

该安装规则目前只安装可执行文件，不打包 Qt 运行库或外部服务；目标系统需具备相应依赖。

## 编辑器

Qt Creator 打开根 `CMakeLists.txt`，为新结构选择 Kit 并使用新的构建目录。原有 `.user` 文件已备份到 `config/local/qtcreator-CMakeLists.txt.user`，保留供查阅，由 Qt Creator 重新生成当前项目的个人配置。

VS Code 打开 Zoey 根目录。共享设置选择 CMake Presets，clangd 指向相对路径 `build/debug/`。先执行 Debug 配置生成 `compile_commands.json`；切换为其他构建目录时，在个人设置中调整路径。原本包含绝对路径的设置备份在 `config/local/vscode-settings.json`。

旧根目录 `build/` 缓存仍保留；原 `ui/build-ZoeyUI-Desktop-Debug/` 已归档到 `build/legacy-qtcreator/`。缓存包含旧绝对路径，只作为历史产物保留，不应用来构建新结构。确认不再需要后可自行清理。

## 添加功能

1. 阅读 [架构文档](architecture.md)，确认逻辑属于哪个模块。
2. 在对应目录实现一个可运行流程；不要先生成一批空 Manager 类。
3. 把源码加入对应 `CMakeLists.txt`。新模块有真实代码时再增加模块库和依赖。
4. 在 `main.cpp` 创建并组装模块，通过 Core 向 UI 提供业务操作和状态通知。
5. 业务层不链接 Qt Widgets；网络请求不阻塞 UI 线程。
6. 对状态变化、错误处理、协议转换等真实行为增加测试，在根目录执行 `ctest --preset debug`。

当前已有多轮异步对话、会话裁剪、新对话和界面错误展示。后续可根据需求依次补充流式输出、设备模型与 MQTT、工具调用校验和设备执行结果。语音、RAG 和自动化在基础流程稳定后再拆分。

## 源码阅读与 Review

先打开 [main.cpp](../src/main.cpp)，看对象创建顺序和 `QObject::connect`。这里的连接决定了信号最终进入哪个模块。在 Qt Creator 用 Debug 预设启动，等界面显示“模型已就绪”，发一句“我叫小周”，再问“我叫什么？”。按下表顺序跟踪，第二轮最适合观察历史如何进入 HTTP 请求。

| 阶段 | 断点或观察位置 | 看什么 |
| --- | --- | --- |
| 点击发送 | [MainWindow 的发送按钮回调](../src/ui/mainwindow.cpp#L61) | 输入框内容如何交给 Core，以及何时清空 |
| 组装本轮对话 | [`AppController::sendMessage()`](../src/core/appcontroller.cpp#L63) | `m_pendingMessage`、`m_busy` 和送给模型的 `messages`。第二轮请求应带有第一轮完整问答 |
| 编码和解析 HTTP | [`AiClient::createRequestBody()`](../src/ai/aiclient.cpp#L38) 与 [`handleFinishedReply()`](../src/ai/aiclient.cpp#L62) | `ChatMessages` 如何变成 JSON，以及网络或解析失败时发出哪个信号 |
| 更新会话和界面 | [`AppController::handleReply()`](../src/core/appcontroller.cpp#L103) 或 [`handleError()`](../src/core/appcontroller.cpp#L119) | 成功才追加问答，成功和失败都解除忙碌状态 |

服务启动可以单独跟踪。先看 [`ModelService::start()`](../src/ai/modelservice.cpp#L58) 如何用 [`ModelConfig`](../src/ai/modelconfig.h) 拼装启动参数，再看 [`checkHealth()`](../src/ai/modelservice.cpp#L78) 在什么条件下发出“模型已就绪”。界面里的服务状态和生成中的忙碌状态各自更新。

Review 时可以从改动影响的流程往回查：

- **窗口操作**：确认点击发送后由 Core 决定是否接受，界面在收到 `messageAccepted` 后再清空输入。
- **会话和失败路径**：检查裁剪按完整问答进行。失败的本轮输入不进入历史，忙碌时发第二条消息或点“新对话”都由 Core 拦下。
- **服务或网络改动**：检查回调的 QObject 生命周期、`QNetworkReply` 的释放，以及成功与错误两条路径是否都能结束本次请求。
- **MQTT 接入**：先沿用现有组装方式。设备层管理设备状态和命令结果，MQTT 层处理 topic 和收发，Core 协调界面操作。职责和依赖见 [架构文档](architecture.md#后续扩展边界)。

## 现有测试

- `appcontroller`：状态变化通知、重复请求不重复通知。
- `mainwindow`：验证发送按钮提交消息并清空输入，以及外部状态更新、初始显示和窗口关闭析构。

这些用例可在没有 llama-server 和模型的环境中运行。HTTP 请求和本地推理目前通过上面的手动流程检查。

`Listening...` 仍是原型演示状态，尚未实现录音；真实推理通过 AiClient 的 HTTP 请求进行，与该入口无关。
