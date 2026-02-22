#pragma once

#include <QAbstractListModel>
#include <QJsonArray>

class CommandRunner;

class FirmwareBackend : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(bool loading READ loading NOTIFY loadingChanged)
    Q_PROPERTY(QString errorMessage READ errorMessage NOTIFY errorMessageChanged)
    Q_PROPERTY(int updateCount READ updateCount NOTIFY updateCountChanged)

public:
    enum Roles {
        DeviceNameRole = Qt::UserRole + 1,
        DeviceIdRole,
        VendorRole,
        CurrentVersionRole,
        UpdateVersionRole,
        HasUpdateRole,
        SummaryRole,
        PluginRole,
    };
    Q_ENUM(Roles)

    explicit FirmwareBackend(CommandRunner *runner, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    bool loading() const { return m_loading; }
    QString errorMessage() const { return m_errorMessage; }
    int updateCount() const { return m_updateCount; }

    Q_INVOKABLE void loadDevices();
    Q_INVOKABLE void checkUpdates();
    Q_INVOKABLE void refreshMetadata();
    Q_INVOKABLE void updateDevice(int index);

Q_SIGNALS:
    void loadingChanged();
    void errorMessageChanged();
    void updateCountChanged();
    void operationFinished(bool success, const QString &message);

private:
    struct DeviceInfo {
        QString name;
        QString deviceId;
        QString vendor;
        QString currentVersion;
        QString updateVersion;
        bool hasUpdate = false;
        QString summary;
        QString plugin;
    };

    void parseDevicesJson(const QString &json);
    void parseUpdatesJson(const QString &json);

    CommandRunner *m_runner;
    QList<DeviceInfo> m_devices;
    bool m_loading = false;
    QString m_errorMessage;
    int m_updateCount = 0;
};
