#include "firmwarebackend.h"
#include "core/commandrunner.h"
#include "core/logmanager.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

FirmwareBackend::FirmwareBackend(CommandRunner *runner, QObject *parent)
    : QAbstractListModel(parent)
    , m_runner(runner)
{
}

int FirmwareBackend::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_devices.size();
}

QVariant FirmwareBackend::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_devices.size()) {
        return {};
    }

    const auto &dev = m_devices.at(index.row());

    switch (role) {
    case DeviceNameRole:
        return dev.name;
    case DeviceIdRole:
        return dev.deviceId;
    case VendorRole:
        return dev.vendor;
    case CurrentVersionRole:
        return dev.currentVersion;
    case UpdateVersionRole:
        return dev.updateVersion;
    case HasUpdateRole:
        return dev.hasUpdate;
    case SummaryRole:
        return dev.summary;
    case PluginRole:
        return dev.plugin;
    }

    return {};
}

QHash<int, QByteArray> FirmwareBackend::roleNames() const
{
    return {
        {DeviceNameRole, "deviceName"},
        {DeviceIdRole, "deviceId"},
        {VendorRole, "vendor"},
        {CurrentVersionRole, "currentVersion"},
        {UpdateVersionRole, "updateVersion"},
        {HasUpdateRole, "hasUpdate"},
        {SummaryRole, "summary"},
        {PluginRole, "plugin"},
    };
}

void FirmwareBackend::loadDevices()
{
    m_loading = true;
    Q_EMIT loadingChanged();

    auto result = m_runner->runSync(
        QStringLiteral("fwupdmgr"),
        {QStringLiteral("get-devices"), QStringLiteral("--json")},
        30000);

    if (result.exitCode == 0) {
        parseDevicesJson(result.stdout);
        m_errorMessage.clear();
    } else {
        m_errorMessage = QStringLiteral("Failed to load devices: ") + result.stderr;
        Q_EMIT errorMessageChanged();
    }

    m_loading = false;
    Q_EMIT loadingChanged();
    LogManager::instance()->log(QStringLiteral("Loaded %1 firmware devices").arg(m_devices.size()));
}

void FirmwareBackend::checkUpdates()
{
    m_loading = true;
    Q_EMIT loadingChanged();

    auto result = m_runner->runSync(
        QStringLiteral("fwupdmgr"),
        {QStringLiteral("get-updates"), QStringLiteral("--json")},
        30000);

    if (result.exitCode == 0) {
        parseUpdatesJson(result.stdout);
    } else {
        // Exit code 2 means no updates available
        if (result.exitCode != 2) {
            m_errorMessage = QStringLiteral("Failed to check updates: ") + result.stderr;
            Q_EMIT errorMessageChanged();
        }
    }

    m_loading = false;
    Q_EMIT loadingChanged();
}

void FirmwareBackend::refreshMetadata()
{
    LogManager::instance()->log(QStringLiteral("Refreshing firmware metadata"));

    m_runner->runPrivileged(
        QStringLiteral("fwupdmgr"),
        {QStringLiteral("refresh")},
        CommandRunner::Embedded,
        60000);
}

void FirmwareBackend::updateDevice(int index)
{
    if (index < 0 || index >= m_devices.size()) {
        return;
    }

    const auto &dev = m_devices.at(index);
    LogManager::instance()->log(QStringLiteral("Updating firmware for: %1").arg(dev.name));

    int jobId = m_runner->runPrivileged(
        QStringLiteral("fwupdmgr"),
        {QStringLiteral("update"), dev.deviceId},
        CommandRunner::Embedded,
        180000);

    connect(m_runner, &CommandRunner::jobFinished, this, [this, jobId, dev](int id, int exitCode, const QString &err) {
        if (id != jobId) return;
        if (exitCode == 0) {
            Q_EMIT operationFinished(true, QStringLiteral("Firmware updated for %1").arg(dev.name));
            loadDevices();
        } else {
            Q_EMIT operationFinished(false, QStringLiteral("Firmware update failed: ") + err);
        }
    });
}

void FirmwareBackend::parseDevicesJson(const QString &json)
{
    auto doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isObject()) {
        return;
    }

    auto devicesArray = doc.object().value(QStringLiteral("Devices")).toArray();

    beginResetModel();
    m_devices.clear();

    for (const auto &val : devicesArray) {
        auto obj = val.toObject();
        DeviceInfo dev;
        dev.name = obj.value(QStringLiteral("Name")).toString();
        dev.deviceId = obj.value(QStringLiteral("DeviceId")).toString();
        dev.vendor = obj.value(QStringLiteral("Vendor")).toString();
        dev.currentVersion = obj.value(QStringLiteral("Version")).toString();
        dev.summary = obj.value(QStringLiteral("Summary")).toString();
        dev.plugin = obj.value(QStringLiteral("Plugin")).toString();
        m_devices.append(dev);
    }

    endResetModel();
}

void FirmwareBackend::parseUpdatesJson(const QString &json)
{
    auto doc = QJsonDocument::fromJson(json.toUtf8());
    if (!doc.isObject()) {
        return;
    }

    auto devicesArray = doc.object().value(QStringLiteral("Devices")).toArray();

    int updateCount = 0;

    for (const auto &val : devicesArray) {
        auto obj = val.toObject();
        QString deviceId = obj.value(QStringLiteral("DeviceId")).toString();

        auto releases = obj.value(QStringLiteral("Releases")).toArray();
        if (releases.isEmpty()) {
            continue;
        }

        QString updateVer = releases.first().toObject().value(QStringLiteral("Version")).toString();

        // Match to existing device
        for (int i = 0; i < m_devices.size(); ++i) {
            if (m_devices[i].deviceId == deviceId) {
                m_devices[i].hasUpdate = true;
                m_devices[i].updateVersion = updateVer;
                QModelIndex idx = index(i);
                Q_EMIT dataChanged(idx, idx, {HasUpdateRole, UpdateVersionRole});
                ++updateCount;
                break;
            }
        }
    }

    if (m_updateCount != updateCount) {
        m_updateCount = updateCount;
        Q_EMIT updateCountChanged();
    }
}
