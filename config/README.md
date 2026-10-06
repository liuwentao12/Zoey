# 配置约定

`zoey.example.json` 是后续 AI/MQTT 接入的配置示例，当前程序尚未读取它，也不会因此连接服务。

- `ai.base_url`：推理服务的 API 根地址，示例使用本机端口 8080。
- `ai.request_timeout_ms`：计划中的请求超时，单位毫秒。
- `mqtt.host` / `port`：Broker 地址；示例为本机 1883。
- `mqtt.client_id`：客户端标识；多实例接入时应使用不同值。
- `mqtt.topic_prefix`：示例 topic 前缀，须与设备固件约定。

实现配置加载后，本机配置放在 `config/local/`，并在配置加载文档中说明文件定位规则。该目录已被 Git 忽略；不要把个人路径、密码或模型权重提交到示例文件。

目录整理时，原有 Qt Creator `.user` 文件和 VS Code 本机设置备份到了 `config/local/`。这些文件供查阅，旧项目路径配置不应直接作为新构建配置使用。
