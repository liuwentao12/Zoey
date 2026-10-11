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

当前已有单次异步对话和界面错误展示。后续可根据需求依次补充对话历史与流式输出、设备模型与 MQTT、工具调用校验和设备执行结果。语音、RAG 和自动化在基础流程稳定后再拆分。

## 源码阅读与复盘路线

1. [main.cpp](../src/main.cpp)：先看模块创建顺序、共享配置和四条信号连接，理解入口如何组装应用。
2. [AppController](../src/core/appcontroller.cpp)：沿发送、回复和错误路径看 busy 如何变化，再看状态去重。
3. [MainWindow](../src/ui/mainwindow.cpp)：看控件所有权、统一刷新 lambda、发送回调和结果展示。
4. [ModelConfig](../src/ai/modelconfig.h)：定位默认地址、路径、模型及超时；JSON 示例尚未加载。
5. [AiClient](../src/ai/aiclient.cpp)：按请求构造、异步发送、完成解析阅读，注意每个提前返回与 reply 的释放。
6. [ModelService](../src/ai/modelservice.cpp)：区分进程启动和模型就绪，再看健康检查重试、进程退出和析构清理。

复盘一次对话时，跟踪 `busyChanged(true) → messageRequested → AI 结果 → busyChanged(false) → Core 转发结果`；检查一次启动时，跟踪 `started → /health → statusChanged`。窗口中的历史记录只用于展示，目前每个 HTTP 请求只携带本次输入。

## 现有测试

- `appcontroller`：状态变化通知、重复请求不重复通知。
- `mainwindow`：包含旧状态按钮用例、外部状态更新界面、已有状态的初始显示、窗口构造关闭与析构。

`mainwindow` 中的旧按钮用例仍查找已移除的 `statusButton`，需要适配当前对话界面。现有测试尚未覆盖 HTTP 对话流程。

`Listening...` 仍是原型演示状态，尚未实现录音；真实推理通过 AiClient 的 HTTP 请求进行，与该入口无关。
