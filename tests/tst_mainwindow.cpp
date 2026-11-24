#include "core/appcontroller.h"
#include "ui/mainwindow.h"

#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSignalSpy>
#include <QtTest>
#include <cstring>
#include <new>

class MainWindowTest : public QObject
{
    Q_OBJECT

private slots:
    void sendButtonSubmitsMessageAndClearsInput()
    {
        zoey::AppController controller;
        zoey::MainWindow window(controller);
        auto *button = window.findChild<QPushButton *>("sendButton");
        auto *input = window.findChild<QLineEdit *>("inputEdit");
        QVERIFY(button);
        QVERIFY(input);
        QSignalSpy requests(&controller, &zoey::AppController::messageRequested);
        QVERIFY(requests.isValid());

        input->setText(QStringLiteral("你好"));
        button->click();

        QCOMPARE(requests.count(), 1);
        const auto messages = requests.at(0).at(0).value<zoey::ChatMessages>();
        QCOMPARE(messages.size(), 1);
        QCOMPARE(messages.first().role, zoey::ChatMessage::Role::User);
        QCOMPARE(messages.first().content, QStringLiteral("你好"));
        QVERIFY(input->text().isEmpty());
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
