#include <QTest>
#include "backends/flatpakbackend.h"

class TestFlatpakBackend : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testParseSearchOutput()
    {
        // flatpak search output is tab-delimited:
        // Name\tDescription\tApplication ID\tVersion\tBranch\tRemotes
        QString output = QStringLiteral(
            "Firefox\tFast, Private & Safe Web Browser\torg.mozilla.firefox\t122.0\tstable\tflathub\n"
            "Firefox ESR\tExtended Support Release\torg.mozilla.firefox.ESR\t115.7.0\tstable\tflathub\n"
        );

        auto packages = FlatpakBackend::parseSearchOutput(output);

        QCOMPARE(packages.size(), 2);

        QCOMPARE(packages[0].name, QStringLiteral("org.mozilla.firefox"));
        QCOMPARE(packages[0].description, QStringLiteral("Fast, Private & Safe Web Browser"));
        QCOMPARE(packages[0].version, QStringLiteral("122.0"));
        QCOMPARE(packages[0].repository, QStringLiteral("flathub"));

        QCOMPARE(packages[1].name, QStringLiteral("org.mozilla.firefox.ESR"));
        QCOMPARE(packages[1].description, QStringLiteral("Extended Support Release"));
    }

    void testParseSearchOutputEmpty()
    {
        auto packages = FlatpakBackend::parseSearchOutput(QString());
        QCOMPARE(packages.size(), 0);
    }

    void testParseSearchOutputMalformed()
    {
        // Lines with fewer than 3 tab-delimited fields should be skipped
        QString output = QStringLiteral(
            "NotEnoughFields\tOnly two\n"
            "Firefox\tGood Browser\torg.mozilla.firefox\t122.0\tstable\tflathub\n"
        );

        auto packages = FlatpakBackend::parseSearchOutput(output);
        QCOMPARE(packages.size(), 1);
        QCOMPARE(packages[0].name, QStringLiteral("org.mozilla.firefox"));
    }
};

QTEST_MAIN(TestFlatpakBackend)
#include "tst_flatpakbackend.moc"
