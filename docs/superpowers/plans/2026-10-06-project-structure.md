# Zoey 目录整理实施计划

> **For agentic workers:** 使用当前会话直接实施，按任务记录并验证。

**Goal:** 将已有 Qt 原型整理为可从根目录构建、模块职责清楚的开发基础。

**Architecture:** 一个应用入口组装 MainWindow 与 AppController；窗口只负责交互，核心层管理状态。AI/设备/MQTT 先保留开发说明，接入时再创建实际类与目标。

**Tech Stack:** C++17、CMake 3.21+、Qt5/Qt6 Core/Widgets/Test。

**Spec:** `docs/architecture.md`。

## 约束

- 保持现有窗口与按钮的可见行为。
- 保留用户已有 README 内容及本机 Qt Creator 配置；仅调整文档排版、增加开发说明、停止跟踪个人配置。
- 不添加推理、MQTT、录音等尚未实现功能的空类。
- 直接整理当前工作目录；变更保持未提交，便于用户审阅。

## 任务

- [x] 1. 用独立生命周期复现验证旧窗口析构问题；写出核心与 UI 集成测试，验证缺失实现导致失败。
- [x] 2. 统一根 CMake/Presets，迁移入口与窗口，建立不依赖 Widgets 的核心库，移除旧入口及闲置 Designer 文件。
- [x] 3. 补齐模块说明、配置示例、README 构建指南与忽略规则；保留本机 IDE 配置但停止 Git 跟踪。
- [x] 4. 验证 Debug 测试、Release 无测试构建、安装及启动，审阅差异并更新任务完成记录。

## 接口与测试

`zoey::AppController(QObject *parent = nullptr)`、`QString status() const`、`void startListening()`、`void statusChanged(const QString &status)`。

`zoey::MainWindow(AppController &controller, QWidget *parent = nullptr)`。控制器引用必须比窗口活得更久。

核心测试验证状态变化通知和重复操作；界面测试验证按钮更新控制器、外部状态变化显示到窗口、非零初始内存下窗口构造/关闭/析构安全。

## 验证关注点

根目录构建能得到真实应用；旧缓存不干扰新构建；窗口不访问未初始化成员；核心不链接 Widgets；个人 IDE 文件留在磁盘；示例配置不被描述为已接入功能。

## 完成记录（2026-10-06）

- 环境：Linux ARM64、GCC 12.2、CMake 3.25.1、Qt 5.15.8。
- 整理前基础项目编译成功，但非零内存构造窗口后析构复现崩溃（退出码 139）；新的生命周期测试通过。
- 新核心测试在实现之前因缺少 `core/appcontroller.h` 编译失败；实现后两组测试、六个业务/生命周期用例通过。
- `cmake --preset debug`、Debug 编译及 `ctest --preset debug` 成功，CTest 2/2 通过。
- Release 预设以 `BUILD_TESTING=OFF` 成功编译；安装到临时目录并验证应用和安装版均能在 offscreen 平台保持运行，随后主动终止启动检查进程。
- JSON 文件可解析，个人 IDE 设置和历史构建缓存备份存在且被 Git 忽略；核心库编译依赖只有 Qt Core，没有 Widgets。
- 独立只读复核未发现功能或架构阻塞问题；复核指出计划完成状态尚未更新，已补齐本记录。
- Qt 6 与真实桌面会话未在本次验证；AI、MQTT、配置加载仍待后续实现。
- 所有修改保留在工作目录，未提交。
