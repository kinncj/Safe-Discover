#include <QTest>
#include "backends/pacmanbackend.h"

class TestPacmanBackend : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testParseSearchOutput()
    {
        QString output = QStringLiteral(
            "core/linux 6.7.1-1 [installed]\n"
            "    The Linux kernel and modules\n"
            "extra/firefox 122.0-1\n"
            "    Fast, Private & Safe Web Browser\n"
            "community/firefox-developer-edition 122.0b9-1 [installed: 122.0b8-1]\n"
            "    Firefox Developer Edition\n"
        );

        auto packages = PacmanBackend::parseSearchOutput(output);

        QCOMPARE(packages.size(), 3);

        // First package
        QCOMPARE(packages[0].name, QStringLiteral("linux"));
        QCOMPARE(packages[0].repository, QStringLiteral("core"));
        QCOMPARE(packages[0].version, QStringLiteral("6.7.1-1"));
        QVERIFY(packages[0].installed);
        QCOMPARE(packages[0].installedVersion, QStringLiteral("6.7.1-1"));
        QCOMPARE(packages[0].description, QStringLiteral("The Linux kernel and modules"));

        // Second package
        QCOMPARE(packages[1].name, QStringLiteral("firefox"));
        QCOMPARE(packages[1].repository, QStringLiteral("extra"));
        QVERIFY(!packages[1].installed);
        QCOMPARE(packages[1].description, QStringLiteral("Fast, Private & Safe Web Browser"));

        // Third package - different installed version
        QCOMPARE(packages[2].name, QStringLiteral("firefox-developer-edition"));
        QVERIFY(packages[2].installed);
        QCOMPARE(packages[2].installedVersion, QStringLiteral("122.0b8-1"));
    }

    void testParseSearchOutputEmpty()
    {
        auto packages = PacmanBackend::parseSearchOutput(QString());
        QCOMPARE(packages.size(), 0);
    }

    void testParseDetailOutput()
    {
        QString output = QStringLiteral(
            "Repository      : extra\n"
            "Name            : firefox\n"
            "Version         : 122.0-1\n"
            "Description     : Fast, Private & Safe Web Browser\n"
            "Architecture    : x86_64\n"
            "URL             : https://www.mozilla.org/firefox/\n"
            "Licenses        : MPL-2.0\n"
            "Depends On      : dbus  gtk3  libxt  mime-types  nss\n"
            "Download Size   : 82.50 MiB\n"
            "Installed Size  : 247.99 MiB\n"
        );

        auto details = PacmanBackend::parseDetailOutput(output);

        QVERIFY(!details.isEmpty());
        QCOMPARE(details.value(QStringLiteral("Name")).toString(), QStringLiteral("firefox"));
        QCOMPARE(details.value(QStringLiteral("Version")).toString(), QStringLiteral("122.0-1"));
        QCOMPARE(details.value(QStringLiteral("Repository")).toString(), QStringLiteral("extra"));
        QVERIFY(details.contains(QStringLiteral("Depends On")));
        QVERIFY(details.contains(QStringLiteral("Download Size")));
    }

    void testParseDetailOutputEmpty()
    {
        auto details = PacmanBackend::parseDetailOutput(QString());
        QVERIFY(details.isEmpty());
    }
};

QTEST_MAIN(TestPacmanBackend)
#include "tst_pacmanbackend.moc"
