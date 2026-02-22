#pragma once

#include "core/packagemodel.h"

class CommandRunner;

class AurBackend : public PackageModel
{
    Q_OBJECT

    Q_PROPERTY(QStringList orphanPackages READ orphanPackages NOTIFY orphanPackagesChanged)

public:
    explicit AurBackend(CommandRunner *runner, QObject *parent = nullptr);

    void search(const QString &query) override;
    void fetchDetails(int index) override;
    void install(int index) override;
    void remove(int index) override;

    Q_INVOKABLE void confirmInstall(int index);
    Q_INVOKABLE void confirmRemove(int index);
    Q_INVOKABLE void checkOrphans();
    Q_INVOKABLE void removeOrphans();

    QStringList orphanPackages() const { return m_orphanPackages; }

Q_SIGNALS:
    void orphanPackagesChanged();
    void orphansFound(const QStringList &orphans);
    void removalStepCompleted(int step, bool success, const QString &message);

private:
    CommandRunner *m_runner;
    QStringList m_orphanPackages;
};
