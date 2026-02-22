#include "packagemodel.h"

PackageModel::PackageModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

int PackageModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        return 0;
    }
    return m_packages.size();
}

QVariant PackageModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_packages.size()) {
        return {};
    }

    const auto &pkg = m_packages.at(index.row());

    switch (role) {
    case NameRole:
        return pkg.name;
    case VersionRole:
        return pkg.version;
    case DescriptionRole:
        return pkg.description;
    case RepositoryRole:
        return pkg.repository;
    case InstalledRole:
        return pkg.installed;
    case InstalledVersionRole:
        return pkg.installedVersion;
    case SizeRole:
        return pkg.size;
    }

    return {};
}

QHash<int, QByteArray> PackageModel::roleNames() const
{
    return {
        {NameRole, "name"},
        {VersionRole, "version"},
        {DescriptionRole, "description"},
        {RepositoryRole, "repository"},
        {InstalledRole, "installed"},
        {InstalledVersionRole, "installedVersion"},
        {SizeRole, "size"},
    };
}

void PackageModel::setSearchQuery(const QString &query)
{
    if (m_searchQuery != query) {
        m_searchQuery = query;
        Q_EMIT searchQueryChanged();
    }
}

void PackageModel::setLoading(bool loading)
{
    if (m_loading != loading) {
        m_loading = loading;
        Q_EMIT loadingChanged();
    }
}

void PackageModel::setError(const QString &msg)
{
    m_errorMessage = msg;
    Q_EMIT errorMessageChanged();
}

void PackageModel::clearError()
{
    if (!m_errorMessage.isEmpty()) {
        m_errorMessage.clear();
        Q_EMIT errorMessageChanged();
    }
}

void PackageModel::setPackages(const QList<PackageInfo> &packages)
{
    beginResetModel();
    m_packages = packages;
    endResetModel();
    Q_EMIT countChanged();
}
