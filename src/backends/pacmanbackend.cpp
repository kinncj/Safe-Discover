#include "pacmanbackend.h"
#include "core/commandrunner.h"
#include "core/logmanager.h"

#include <QRegularExpression>

PacmanBackend::PacmanBackend(CommandRunner *runner, QObject *parent)
    : PackageModel(parent)
    , m_runner(runner)
{
    connect(m_runner, &CommandRunner::jobFinished, this, [this](int jobId, int exitCode, const QString &errorString) {
        if (jobId == m_searchJobId) {
            m_searchJobId = -1;
            setLoading(false);
            if (exitCode != 0 && !errorString.isEmpty()) {
                setError(QStringLiteral("Search failed: ") + errorString);
            }
        } else if (jobId == m_detailJobId) {
            m_detailJobId = -1;
            if (exitCode == 0) {
                auto details = parseDetailOutput(m_detailOutput);
                Q_EMIT detailsReady(m_pendingDetailIndex, details);
            }
            m_detailOutput.clear();
        } else if (jobId == m_actionJobId) {
            m_actionJobId = -1;
            if (exitCode == 0) {
                Q_EMIT operationFinished(true, QStringLiteral("Operation completed successfully"));
                // Refresh search results
                if (!searchQuery().isEmpty()) {
                    search(searchQuery());
                }
            } else {
                Q_EMIT operationFinished(false, QStringLiteral("Operation failed: ") + errorString);
            }
        }
    });

    connect(m_runner, &CommandRunner::outputLine, this, [this](int jobId, const QString &line, bool) {
        if (jobId == m_detailJobId) {
            m_detailOutput += line + QStringLiteral("\n");
        }
    });
}

void PacmanBackend::search(const QString &query)
{
    if (query.isEmpty()) {
        setPackages({});
        return;
    }

    setSearchQuery(query);
    clearError();
    setLoading(true);

    auto result = m_runner->runSync(QStringLiteral("pacman"), {QStringLiteral("-Ss"), query});

    if (result.exitCode == 0) {
        auto pkgs = parseSearchOutput(result.stdout);
        setPackages(pkgs);
    } else if (result.exitCode == 1) {
        // No results
        setPackages({});
    } else {
        setError(QStringLiteral("Search failed: ") + result.stderr);
    }

    setLoading(false);
    LogManager::instance()->log(QStringLiteral("Pacman search: %1 (%2 results)").arg(query).arg(count()));
}

void PacmanBackend::fetchDetails(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }

    const auto &pkg = packages().at(index);
    m_pendingDetailIndex = index;
    m_detailOutput.clear();

    // Use -Qi for installed, -Si for repo
    QString flag = pkg.installed ? QStringLiteral("-Qi") : QStringLiteral("-Si");
    auto result = m_runner->runSync(QStringLiteral("pacman"), {flag, pkg.name});

    if (result.exitCode == 0) {
        auto details = parseDetailOutput(result.stdout);
        Q_EMIT detailsReady(index, details);
    }
}

void PacmanBackend::install(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    const auto &pkg = packages().at(index);
    Q_EMIT confirmationRequired(
        QStringLiteral("install"),
        pkg.name,
        QStringLiteral("Install %1 %2 from %3?").arg(pkg.name, pkg.version, pkg.repository));
}

void PacmanBackend::remove(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    const auto &pkg = packages().at(index);
    Q_EMIT confirmationRequired(
        QStringLiteral("remove"),
        pkg.name,
        QStringLiteral("Remove %1 and its unneeded dependencies?").arg(pkg.name));
}

void PacmanBackend::confirmInstall(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    const auto &pkg = packages().at(index);
    LogManager::instance()->log(QStringLiteral("Installing pacman package: %1").arg(pkg.name));

    m_actionJobId = m_runner->runPrivileged(
        QStringLiteral("pacman"),
        {QStringLiteral("-S"), QStringLiteral("--needed"), QStringLiteral("--noconfirm"), pkg.name});
}

void PacmanBackend::confirmRemove(int index)
{
    if (index < 0 || index >= count()) {
        return;
    }
    const auto &pkg = packages().at(index);
    LogManager::instance()->log(QStringLiteral("Removing pacman package: %1").arg(pkg.name));

    m_actionJobId = m_runner->runPrivileged(
        QStringLiteral("pacman"),
        {QStringLiteral("-Rns"), QStringLiteral("--noconfirm"), pkg.name});
}

QList<PackageInfo> PacmanBackend::parseSearchOutput(const QString &output)
{
    QList<PackageInfo> packages;

    // pacman -Ss format:
    // repo/name version [installed] [installed: version]
    //     description
    static QRegularExpression headerRe(
        QStringLiteral(R"(^(\S+)/(\S+)\s+(\S+)(.*)$)"));

    const auto lines = output.split(QLatin1Char('\n'));

    for (int i = 0; i < lines.size(); ++i) {
        const auto &line = lines[i];
        auto match = headerRe.match(line);

        if (match.hasMatch()) {
            PackageInfo pkg;
            pkg.repository = match.captured(1);
            pkg.name = match.captured(2);
            pkg.version = match.captured(3);

            QString rest = match.captured(4).trimmed();
            pkg.installed = rest.contains(QStringLiteral("[installed"));

            if (pkg.installed) {
                static QRegularExpression installedVerRe(
                    QStringLiteral(R"(\[installed:\s*(\S+)\])"));
                auto verMatch = installedVerRe.match(rest);
                if (verMatch.hasMatch()) {
                    pkg.installedVersion = verMatch.captured(1);
                } else {
                    pkg.installedVersion = pkg.version;
                }
            }

            // Next line is description
            if (i + 1 < lines.size()) {
                pkg.description = lines[i + 1].trimmed();
                ++i;
            }

            packages.append(pkg);
        }
    }

    return packages;
}

QVariantMap PacmanBackend::parseDetailOutput(const QString &output)
{
    QVariantMap details;

    // pacman -Si/-Qi format:
    // Key                : Value
    static QRegularExpression kvRe(
        QStringLiteral(R"(^([A-Za-z ]+?)\s*:\s*(.*)$)"));

    const auto lines = output.split(QLatin1Char('\n'));
    for (const auto &line : lines) {
        auto match = kvRe.match(line);
        if (match.hasMatch()) {
            QString key = match.captured(1).trimmed();
            QString value = match.captured(2).trimmed();
            details.insert(key, value);
        }
    }

    return details;
}
