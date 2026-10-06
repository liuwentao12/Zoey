#pragma once

#include <QMainWindow>

namespace zoey {

class AppController;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // The controller must outlive this window; the window does not own it.
    explicit MainWindow(AppController &controller, QWidget *parent = nullptr);
};

} // namespace zoey
