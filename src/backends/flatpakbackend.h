#pragma once

#include "core/packagemodel.h"
#include <QStringList>

class CommandRunner;

class FlatpakBackend : public PackageModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList remotes READ remotes NOTIFY remotesChanged)
    Q_PROPERTY(QString activeRemote READ activeRemote WRITE setActiveRemote NOTIFY activeRemoteChanged)

public:
    explicit FlatpakBackend(CommandRunner *runner, QObject *parent = nullptr);

    void search(const QString &query) override;
    void fetchDetails(int index) override;
    void install(int index) override;
    void remove(int index) override;

    Q_INVOKABLE void confirmInstall(int index);
    Q_INVOKABLE void confirmRemove(int index);
    Q_INVOKABLE void loadRemotes();

    QStringList remotes() const { return m_remotes; }
    QString activeRemote() const { return m_activeRemote; }
    void setActiveRemote(const QString &remote);

    // Exposed for testing
    static QList<PackageInfo> parseSearchOutput(const QString &output);

Q_SIGNALS:
    void remotesChanged();
    void activeRemoteChanged();

private:
    CommandRunner *m_runner;
    QStringList m_remotes;
    QString m_activeRemote = QStringLiteral("flathub");
};
