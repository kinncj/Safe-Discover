#include "flatpakbackend.h"
#include "core/commandrunner.h"
#include "core/logmanager.h"

FlatpakBackend::FlatpakBackend(CommandRunner *runner, QObject *parent)
    : PackageModel(parent)
    , m_runner(runner)
{
    loadRemotes();
}

void FlatpakBackend::loadRemotes()
{
    auto result = m_runner->runSync(
        QStringLiteral("flatpak"),
        {QStringLiteral("remotes"), QStringLiteral("--columns=name")});

    m_remotes.clear();
    if (result.exitCode == 0) {
        const auto lines = result.stdout.trimmed().split(QLatin1Char('\n'));
        for (const auto &line : lines) {
            QString remote = line.trimmed();
            if (!remote.isEmpty()) {
                m_remotes.append(remote);
            }
        }
    }

    if (m_remotes.isEmpty()) {
        m_remotes.append(QStringLiteral("flathub"));
    }

    if (!m_remotes.contains(m_activeRemote)) {
        m_activeRemote = m_remotes.first();
    }

    Q_EMIT remotesChanged();
}

void FlatpakBackend::setActiveRemote(const QString &remote)
{
    if (m_activeRemote != remote) {
        m_activeRemote = remote;
        Q_EMIT activeRemoteChanged();
    }
}

void FlatpakBackend::search(const QString &query)
{
    if (query.isEmpty()) {
        setPackages({});
        return;
    }

    setSearchQuery(query);
    clearError();
    setLoading(true);

    auto result = m_runner->runSync(
        QStringLiteral("flatpak"),
        {QStringLiteral("search"),
         QStringLiteral("--columns=name,description,application,version,branch,remotes"),
         query});

    if (result.exitCode == 0) {
        auto pkgs = parseSearchOutput(result.stdout);
        setPackages(pkgs);
    } else {
        setPackages({});
        if (result.exitCode != 1) {
            setError(QStringLiteral("Search failed: ") + result.stderr);
        }
    }

    setLoading(false);
    LogManager::instance()->log(QStringLiteral("Flatpak search: %1 (%2 results)").arg(query).arg(count()));
}

void FlatpakBackend::fetchDetails(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }

    const auto &pkg = packages().at(index);

    // Try installed info first, then remote info
    auto result = m_runner->runSync(
        QStringLiteral("flatpak"),
        {QStringLiteral("info"), pkg.name});

    QVariantMap details;

    if (result.exitCode == 0) {
        // Parse key-value output
        const auto lines = result.stdout.split(QLatin1Char('\n'));
        for (const auto &line : lines) {
            int colonIdx = line.indexOf(QLatin1Char(':'));
            if (colonIdx > 0) {
                QString key = line.left(colonIdx).trimmed();
                QString value = line.mid(colonIdx + 1).trimmed();
                details.insert(key, value);
            }
        }
        details.insert(QStringLiteral("Installed"), true);
    } else {
        // Try remote info
        auto remoteResult = m_runner->runSync(
            QStringLiteral("flatpak"),
            {QStringLiteral("remote-info"), m_activeRemote, pkg.name});

        if (remoteResult.exitCode == 0) {
            const auto lines = remoteResult.stdout.split(QLatin1Char('\n'));
            for (const auto &line : lines) {
                int colonIdx = line.indexOf(QLatin1Char(':'));
                if (colonIdx > 0) {
                    QString key = line.left(colonIdx).trimmed();
                    QString value = line.mid(colonIdx + 1).trimmed();
                    details.insert(key, value);
                }
            }
        }
        details.insert(QStringLiteral("Installed"), false);
    }

    Q_EMIT detailsReady(index, details);
}

void FlatpakBackend::install(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    const auto &pkg = packages().at(index);
    Q_EMIT confirmationRequired(
        QStringLiteral("install"),
        pkg.name,
        QStringLiteral("Install %1 from %2?").arg(pkg.name, m_activeRemote));
}

void FlatpakBackend::remove(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    const auto &pkg = packages().at(index);
    Q_EMIT confirmationRequired(
        QStringLiteral("remove"),
        pkg.name,
        QStringLiteral("Remove %1?").arg(pkg.name));
}

void FlatpakBackend::confirmInstall(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    const auto &pkg = packages().at(index);
    LogManager::instance()->log(QStringLiteral("Installing flatpak: %1 from %2").arg(pkg.name, m_activeRemote));

    m_runner->run(
        QStringLiteral("flatpak"),
        {QStringLiteral("install"), QStringLiteral("-y"), m_activeRemote, pkg.name});
}

void FlatpakBackend::confirmRemove(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    const auto &pkg = packages().at(index);
    LogManager::instance()->log(QStringLiteral("Removing flatpak: %1").arg(pkg.name));

    m_runner->run(
        QStringLiteral("flatpak"),
        {QStringLiteral("uninstall"), QStringLiteral("-y"), pkg.name});
}

QList<PackageInfo> FlatpakBackend::parseSearchOutput(const QString &output)
{
    QList<PackageInfo> packages;

    // flatpak search output is tab-delimited:
    // Name\tDescription\tApplication ID\tVersion\tBranch\tRemotes
    const auto lines = output.trimmed().split(QLatin1Char('\n'));

    for (const auto &line : lines) {
        if (line.trimmed().isEmpty()) {
            continue;
        }

        const auto fields = line.split(QLatin1Char('\t'));
        if (fields.size() < 3) {
            continue;
        }

        PackageInfo pkg;
        pkg.description = fields.value(0).trimmed();  // Name column is display name
        if (fields.size() > 1) {
            pkg.description = fields.value(1).trimmed();
        }
        pkg.name = fields.value(2).trimmed();  // Application ID
        pkg.version = fields.value(3, QString()).trimmed();
        pkg.repository = fields.value(5, QStringLiteral("flathub")).trimmed();

        // Check if installed
        auto checkResult = CommandRunner::instance()->runSync(
            QStringLiteral("flatpak"),
            {QStringLiteral("info"), pkg.name});
        pkg.installed = (checkResult.exitCode == 0);

        if (!pkg.name.isEmpty()) {
            packages.append(pkg);
        }
    }

    return packages;
}
