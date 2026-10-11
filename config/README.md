# 配置约定

`zoey.example.json` 是后续配置加载的 AI/MQTT 示例，当前程序尚未读取它。AI 已接入，但实际参数来自 [ModelConfig](../src/ai/modelconfig.h)，由 main 创建并同时传给进程服务和对话客户端。

## 当前实际参数

| 参数 | 默认值 |
| --- | --- |
| 模型标识 | `Qwen/Qwen3-4B-GGUF:Q4_K_M` |
| 服务主机与端口 | `127.0.0.1:8080` |
| llama.cpp 目录 | 用户主目录下的 `llama.cpp` |
| 服务程序相对路径 | `build/bin/llama-server` |
| 服务上下文容量 | `8192` token |
| 单次回复上限 | `128` token |
| 对话传输超时 | `180000` ms |
| 健康检查传输超时 | `2000` ms |
| 健康检查重试间隔 | `1000` ms |

URL 从同一主机和端口生成：`/health` 用于就绪检查，`/v1/chat/completions` 用于对话。传输超时表示无数据活动时的等待限制，不是模型加载或推理的总时限。修改默认参数时只需调整 ModelConfig；当前不支持运行中重新加载。

## JSON 示例约定

- `ai.base_url`：推理服务的 API 根地址，示例使用本机端口 8080。
- `ai.request_timeout_ms`：配置加载接入后的请求超时示例，单位毫秒；示例中的 `60000` 尚未生效，当前实际值为 `180000`。
- `mqtt.host` / `port`：Broker 地址；示例为本机 1883。
- `mqtt.client_id`：客户端标识；多实例接入时应使用不同值。
- `mqtt.topic_prefix`：示例 topic 前缀，须与设备固件约定。

实现配置加载后，本机配置放在 `config/local/`，并在配置加载文档中说明文件定位规则。该目录已被 Git 忽略；不要把个人路径、密码或模型权重提交到示例文件。

目录整理时，原有 Qt Creator `.user` 文件和 VS Code 本机设置备份到了 `config/local/`。这些文件供查阅，旧项目路径配置不应直接作为新构建配置使用。
