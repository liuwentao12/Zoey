# Zoey

Zoey 是一个运行在 ROCK 5 ITX 上的本地 AI + IoT 边缘智能中枢，基于 Qt/C++ 开发。

项目目标是将本地大模型、MQTT 设备通信和自动化控制整合到统一应用中，使 ESP32、Zephyr 设备及其他 IoT 节点能够接入 Zoey，并由 AI 获取设备状态、理解用户指令并执行相应操作。

## Architecture

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
- MQTT IoT 节点接入
- 设备状态统一管理
- AI 获取设备信息
- AI Tool Calling 控制设备
- 后续支持语音、RAG、自动化和 RK3588 NPU 加速

## Current Progress

- [x] Zoey Qt 基础界面
- [x] 统一构建入口、模块目录与基础回归测试
- [x] ROCK 5 ITX 本地部署 llama.cpp
- [x] Qwen3-4B Q4_K_M 本地推理
- [x] OpenAI Compatible HTTP API
- [ ] Qt 自动启动 / 管理 AI 服务
- [ ] Qt AI 对话界面
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
│   ├── ai/                 # AI 接入开发约定
│   ├── devices/            # 设备管理开发约定
│   └── mqtt/               # MQTT 接入开发约定
├── config/                 # 共享配置示例
├── tests/                  # 核心与界面测试
├── docs/                   # 架构和开发指南
└── build/                  # 编译产物（不提交）
```

当前仓库实现基础 Qt 窗口和 `AppController` 状态协调；AI、设备与 MQTT 目录暂时只有开发说明。上面的完整系统架构是后续目标。`Listening...` 为演示状态，尚未启动录音或推理。此前本机部署的 llama.cpp 与模型属于独立服务，不等于本仓库已完成客户端接入。

## 构建与运行

依赖 CMake 3.21+、C++17 编译器和 Qt 5/6 Core、Widgets；Debug 测试额外需要 Qt Test。以下命令均在项目根目录执行：

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
