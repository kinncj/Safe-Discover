#include <QApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QIcon>

#include <KAboutData>
#include <KLocalizedContext>
#include <KLocalizedString>

#include "core/commandrunner.h"
#include "core/toolchecker.h"
#include "core/logmanager.h"
#include "core/packagemodel.h"
#include "backends/pacmanbackend.h"
#include "backends/aurbackend.h"
#include "backends/flatpakbackend.h"
#include "backends/firmwarebackend.h"
#include "backends/updatemanager.h"
#include "safediscoverconfig.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));

    KLocalizedString::setApplicationDomain("safe-discover");

    KAboutData aboutData(
        QStringLiteral("safe-discover"),
        i18n("Safe Discover"),
        QStringLiteral("0.2.0"),
        i18n("Browse, install, and update packages safely"),
        KAboutLicense::GPL_V3,
        i18n("(c) 2026 Kinn Coelho Juliao")
    );
    aboutData.setBugAddress("https://github.com/kinncj/Safe-Discover/issues");
    KAboutData::setApplicationData(aboutData);
    QApplication::setWindowIcon(QIcon::fromTheme(QStringLiteral("system-software-install")));
    QApplication::setDesktopFileName(QStringLiteral("ca.kinncj.SafeDiscover"));

    // Initialize singletons
    auto *config = SafeDiscoverConfig::self();
    auto *runner = CommandRunner::instance();
    auto *toolChecker = ToolChecker::instance();
    auto *logManager = LogManager::instance();

    QQmlApplicationEngine engine;

    // Register singletons
    qmlRegisterSingletonInstance("ca.kinncj.SafeDiscover", 1, 0, "CommandRunner", runner);
    qmlRegisterSingletonInstance("ca.kinncj.SafeDiscover", 1, 0, "ToolChecker", toolChecker);
    qmlRegisterSingletonInstance("ca.kinncj.SafeDiscover", 1, 0, "LogManager", logManager);
    qmlRegisterSingletonInstance("ca.kinncj.SafeDiscover", 1, 0, "Config", config);

    // Register backend models
    auto *pacmanBackend = new PacmanBackend(runner, &app);
    auto *aurBackend = new AurBackend(runner, &app);
    auto *flatpakBackend = new FlatpakBackend(runner, &app);
    auto *firmwareBackend = new FirmwareBackend(runner, &app);
    auto *updateManager = new UpdateManager(runner, pacmanBackend, aurBackend, flatpakBackend, firmwareBackend, &app);

    qmlRegisterSingletonInstance("ca.kinncj.SafeDiscover", 1, 0, "PacmanBackend", pacmanBackend);
    qmlRegisterSingletonInstance("ca.kinncj.SafeDiscover", 1, 0, "AurBackend", aurBackend);
    qmlRegisterSingletonInstance("ca.kinncj.SafeDiscover", 1, 0, "FlatpakBackend", flatpakBackend);
    qmlRegisterSingletonInstance("ca.kinncj.SafeDiscover", 1, 0, "FirmwareBackend", firmwareBackend);
    qmlRegisterSingletonInstance("ca.kinncj.SafeDiscover", 1, 0, "UpdateManager", updateManager);

    engine.rootContext()->setContextObject(new KLocalizedContext(&engine));
    engine.loadFromModule("ca.kinncj.SafeDiscover", "Main");

    if (engine.rootObjects().isEmpty()) {
        return -1;
    }

    return app.exec();
}
