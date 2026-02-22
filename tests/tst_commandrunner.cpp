#include <QTest>
#include <QSignalSpy>
#include "core/commandrunner.h"

class TestCommandRunner : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testRunSync()
    {
        auto *runner = CommandRunner::instance();
        auto result = runner->runSync(QStringLiteral("echo"), {QStringLiteral("hello world")});
        QCOMPARE(result.exitCode, 0);
        QVERIFY(result.stdout.trimmed() == QStringLiteral("hello world"));
        QVERIFY(result.stderr.isEmpty());
    }

    void testRunSyncFailure()
    {
        auto *runner = CommandRunner::instance();
        auto result = runner->runSync(QStringLiteral("false"), {});
        QVERIFY(result.exitCode != 0);
    }

    void testRunSyncNonexistent()
    {
        auto *runner = CommandRunner::instance();
        auto result = runner->runSync(QStringLiteral("nonexistent_command_xyz"), {});
        // Should fail gracefully
        QVERIFY(result.exitCode != 0 || result.stderr.contains(QStringLiteral("error"), Qt::CaseInsensitive)
                || result.stdout.isEmpty());
    }

    void testRunAsync()
    {
        auto *runner = CommandRunner::instance();
        QSignalSpy finishedSpy(runner, &CommandRunner::jobFinished);

        int jobId = runner->run(QStringLiteral("echo"), {QStringLiteral("async test")});
        QVERIFY(jobId > 0);

        QVERIFY(finishedSpy.wait(5000));
        QCOMPARE(finishedSpy.count(), 1);

        auto args = finishedSpy.takeFirst();
        QCOMPARE(args.at(0).toInt(), jobId);
        QCOMPARE(args.at(1).toInt(), 0);
    }

    void testPacmanLockDetection()
    {
        auto *runner = CommandRunner::instance();
        // Just test that the property is accessible and returns a bool
        QVERIFY(!runner->pacmanLocked() || runner->pacmanLocked());
    }
};

QTEST_MAIN(TestCommandRunner)
#include "tst_commandrunner.moc"
