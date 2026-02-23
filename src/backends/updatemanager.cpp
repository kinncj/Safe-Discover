#include "updatemanager.h"
#include "core/commandrunner.h"
#include "core/logmanager.h"
#include "pacmanbackend.h"
#include "aurbackend.h"
#include "flatpakbackend.h"
#include "firmwarebackend.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

UpdateManager::UpdateManager(CommandRunner *runner,
                             PacmanBackend *pacman,
                             AurBackend *aur,
                             FlatpakBackend *flatpak,
                             FirmwareBackend *firmware,
                             QObject *parent)
    : QObject(parent)
    , m_runner(runner)
    , m_pacman(pacman)
    , m_aur(aur)
    , m_flatpak(flatpak)
    , m_firmware(firmware)
{
    connect(m_runner, &CommandRunner::jobFinished,
            this, &UpdateManager::onJobFinished);
}

void UpdateManager::checkAllUpdates()
{
    m_checking = true;
    Q_EMIT checkingChanged();

    LogManager::instance()->log(QStringLiteral("Checking for updates across all backends"));

    // Check pacman updates
    auto pacResult = m_runner->runSync(QStringLiteral("pacman"), {QStringLiteral("-Qu")});
    if (pacResult.exitCode == 0 && !pacResult.stdout.trimmed().isEmpty()) {
        m_pacmanUpdateList = parsePacmanQuOutput(pacResult.stdout);
    } else {
        m_pacmanUpdateList.clear();
    }
    Q_EMIT pacmanUpdatesChanged();

    // Check AUR updates
    auto aurResult = m_runner->runSync(QStringLiteral("paru"), {QStringLiteral("-Qua")});
    if (aurResult.exitCode == 0 && !aurResult.stdout.trimmed().isEmpty()) {
        m_aurUpdateList = parsePacmanQuOutput(aurResult.stdout);
    } else {
        m_aurUpdateList.clear();
    }
    Q_EMIT aurUpdatesChanged();

    // Check flatpak updates
    auto fpResult = m_runner->runSync(
        QStringLiteral("flatpak"),
        {QStringLiteral("remote-ls"), QStringLiteral("--updates"),
         QStringLiteral("--columns=name,application,version,branch,remotes")});
    if (fpResult.exitCode == 0 && !fpResult.stdout.trimmed().isEmpty()) {
        m_flatpakUpdateList = parseFlatpakUpdatesOutput(fpResult.stdout);
    } else {
        m_flatpakUpdateList.clear();
    }
    Q_EMIT flatpakUpdatesChanged();

    // Check firmware updates
    auto fwResult = m_runner->runSync(
        QStringLiteral("fwupdmgr"),
        {QStringLiteral("get-updates"), QStringLiteral("--json")});
    if (fwResult.exitCode == 0) {
        m_firmwareUpdateList = parseFirmwareUpdatesJson(fwResult.stdout);
    } else {
        m_firmwareUpdateList.clear();
    }
    Q_EMIT firmwareUpdatesChanged();

    m_checking = false;
    Q_EMIT checkingChanged();

    LogManager::instance()->log(QStringLiteral("Update check complete: pacman=%1 aur=%2 flatpak=%3 firmware=%4")
        .arg(pacmanUpdates()).arg(aurUpdates()).arg(flatpakUpdates()).arg(firmwareUpdates()));
}

QVariantList UpdateManager::parsePacmanQuOutput(const QString &output)
{
    QVariantList list;
    const auto lines = output.trimmed().split(QLatin1Char('\n'));
    for (const auto &line : lines) {
        const auto trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }
        // Format: "name currentVer -> newVer"
        const auto parts = trimmed.split(QLatin1Char(' '), Qt::SkipEmptyParts);
        if (parts.size() >= 4 && parts[2] == QStringLiteral("->")) {
            QVariantMap entry;
            entry[QStringLiteral("name")] = parts[0];
            entry[QStringLiteral("currentVersion")] = parts[1];
            entry[QStringLiteral("newVersion")] = parts[3];
            list.append(entry);
        } else if (parts.size() >= 2) {
            // Fallback: "name version"
            QVariantMap entry;
            entry[QStringLiteral("name")] = parts[0];
            entry[QStringLiteral("currentVersion")] = parts[1];
            entry[QStringLiteral("newVersion")] = QString();
            list.append(entry);
        }
    }
    return list;
}

QVariantList UpdateManager::parseFlatpakUpdatesOutput(const QString &output)
{
    QVariantList list;
    const auto lines = output.trimmed().split(QLatin1Char('\n'));
    for (const auto &line : lines) {
        const auto trimmed = line.trimmed();
        if (trimmed.isEmpty()) {
            continue;
        }
        // Tab-delimited: name, application, version, branch, remotes
        const auto parts = trimmed.split(QLatin1Char('\t'));
        QVariantMap entry;
        entry[QStringLiteral("name")] = parts.size() > 0 ? parts[0].trimmed() : QString();
        entry[QStringLiteral("appId")] = parts.size() > 1 ? parts[1].trimmed() : QString();
        entry[QStringLiteral("newVersion")] = parts.size() > 2 ? parts[2].trimmed() : QString();
        entry[QStringLiteral("branch")] = parts.size() > 3 ? parts[3].trimmed() : QString();
        entry[QStringLiteral("remote")] = parts.size() > 4 ? parts[4].trimmed() : QString();
        entry[QStringLiteral("currentVersion")] = QString();
        if (!entry[QStringLiteral("name")].toString().isEmpty()) {
            list.append(entry);
        }
    }
    return list;
}

