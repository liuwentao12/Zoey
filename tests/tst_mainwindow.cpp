#include "core/appcontroller.h"
#include "ui/mainwindow.h"

#include <QLabel>
#include <QPushButton>
#include <QSignalSpy>
#include <QtTest>
#include <cstring>
#include <new>

class MainWindowTest : public QObject
{
    Q_OBJECT

private slots:
    void buttonUpdatesApplicationState()
    {
        zoey::AppController controller;
        zoey::MainWindow window(controller);
        auto *button = window.findChild<QPushButton *>("statusButton");
        auto *label = window.findChild<QLabel *>("statusLabel");
        QVERIFY(button);
        QVERIFY(label);
        const QString initialStatus = label->text();
        QSignalSpy changes(&controller, &zoey::AppController::statusChanged);

        button->click();

        QCOMPARE(changes.count(), 1);
        QVERIFY(label->text() != initialStatus);
        QCOMPARE(label->text(), controller.status());
    }

    void externalStateChangeUpdatesWindow()
    {
        zoey::AppController controller;
        zoey::MainWindow window(controller);
        auto *label = window.findChild<QLabel *>("statusLabel");
        QVERIFY(label);
        const QString initialStatus = label->text();

        controller.startListening();

        QVERIFY(label->text() != initialStatus);
        QCOMPARE(label->text(), controller.status());
    }

    void newWindowDisplaysExistingState()
    {
        zoey::AppController controller;
        controller.startListening();
        zoey::MainWindow window(controller);
        auto *label = window.findChild<QLabel *>("statusLabel");
        QVERIFY(label);
        QCOMPARE(label->text(), controller.status());
    }

    void windowCanCloseAndBeDestroyed()
    {
        zoey::AppController controller;
        // Reproduce the old destructor bug without relying on zero-filled memory.
        alignas(zoey::MainWindow) unsigned char storage[sizeof(zoey::MainWindow)];
        std::memset(storage, 0xa5, sizeof(storage));
        auto *window = new (storage) zoey::MainWindow(controller);
        window->show();
        QVERIFY(window->close());
        window->~MainWindow();

        // The controller is owned by main(), not by the window.
        controller.startListening();
    }
};

QTEST_MAIN(MainWindowTest)
#include "tst_mainwindow.moc"
