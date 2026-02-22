#pragma once

#include "core/packagemodel.h"

class CommandRunner;

class PacmanBackend : public PackageModel
{
    Q_OBJECT

public:
    explicit PacmanBackend(CommandRunner *runner, QObject *parent = nullptr);

    void search(const QString &query) override;
    void fetchDetails(int index) override;
    void install(int index) override;
    void remove(int index) override;

    Q_INVOKABLE void confirmInstall(int index);
    Q_INVOKABLE void confirmRemove(int index);

    // Exposed for testing
    static QList<PackageInfo> parseSearchOutput(const QString &output);
    static QVariantMap parseDetailOutput(const QString &output);

private:
    CommandRunner *m_runner;
    int m_searchJobId = -1;
    int m_detailJobId = -1;
    int m_actionJobId = -1;
    int m_pendingDetailIndex = -1;
    QString m_detailOutput;
};