QVariantList UpdateManager::parseFirmwareUpdatesJson(const QString &json)
{
    QVariantList list;
    auto doc = QJsonDocument::fromJson(json.toUtf8());
    if (doc.isNull()) {
        return list;
    }
    const auto devices = doc.object().value(QStringLiteral("Devices")).toArray();
    for (const auto &deviceVal : devices) {
        const auto device = deviceVal.toObject();
        QVariantMap entry;
        entry[QStringLiteral("name")] = device.value(QStringLiteral("Name")).toString();
        entry[QStringLiteral("vendor")] = device.value(QStringLiteral("Vendor")).toString();
        entry[QStringLiteral("currentVersion")] = device.value(QStringLiteral("Version")).toString();
        const auto releases = device.value(QStringLiteral("Releases")).toArray();
        if (!releases.isEmpty()) {
            entry[QStringLiteral("newVersion")] = releases[0].toObject().value(QStringLiteral("Version")).toString();
        } else {
            entry[QStringLiteral("newVersion")] = QString();
        }
        list.append(entry);
    }
    return list;
}

void UpdateManager::runSafeUpdate()
{
    if (m_running) {
        return;
    }

    m_running = true;
    m_cancelled = false;
    m_currentStep = 0;
    Q_EMIT runningChanged();

    LogManager::instance()->log(QStringLiteral("Starting safe update sequence"));

    runStep(1);
}

void UpdateManager::cancel()
{
    m_cancelled = true;
    if (m_currentJobId >= 0) {
        m_runner->cancel(m_currentJobId);
    }
    LogManager::instance()->log(QStringLiteral("Update cancelled by user"));
}

void UpdateManager::runStep(int step)
{
    if (m_cancelled) {
        m_running = false;
        m_currentJobId = -1;
        Q_EMIT runningChanged();
        Q_EMIT updateFinished(false, QStringLiteral("Update cancelled by user"));
        return;
    }

    m_currentStep = step;
    Q_EMIT currentStepChanged();

    switch (step) {
    case 1:
        setStepLabel(QStringLiteral("Updating system packages (pacman)..."));
        if (pacmanUpdates() > 0) {
            m_currentJobId = m_runner->runPrivileged(
                QStringLiteral("pacman"),
                {QStringLiteral("-Syu"), QStringLiteral("--noconfirm")},
                CommandRunner::Embedded, 300000);
        } else {
            runStep(2);
        }
        break;

    case 2:
        setStepLabel(QStringLiteral("Updating AUR packages (paru)..."));
        if (aurUpdates() > 0) {
            // AUR updates use terminal mode — fire-and-forget (detached process).
            // Don't track the jobId: Terminal mode emits jobFinished synchronously
            // inside run(), before the return value can be assigned to m_currentJobId.
            m_runner->run(
                QStringLiteral("paru"),
                {QStringLiteral("-Sua"), QStringLiteral("--noconfirm")},
                CommandRunner::Terminal);
        }
        // Always advance immediately — terminal is untrackable
        runStep(3);
        break;

    case 3:
        setStepLabel(QStringLiteral("Updating Flatpak applications..."));
        if (flatpakUpdates() > 0) {
            m_currentJobId = m_runner->run(
                QStringLiteral("flatpak"),
                {QStringLiteral("update"), QStringLiteral("-y")},
                CommandRunner::Embedded, 300000);
        } else {
            runStep(4);
        }
        break;

    case 4:
        setStepLabel(QStringLiteral("Updating firmware..."));
        if (firmwareUpdates() > 0) {
            m_currentJobId = m_runner->runPrivileged(
                QStringLiteral("fwupdmgr"),
                {QStringLiteral("update")},
                CommandRunner::Embedded, 180000);
        } else {
            // All steps completed (nothing to do in this step)
            m_running = false;
            m_currentJobId = -1;
            Q_EMIT runningChanged();
            setStepLabel(QStringLiteral("Update complete"));
            Q_EMIT updateFinished(true, QStringLiteral("All updates applied successfully"));
            LogManager::instance()->log(QStringLiteral("Safe update sequence completed"));
        }
        break;
    }
}

void UpdateManager::onJobFinished(int jobId, int exitCode, const QString &errorString)
{
    if (jobId != m_currentJobId || !m_running) {
        return;
    }
    m_currentJobId = -1;

    if (m_cancelled) {
        m_running = false;
        Q_EMIT runningChanged();
        Q_EMIT updateFinished(false, QStringLiteral("Update cancelled by user"));
        return;
    }

    if (exitCode != 0) {
        QString failMsg = m_currentStepLabel + QStringLiteral(" failed");
        if (!errorString.isEmpty()) {
            failMsg += QStringLiteral(": ") + errorString;
        }
        m_running = false;
        Q_EMIT runningChanged();
        Q_EMIT updateFinished(false, failMsg);
        return;
    }

    // Advance to next step
    if (m_currentStep < 4) {
        runStep(m_currentStep + 1);
    } else {
        // All steps completed
        m_running = false;
        Q_EMIT runningChanged();
        setStepLabel(QStringLiteral("Update complete"));
        Q_EMIT updateFinished(true, QStringLiteral("All updates applied successfully"));
        LogManager::instance()->log(QStringLiteral("Safe update sequence completed"));
    }
}

void UpdateManager::setStepLabel(const QString &label)
{
    m_currentStepLabel = label;
    Q_EMIT currentStepLabelChanged();
    LogManager::instance()->log(label);
}
