#include "core/appcontroller.h"

#include <QSignalSpy>
#include <QtTest>

class AppControllerTest : public QObject
{
    Q_OBJECT

private slots:
    void listeningPublishesChangedState()
    {
        zoey::AppController controller;
        const QString initialStatus = controller.status();
        QSignalSpy changes(&controller, &zoey::AppController::statusChanged);

        controller.startListening();

        QVERIFY(controller.status() != initialStatus);
        QCOMPARE(changes.count(), 1);
        QCOMPARE(changes.at(0).at(0).toString(), controller.status());
    }

    void repeatedRequestDoesNotPublishDuplicateState()
    {
        zoey::AppController controller;
        controller.startListening();
        QSignalSpy changes(&controller, &zoey::AppController::statusChanged);

        controller.startListening();

        QCOMPARE(changes.count(), 0);
    }
};

QTEST_GUILESS_MAIN(AppControllerTest)
#include "tst_appcontroller.moc"
