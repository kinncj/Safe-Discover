#include "aurbackend.h"
#include "core/commandrunner.h"
#include "core/logmanager.h"
#include "pacmanbackend.h"

AurBackend::AurBackend(CommandRunner *runner, QObject *parent)
    : PackageModel(parent)
    , m_runner(runner)
{
}

void AurBackend::search(const QString &query)
{
    if (query.isEmpty()) {
        setPackages({});
        return;
    }

    setSearchQuery(query);
    clearError();
    setLoading(true);

    auto result = m_runner->runSync(QStringLiteral("paru"), {QStringLiteral("-Ss"), query});

    if (result.exitCode == 0) {
        auto pkgs = PacmanBackend::parseSearchOutput(result.stdout);
        setPackages(pkgs);
    } else if (result.exitCode == 1) {
        setPackages({});
    } else {
        setError(QStringLiteral("Search failed: ") + result.stderr);
    }

    setLoading(false);
    LogManager::instance()->log(QStringLiteral("AUR search: %1 (%2 results)").arg(query).arg(count()));
}

void AurBackend::fetchDetails(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }

    const auto &pkg = packages().at(index);
    QString flag = pkg.installed ? QStringLiteral("-Qi") : QStringLiteral("-Si");
    auto result = m_runner->runSync(QStringLiteral("paru"), {flag, pkg.name});

    if (result.exitCode == 0) {
        auto details = PacmanBackend::parseDetailOutput(result.stdout);
        Q_EMIT detailsReady(index, details);
    }
}

void AurBackend::install(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    const auto &pkg = packages().at(index);
    Q_EMIT confirmationRequired(
        QStringLiteral("install"),
        pkg.name,
        QStringLiteral("Install %1 from AUR? This will open a terminal for the build process.").arg(pkg.name));
}

void AurBackend::remove(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    const auto &pkg = packages().at(index);
    Q_EMIT confirmationRequired(
        QStringLiteral("remove"),
        pkg.name,
        QStringLiteral("Remove %1? This will also check for orphaned dependencies.").arg(pkg.name));
}

void AurBackend::confirmInstall(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    const auto &pkg = packages().at(index);
    LogManager::instance()->log(QStringLiteral("Installing AUR package (terminal): %1").arg(pkg.name));

    // AUR installs use terminal mode — paru handles sudo internally
    m_runner->run(
        QStringLiteral("paru"),
        {QStringLiteral("-S"), QStringLiteral("--needed"), pkg.name},
        CommandRunner::Terminal);
}

void AurBackend::confirmRemove(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    const auto &pkg = packages().at(index);
    LogManager::instance()->log(QStringLiteral("Removing package: %1 (step 1/3)").arg(pkg.name));

    // Step 1: Remove the package with dependencies
    auto result = m_runner->runSync(
        QStringLiteral("pkexec"),
        {QStringLiteral("/usr/libexec/safe-discover-helper.sh"),
         QStringLiteral("pacman"), QStringLiteral("-Rns"), QStringLiteral("--noconfirm"), pkg.name},
        60000);

    if (result.exitCode == 0) {
        Q_EMIT removalStepCompleted(1, true, QStringLiteral("Package removed successfully"));
        // Step 2: Check for orphans
        checkOrphans();
    } else {
        Q_EMIT removalStepCompleted(1, false, QStringLiteral("Removal failed: ") + result.stderr);
    }
}

void AurBackend::checkOrphans()
{
    LogManager::instance()->log(QStringLiteral("Checking for orphan packages (step 2/3)"));

    auto result = m_runner->runSync(
        QStringLiteral("pacman"), {QStringLiteral("-Qdtq")});

    m_orphanPackages.clear();

    if (result.exitCode == 0 && !result.stdout.trimmed().isEmpty()) {
        m_orphanPackages = result.stdout.trimmed().split(QLatin1Char('\n'));
        Q_EMIT orphanPackagesChanged();
        Q_EMIT orphansFound(m_orphanPackages);
        Q_EMIT removalStepCompleted(2, true,
            QStringLiteral("Found %1 orphan package(s)").arg(m_orphanPackages.size()));
    } else {
        Q_EMIT orphanPackagesChanged();
        Q_EMIT removalStepCompleted(2, true, QStringLiteral("No orphan packages found"));
    }
}

void AurBackend::removeOrphans()
{
    if (m_orphanPackages.isEmpty()) {
        Q_EMIT removalStepCompleted(3, true, QStringLiteral("No orphans to remove"));
        return;
    }

    LogManager::instance()->log(
        QStringLiteral("Removing %1 orphan package(s) (step 3/3)").arg(m_orphanPackages.size()));

    QStringList args = {QStringLiteral("/usr/libexec/safe-discover-helper.sh"),
                        QStringLiteral("pacman"), QStringLiteral("-Rns"), QStringLiteral("--noconfirm")};
    args.append(m_orphanPackages);

    auto result = m_runner->runSync(QStringLiteral("pkexec"), args, 120000);

    if (result.exitCode == 0) {
        m_orphanPackages.clear();
        Q_EMIT orphanPackagesChanged();
        Q_EMIT removalStepCompleted(3, true, QStringLiteral("Orphan packages removed"));
        Q_EMIT operationFinished(true, QStringLiteral("Package and orphans removed successfully"));
    } else {
        Q_EMIT removalStepCompleted(3, false, QStringLiteral("Failed to remove orphans: ") + result.stderr);
        Q_EMIT operationFinished(false, QStringLiteral("Orphan removal failed"));
    }

    // Refresh search
    if (!searchQuery().isEmpty()) {
        search(searchQuery());
    }
}
