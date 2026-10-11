#pragma once

#include <QMainWindow>

namespace zoey {

class AppController;

// 只负责展示和转发用户操作；通过 AppController 获取业务状态与对话结果。
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // 控制器必须比窗口活得更久；窗口仅引用它，不负责释放。
    explicit MainWindow(AppController &controller, QWidget *parent = nullptr);
};

} // namespace zoey
