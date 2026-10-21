# Zoey — Local AI and IoT Edge Hub

**Experimental, early-stage | C++17 · Qt · ARM Linux · llama.cpp**

Zoey is a desktop application for exploring **on-device AI** on ARM Linux boards such as the ROCK 5 ITX (RK3588). It connects a Qt interface to a locally managed llama.cpp inference server. Its longer-term direction is to connect embedded sensor and actuator nodes through MQTT, but **IoT device integration is not yet implemented**.

## What works today

- Qt desktop interface and asynchronous, single-turn HTTP chat with a local model
- `ModelService` process management for starting, monitoring, and stopping `llama-server`
- CMake Debug/Release build presets, module architecture documentation, and a test scaffold
- Local Qwen3-4B inference via an OpenAI-compatible HTTP endpoint

**Not yet implemented:** streaming chat, conversation context reuse, MQTT broker/device management, AI tool calling, voice input, and RK3588 NPU inference. The UI test still targets a retired widget; do not interpret the current test suite as fully passing.

## System overview

```text
Qt UI (MainWindow)
         |
   AppController
     /       \
ModelService  AiClient
     |          |
llama-server <-- HTTP
     |
 Local GGUF model

Planned (not connected): MQTT Manager -> ESP32 / Zephyr / other IoT nodes
```

## Build and explore

Requirements: CMake 3.21+, C++17 compiler, Qt 5/6 (Core, Widgets, Network). Qt Test is additionally required for Debug tests. A compatible llama.cpp server binary and local GGUF model are needed for inference.

```bash
cmake --preset debug
cmake --build --preset debug --parallel 2
./build/debug/src/Zoey
```

To run the existing tests (some UI tests need maintenance):

```bash
ctest --preset debug
```

Read [architecture](docs/architecture.md), [development guide](docs/development.md), and [configuration notes](config/README.md) before adapting model paths, ports, or dependencies. The current application does not automatically read the sample config files.

## Contributing and project status

This is a maintainer-led prototype, **not** a production-ready AI assistant or a security-hardened remote-control system. Reproducible bug reports, documentation corrections, platform test results, and small reviewable patches are welcome. Please open a GitHub issue to discuss larger changes before investing in implementation.

No open-source license has yet been selected for the repository. Public visibility alone does not grant reuse or redistribution rights; do not assume a license until the maintainer publishes one. Dependencies and model weights have their own licenses.

---

## 中文项目说明

Zoey 是一个运行在 ROCK 5 ITX 上的本地 AI + IoT 边缘智能中枢，基于 Qt/C++ 开发。

项目目标是将本地大模型、MQTT 设备通信和自动化控制整合到统一应用中，使 ESP32、Zephyr 设备及其他 IoT 节点能够接入 Zoey，并由 AI 获取设备状态、理解用户指令并执行相应操作。

## Architecture

当前已实现的流程：

```text
MainWindow → AppController → AiClient → llama-server
     ↑             ↑                       ↑
     └── 状态/结果 ──┘                  ModelService
                                      启动/健康检查/退出
```

`main.cpp` 组装模块；`ModelConfig` 为服务启动与对话请求提供同一份默认参数。以下是后续 AI + IoT 的目标架构：

```text
                    Zoey
               ROCK 5 ITX
                    │
                 Qt UI
                    │
                Zoey Core
             ┌──────┴──────┐
             │             │
        AI Manager    Device Manager
             │             │
       llama-server   MQTT Manager
             │             │
        Qwen3-4B       Mosquitto
                           │
                ┌──────────┼──────────┐
                ▼          ▼          ▼
             ESP32     Sentinel    Other Nodes
```

## Features

- Qt/C++ 桌面应用
- ROCK 5 ITX 本地运行
- Qwen3-4B 本地大模型
- llama.cpp 推理服务
- 本地 HTTP API
- 自动启动模型服务、加载健康检查和进程退出状态通知
- 单次非流式对话、忙碌提示和请求错误展示
- 后续规划：MQTT IoT 节点接入、设备状态管理、AI 获取设备信息和 Tool Calling
- 后续规划：语音、RAG、自动化和 RK3588 NPU 加速

## Current Progress

- [x] Zoey Qt 基础界面
- [x] 统一构建入口、模块目录与基础回归测试
- [x] ROCK 5 ITX 本地部署 llama.cpp
- [x] Qwen3-4B Q4_K_M 本地推理
- [x] OpenAI Compatible HTTP API
- [x] Qt 自动启动 / 管理 AI 服务
- [x] Qt AI 对话界面
- [ ] Mosquitto Broker
- [ ] MQTT Manager
- [ ] Device Manager
- [ ] Zephyr-Sentinel 接入
- [ ] AI Tool Calling
- [ ] RK3588 NPU 推理

## Tech Stack

`C++` · `Qt` · `Linux` · `MQTT` · `Mosquitto` · `llama.cpp` · `Qwen3` · `RK3588` · `Zephyr`

## Goal

Zoey 最终将成为一个可扩展的个人边缘 AI 系统：

```text
感知设备状态
     ↓
理解环境信息
     ↓
AI 推理与决策
     ↓
调用工具
     ↓
控制 IoT 设备
```

> Local AI. Local Devices. Local Intelligence.

## 项目目录

```text
Zoey/
├── CMakeLists.txt          # 项目统一构建入口
├── CMakePresets.json       # Debug / Release 构建配置
├── src/
│   ├── main.cpp            # 程序入口与模块组装
│   ├── ui/                 # 窗口与用户交互
│   ├── core/               # 应用状态与业务协调
│   ├── ai/                 # 模型参数、进程管理与 HTTP 对话客户端
│   ├── devices/            # 设备管理开发约定
│   └── mqtt/               # MQTT 接入开发约定
├── config/                 # 共享配置示例
├── tests/                  # 核心与界面测试
├── docs/                   # 架构和开发指南
└── build/                  # 编译产物（不提交）
```

当前仓库已实现 Qt 对话界面、`AppController` 协调、llama-server 自动启动和异步 HTTP 对话。每次请求只发送本次输入，界面的聊天记录不会成为下一次请求的模型上下文。设备与 MQTT 目录仍只有开发约定；`startListening()` 保留为原型演示入口，尚未实现录音。

默认模型、服务地址、路径及超时统一位于 [ModelConfig](src/ai/modelconfig.h)。服务就绪前发送消息仍会走 HTTP 请求及错误处理；尚未实现对话取消、服务自动重启和流式输出。

## 构建与运行

依赖 CMake 3.21+、C++17 编译器和 Qt 5/6 Core、Widgets、Network；Debug 测试额外需要 Qt Test。以下命令供手动构建时在项目根目录执行：

```bash
cmake --preset debug
cmake --build --preset debug --parallel 2
ctest --preset debug
./build/debug/src/Zoey
```

Qt Creator 打开根目录的 `CMakeLists.txt`。Release 构建：

```bash
cmake --preset release
cmake --build --preset release --parallel 2
./build/release/src/Zoey
```

开发前请阅读 [架构与模块边界](docs/architecture.md) 和 [开发指南](docs/development.md)。配置约定见 [config/README.md](config/README.md)；示例目前尚未由应用读取。

现有界面测试仍查找旧的 `statusButton`，需后续适配对话界面；详见开发指南。
